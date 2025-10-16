#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

// Use bundled nlohmann json.hpp
#include "json.hpp"

// JSON-RPC 2.0 implementation using nlohmann::json
// https://www.jsonrpc.org/specification
// This is an adapted version from the jsonrpc20 library by Pooria Yousefi

namespace pooriayousefi
{

    using nlohmann::json;

    // JSON-RPC 2.0 error codes
    // https://www.jsonrpc.org/specification#error_object
    struct error
    {
        int code;
        std::string message;
        json data = nullptr; // optional
    };

    inline json make_error_object(const error &e)
    {
        json j = {{"code", e.code}, {"message", e.message}};
        if (!e.data.is_null())
            j["data"] = e.data;
        return j;
    }

    // Standard errors
    static const error parse_error{-32700, "Parse error", nullptr};
    static const error invalid_request{-32600, "Invalid Request", nullptr};
    static const error method_not_found{-32601, "Method not found", nullptr};
    static const error invalid_params{-32602, "Invalid params", nullptr};
    static const error internal_error{-32603, "Internal error", nullptr};
    static const error request_cancelled{-32800, "Request cancelled", nullptr};

    // Helpers to detect message flavors
    inline bool is_request(const json &j)
    {
        return j.is_object() && j.value("jsonrpc", "") == "2.0" && j.contains("method") &&
               !j.contains("result") && !j.contains("error") &&
               (j.contains("id") || !j.contains("id"));
    }

    inline bool is_notification(const json &j) { return is_request(j) && !j.contains("id"); }

    inline bool is_response(const json &j)
    {
        return j.is_object() && j.value("jsonrpc", "") == "2.0" && j.contains("id") &&
               (j.contains("result") ^ j.contains("error"));
    }

    // Validation utilities
    inline bool valid_id_type(const json &id)
    {
        return id.is_null() || id.is_string() || id.is_number_integer() || id.is_number_unsigned();
    }

    inline bool validate_request(const json &j, std::string *why = nullptr)
    {
        if (!j.is_object())
        {
            if (why)
                *why = "not an object";
            return false;
        }
        if (j.value("jsonrpc", "") != "2.0")
        {
            if (why)
                *why = "jsonrpc != 2.0";
            return false;
        }
        if (!j.contains("method") || !j["method"].is_string())
        {
            if (why)
                *why = "method missing or not string";
            return false;
        }
        if (j.contains("id") && !valid_id_type(j["id"]))
        {
            if (why)
                *why = "invalid id type";
            return false;
        }
        if (j.contains("params") && !(j["params"].is_array() || j["params"].is_object()))
        {
            if (why)
                *why = "params must be array or object";
            return false;
        }
        return true;
    }

    inline bool validate_response(const json &j, std::string *why = nullptr)
    {
        if (!j.is_object())
        {
            if (why)
                *why = "not an object";
            return false;
        }
        if (j.value("jsonrpc", "") != "2.0")
        {
            if (why)
                *why = "jsonrpc != 2.0";
            return false;
        }
        if (!j.contains("id") || !valid_id_type(j["id"]))
        {
            if (why)
                *why = "missing or invalid id";
            return false;
        }
        bool has_result = j.contains("result");
        bool has_error = j.contains("error");
        if (has_result == has_error)
        {
            if (why)
                *why = "must have exactly one of result or error";
            return false;
        }
        if (has_error)
        {
            const auto &e = j["error"];
            if (!e.is_object() || !e.contains("code") || !e["code"].is_number_integer() ||
                !e.contains("message") || !e["message"].is_string())
            {
                if (why)
                {
                    *why = "invalid error object";
                }
                return false;
            }
        }
        return true;
    }

    // Builders
    inline json make_request(const std::variant<std::nullptr_t, std::string, int64_t, uint64_t> &id,
                             const std::string &method, const json &params = json{})
    {
        json j = {{"jsonrpc", "2.0"}, {"method", method}};
        if (!params.is_null() && !params.empty())
            j["params"] = params;
        if (!std::holds_alternative<std::nullptr_t>(id))
        {
            std::visit([&](auto &&v) { j["id"] = v; }, id);
        }
        return j;
    }

    inline json make_notification(const std::string &method, const json &params = json{})
    {
        return make_request(nullptr, method, params);
    }

    inline json make_result(const json &id, const json &result)
    {
        return json{{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
    }

    inline json make_error(const json &id, const error &e)
    {
        return json{{"jsonrpc", "2.0"}, {"id", id}, {"error", make_error_object(e)}};
    }

    // rpc_exception allows handlers to intentionally return a specific JSON-RPC error
    struct rpc_exception : std::runtime_error
    {
        error err;
        explicit rpc_exception(error e) : std::runtime_error(e.message), err(std::move(e)) {}
    };

    [[noreturn]] inline void throw_rpc_error(error e) { throw rpc_exception(std::move(e)); }

    // --- Serialization/Deserialization Support ---
    namespace detail
    {
        // Helper to deserialize params based on ParamsT type
        template <typename ParamsT> ParamsT deserialize_params(const json &params)
        {
            if constexpr (std::is_same_v<ParamsT, json>)
            {
                // Pass through raw JSON
                return params;
            }
            else if constexpr (std::is_same_v<ParamsT, void>)
            {
                // No params expected
                static_assert(std::is_same_v<ParamsT, void>,
                              "void params not directly deserializable");
            }
            else
            {
                // Params must be array or object per JSON-RPC 2.0 spec
                // If it's an array with one element, extract that element
                // Otherwise, deserialize the whole structure
                if (params.is_array() && params.size() == 1)
                {
                    return params[0].get<ParamsT>();
                }
                else
                {
                    return params.get<ParamsT>();
                }
            }
        }

        // Helper to serialize params for sending (wraps in array if needed)
        template <typename ParamsT> json serialize_params(ParamsT &&params)
        {
            if constexpr (std::is_same_v<std::decay_t<ParamsT>, json>)
            {
                // If already JSON, check if it's array or object
                json j = std::forward<ParamsT>(params);
                if (j.is_array() || j.is_object())
                {
                    return j;
                }
                else
                {
                    // Wrap primitive in array
                    return json::array({j});
                }
            }
            else
            {
                // Convert to JSON
                json j = json(std::forward<ParamsT>(params));
                // Wrap non-array/object in array for JSON-RPC 2.0 compliance
                if (!j.is_array() && !j.is_object())
                {
                    return json::array({j});
                }
                return j;
            }
        }

        // Helper to serialize result based on ResultT type
        template <typename ResultT> json serialize_result(ResultT &&result)
        {
            if constexpr (std::is_same_v<std::decay_t<ResultT>, json>)
            {
                // Pass through raw JSON
                return std::forward<ResultT>(result);
            }
            else if constexpr (std::is_same_v<std::decay_t<ResultT>, void>)
            {
                // Void return - return null
                return json(nullptr);
            }
            else
            {
                // Use nlohmann::json's automatic conversion
                return json(std::forward<ResultT>(result));
            }
        }

        // Wrapper to create typed handler from function with C++ types
        template <typename ParamsT, typename ResultT>
        std::function<json(const json &)> make_typed_handler(std::function<ResultT(ParamsT)> fn)
        {
            return [fn = std::move(fn)](const json &params) -> json
            {
                try
                {
                    ParamsT typed_params = deserialize_params<ParamsT>(params);
                    if constexpr (std::is_same_v<ResultT, void>)
                    {
                        fn(std::move(typed_params));
                        return json(nullptr);
                    }
                    else
                    {
                        ResultT result = fn(std::move(typed_params));
                        return serialize_result(std::move(result));
                    }
                }
                catch (const json::exception &ex)
                {
                    // JSON parsing/conversion error
                    error e = invalid_params;
                    e.data = json{{"what", ex.what()}};
                    throw rpc_exception(e);
                }
            };
        }

        // Wrapper for no-params handlers
        template <typename ResultT>
        std::function<json(const json &)> make_no_params_handler(std::function<ResultT()> fn)
        {
            return [fn = std::move(fn)](const json &) -> json
            {
                if constexpr (std::is_same_v<ResultT, void>)
                {
                    fn();
                    return json(nullptr);
                }
                else
                {
                    ResultT result = fn();
                    return serialize_result(std::move(result));
                }
            };
        }
    } // namespace detail

    // Dispatcher
    class dispatcher
    {
      public:
        using handler_t = std::function<json(const json &params)>; // params may be array or object

        // Add raw JSON handler (original method)
        void add(const std::string &method, handler_t fn) { handlers_[method] = std::move(fn); }

        // Add typed handler: takes C++ type ParamsT and returns ResultT
        template <typename ParamsT, typename ResultT>
        void add_typed(const std::string &method, std::function<ResultT(ParamsT)> fn)
        {
            handlers_[method] = detail::make_typed_handler<ParamsT, ResultT>(std::move(fn));
        }

        // Add no-params handler: takes no parameters and returns ResultT
        template <typename ResultT>
        void add_no_params(const std::string &method, std::function<ResultT()> fn)
        {
            handlers_[method] = detail::make_no_params_handler<ResultT>(std::move(fn));
        }

        // Handle a single request/notification. Returns optional response (none for notifications).
        std::optional<json> handle_single(const json &msg) const
        {
            std::string why;
            if (!validate_request(msg, &why))
            {
                // Per spec, invalid request returns an error with id = null
                return make_error(nullptr, invalid_request);
            }
            const std::string method = msg["method"].get<std::string>();
            const bool is_notif = !msg.contains("id");
            const json id = is_notif ? json() : msg["id"]; // null if notification

            auto it = handlers_.find(method);
            if (it == handlers_.end())
            {
                if (is_notif)
                    return std::nullopt; // notifications get no response
                return make_error(id, method_not_found);
            }
            try
            {
                json params = msg.contains("params") ? msg["params"] : json{};
                json result = it->second(params);
                if (is_notif)
                    return std::nullopt;
                return make_result(id, result);
            }
            catch (const rpc_exception &ex)
            {
                if (is_notif)
                    return std::nullopt;
                return make_error(id, ex.err);
            }
            catch (const std::exception &ex)
            {
                if (is_notif)
                    return std::nullopt; // swallow per spec
                error e = internal_error;
                e.data = json{{"what", ex.what()}};
                return make_error(id, e);
            }
        }

        // Handle input that may be single or batch. Returns either a single response object,
        // an array of response objects (for batch), or null if only notifications.
        std::optional<json> handle(const json &input) const
        {
            if (input.is_array())
            {
                if (input.empty())
                {
                    // Spec: empty batch is an invalid request
                    return make_error(nullptr, invalid_request);
                }
                std::vector<json> out;
                out.reserve(input.size());
                for (const auto &el : input)
                {
                    auto r = handle_single(el);
                    if (r)
                        out.push_back(*r);
                }
                if (out.empty())
                    return std::nullopt; // all were notifications
                return json(out);
            }
            else
            {
                return handle_single(input);
            }
        }

      private:
        std::unordered_map<std::string, handler_t> handlers_;
    };

    // --- Handler call context (progress + cancellation) ---
    struct call_context
    {
        json id;                                         // null for notifications
        std::function<void(const json &value)> progress; // send $/progress
        std::function<bool()> is_canceled;               // polling cancellation
    };

    namespace detail
    {
        inline thread_local call_context *tls_ctx = nullptr;
    }

    inline const call_context *current_context() { return detail::tls_ctx; }
    inline bool is_canceled() { return detail::tls_ctx ? detail::tls_ctx->is_canceled() : false; }
    inline void report_progress(const json &value)
    {
        if (detail::tls_ctx && detail::tls_ctx->progress)
            detail::tls_ctx->progress(value);
    }

    // --- Endpoint: transport + client/server conveniences (MCP/LSP-style) ---
    class endpoint
    {
      public:
        using send_fn = std::function<void(const json &)>;
        using result_cb = std::function<void(const json &)>;
        using error_cb = std::function<void(const json &)>;

        explicit endpoint(send_fn sender) : send_(std::move(sender))
        {
            // Register built-in notifications
            disp_.add("$/cancelRequest",
                      [this](const json &params) -> json
                      {
                          // params: { id: string|number|null }
                          if (!params.is_object() || !params.contains("id"))
                              return json{};
                          auto key = key_for_id(params["id"]);
                          auto &flag = server_cancels_[key];
                          if (!flag)
                              flag = std::make_shared<std::atomic_bool>(false);
                          flag->store(true, std::memory_order_relaxed);
                          return json{}; // notification: ignored
                      });

            disp_.add("$/progress",
                      [this](const json &params) -> json
                      {
                          // params: { token: string, value: any }
                          if (!params.is_object())
                              return json{};
                          std::string token = params.value("token", std::string());
                          if (token.empty())
                              return json{};
                          auto it = progress_handlers_.find(token);
                          if (it != progress_handlers_.end() && it->second)
                              it->second(params.value("value", json{}));
                          return json{}; // notification
                      });

            disp_.add("initialize",
                      [this](const json &params) -> json
                      {
                          (void)params; // capture if needed
                          initialized_ = true;
                          return json{{"capabilities", server_capabilities_}};
                      });
        }

        // Server registration: wrap to enable context in handlers
        void add(const std::string &method, dispatcher::handler_t fn)
        {
            disp_.add(
                method,
                [this, fn = std::move(fn)](const json &params) -> json
                {
                    // Build a context hooked into this endpoint
                    json id = current_req_id_.value_or(nullptr);
                    auto id_key = key_for_id(id);
                    auto cancel_flag = server_cancels_[id_key];
                    if (!cancel_flag)
                        cancel_flag =
                            (server_cancels_[id_key] = std::make_shared<std::atomic_bool>(false));

                    // Determine progress token: either from params.progressToken or fallback to
                    // id key
                    std::string token;
                    if (params.is_object() && params.contains("progressToken") &&
                        params["progressToken"].is_string())
                    {
                        token = params["progressToken"].get<std::string>();
                    }
                    else
                    {
                        token = id_key;
                    }

                    call_context ctx{
                        id, [this, token](const json &value) { send_progress(token, value); },
                        [cancel_flag]()
                        { return cancel_flag && cancel_flag->load(std::memory_order_relaxed); }};
                    detail::tls_ctx = &ctx;
                    try
                    {
                        auto out = fn(params);
                        detail::tls_ctx = nullptr;
                        return out;
                    }
                    catch (...)
                    {
                        detail::tls_ctx = nullptr;
                        throw;
                    }
                });
        }

        // Server registration with typed parameters and return type
        template <typename ParamsT, typename ResultT>
        void add_typed(const std::string &method, std::function<ResultT(ParamsT)> fn)
        {
            // Wrap the typed function with the endpoint context
            add(method, detail::make_typed_handler<ParamsT, ResultT>(std::move(fn)));
        }

        // Server registration with no parameters
        template <typename ResultT>
        void add_no_params(const std::string &method, std::function<ResultT()> fn)
        {
            add(method, detail::make_no_params_handler<ResultT>(std::move(fn)));
        }

        // Client-side: send request (auto-generated id)
        std::string send_request(const std::string &method, const json &params, result_cb on_result,
                                 error_cb on_error)
        {
            std::string id = gen_id();
            pending_[id] = {std::move(on_result), std::move(on_error)};
            send_(make_request(id, method, params));
            return id;
        }

        // Client-side: send typed request with automatic serialization/deserialization
        template <typename ParamsT, typename ResultT>
        std::string send_request_typed(const std::string &method, const ParamsT &params,
                                       std::function<void(ResultT)> on_result,
                                       error_cb on_error = nullptr)
        {
            // Serialize params to JSON (wraps in array if needed)
            json params_json = detail::serialize_params(params);

            // Wrap the typed callback to deserialize the result
            result_cb wrapped_callback = [on_result = std::move(on_result)](const json &result_json)
            {
                if (on_result)
                {
                    ResultT typed_result = detail::deserialize_params<ResultT>(result_json);
                    on_result(std::move(typed_result));
                }
            };

            return send_request(method, params_json, std::move(wrapped_callback),
                                std::move(on_error));
        }

        // Client-side: send request with explicit id (useful for testing/cancellation ordering)
        void send_request_with_id(const std::string &id, const std::string &method,
                                  const json &params, result_cb on_result, error_cb on_error)
        {
            pending_[id] = {std::move(on_result), std::move(on_error)};
            send_(make_request(id, method, params));
        }

        // Client-side: notifications
        void send_notification(const std::string &method, const json &params = json{})
        {
            send_(make_notification(method, params));
        }

        // Client-side: typed notification
        template <typename ParamsT>
        void send_notification_typed(const std::string &method, const ParamsT &params)
        {
            json params_json = detail::serialize_params(params);
            send_notification(method, params_json);
        }

        // Progress helpers
        std::string create_progress_token() { return gen_id("tok-"); }
        void on_progress(const std::string &token, std::function<void(const json &)> cb)
        {
            progress_handlers_[token] = std::move(cb);
        }
        void send_progress(const std::string &token, const json &value)
        {
            send_(make_notification("$/progress", json{{"token", token}, {"value", value}}));
        }

        // Cancellation
        void cancel(const json &id)
        {
            send_(make_notification("$/cancelRequest", json{{"id", id}}));
        }

        // Initialize convenience
        std::string initialize(const json &params, result_cb on_result, error_cb on_error)
        {
            return send_request("initialize", params, std::move(on_result), std::move(on_error));
        }

        void set_server_capabilities(json caps) { server_capabilities_ = std::move(caps); }
        bool is_initialized() const { return initialized_; }

        // Incoming single or batch message entrypoint
        void receive(const json &msg)
        {
            if (msg.is_array())
            {
                if (msg.empty())
                {
                    send_(make_error(nullptr, invalid_request));
                    return;
                }
                std::vector<json> outs;
                outs.reserve(msg.size());
                for (const auto &m : msg)
                {
                    // Gather responses but do not emit immediately
                    std::optional<json> id_opt = m.contains("id")
                                                     ? std::optional<json>(m["id"])
                                                     : std::optional<json>(json(nullptr));
                    current_req_id_ = id_opt;
                    auto r = disp_.handle_single(m);
                    current_req_id_.reset();
                    if (r)
                        outs.push_back(*r);
                    // Clean up cancellation flag for requests
                    if (m.contains("id"))
                    {
                        auto id_key = key_for_id(m["id"]);
                        server_cancels_.erase(id_key);
                    }
                }
                if (!outs.empty())
                    send_(json(outs));
                return;
            }
            if (is_response(msg))
            {
                handle_incoming_response(msg);
                return;
            }
            // Request/notification path
            std::optional<json> id_opt;
            if (msg.contains("id"))
                id_opt = msg["id"];
            else
                id_opt = json(nullptr);
            // Set current id for context
            current_req_id_ = id_opt;
            auto resp = disp_.handle_single(msg);
            current_req_id_.reset();
            if (resp)
            {
                send_(*resp);
                // Clean up cancellation flag for completed request
                if (msg.contains("id"))
                {
                    auto id_key = key_for_id(msg["id"]);
                    server_cancels_.erase(id_key);
                }
            }
        }

      private:
        // Helper: normalize id into string key
        static std::string key_for_id(const json &id)
        {
            if (id.is_string())
                return id.get<std::string>();
            return id.dump();
        }

        // Incoming responses
        void handle_incoming_response(const json &r)
        {
            std::string key = key_for_id(r.at("id"));
            auto it = pending_.find(key);
            if (it == pending_.end())
                return; // unknown/late
            auto [on_ok, on_err] = it->second;
            pending_.erase(it);
            if (r.contains("result"))
            {
                if (on_ok)
                    on_ok(r["result"]);
            }
            else if (r.contains("error"))
            {
                if (on_err)
                    on_err(r["error"]);
            }
        }

        std::string gen_id(const std::string &prefix = "req-")
        {
            std::ostringstream oss;
            oss << prefix << (++id_counter_);
            return oss.str();
        }

        send_fn send_;
        dispatcher disp_;
        std::map<std::string, std::pair<result_cb, error_cb>> pending_;
        std::unordered_map<std::string, std::shared_ptr<std::atomic_bool>> server_cancels_;
        std::unordered_map<std::string, std::function<void(const json &)>> progress_handlers_;
        std::optional<json> current_req_id_;
        json server_capabilities_ = json::object();
        bool initialized_ = false;
        size_t id_counter_ = 0;
    };

} // namespace pooriayousefi
