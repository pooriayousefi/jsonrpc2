/*
 * JSON-RPC 2.0 Library - Unit Tests
 *
 * Comprehensive tests for the jsonrpc.hpp library.
 * Tests all core functionality: validation, builders, dispatcher, endpoint.
 */

#include "../include/jsonrpc.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace pooriayousefi;
using json = nlohmann::json;

// Test helper macros
#define ASSERT(expr)                                                                               \
    do                                                                                             \
    {                                                                                              \
        if (!(expr))                                                                               \
        {                                                                                          \
            std::cerr << "  ✗ Assertion failed: " << #expr << " at " << __FILE__ << ":"            \
                      << __LINE__ << std::endl;                                                    \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

#define TEST(name)                                                                                 \
    bool test_##name();                                                                            \
    bool test_##name()

#define RUN_TEST(name)                                                                             \
    do                                                                                             \
    {                                                                                              \
        std::cout << "  Running: " << #name << "...";                                              \
        if (test_##name())                                                                         \
        {                                                                                          \
            std::cout << " ✓" << std::endl;                                                        \
            passed++;                                                                              \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            std::cout << " ✗" << std::endl;                                                        \
            failed++;                                                                              \
        }                                                                                          \
        total++;                                                                                   \
    } while (0)

// ============================================================================
// Validation Tests
// ============================================================================

TEST(valid_id_types)
{
    ASSERT(valid_id_type(json(nullptr)));
    ASSERT(valid_id_type(json("test-id")));
    ASSERT(valid_id_type(json(42)));
    ASSERT(valid_id_type(json(42u)));
    ASSERT(!valid_id_type(json(3.14))); // float not allowed
    ASSERT(!valid_id_type(json::array()));
    ASSERT(!valid_id_type(json::object()));
    return true;
}

TEST(validate_request_valid)
{
    json req = {{"jsonrpc", "2.0"}, {"method", "test"}, {"id", 1}};
    std::string why;
    ASSERT(validate_request(req, &why));

    // Without id (notification)
    json notif = {{"jsonrpc", "2.0"}, {"method", "test"}};
    ASSERT(validate_request(notif, &why));

    // With params
    json with_params = {{"jsonrpc", "2.0"}, {"method", "test"}, {"params", json::array()}};
    ASSERT(validate_request(with_params, &why));

    json with_obj_params = {{"jsonrpc", "2.0"}, {"method", "test"}, {"params", json::object()}};
    ASSERT(validate_request(with_obj_params, &why));

    return true;
}

TEST(validate_request_invalid)
{
    std::string why;

    // Not an object
    ASSERT(!validate_request(json("string"), &why));

    // Missing jsonrpc
    json missing_rpc = {{"method", "test"}};
    ASSERT(!validate_request(missing_rpc, &why));

    // Wrong jsonrpc version
    json wrong_ver = {{"jsonrpc", "1.0"}, {"method", "test"}};
    ASSERT(!validate_request(wrong_ver, &why));

    // Missing method
    json no_method = {{"jsonrpc", "2.0"}};
    ASSERT(!validate_request(no_method, &why));

    // Method not string
    json bad_method = {{"jsonrpc", "2.0"}, {"method", 123}};
    ASSERT(!validate_request(bad_method, &why));

    // Invalid id type
    json bad_id = {{"jsonrpc", "2.0"}, {"method", "test"}, {"id", 3.14}};
    ASSERT(!validate_request(bad_id, &why));

    // Invalid params type
    json bad_params = {{"jsonrpc", "2.0"}, {"method", "test"}, {"params", "string"}};
    ASSERT(!validate_request(bad_params, &why));

    return true;
}

TEST(validate_response_valid)
{
    std::string why;

    // Success response
    json success = {{"jsonrpc", "2.0"}, {"id", 1}, {"result", "test"}};
    ASSERT(validate_response(success, &why));

    // Error response
    json error_resp = {{"jsonrpc", "2.0"},
                       {"id", 1},
                       {"error", {{"code", -32600}, {"message", "Invalid Request"}}}};
    ASSERT(validate_response(error_resp, &why));

    return true;
}

TEST(validate_response_invalid)
{
    std::string why;

    // Missing id
    json no_id = {{"jsonrpc", "2.0"}, {"result", "test"}};
    ASSERT(!validate_response(no_id, &why));

    // Both result and error
    json both = {{"jsonrpc", "2.0"}, {"id", 1}, {"result", "test"}, {"error", json::object()}};
    ASSERT(!validate_response(both, &why));

    // Neither result nor error
    json neither = {{"jsonrpc", "2.0"}, {"id", 1}};
    ASSERT(!validate_response(neither, &why));

    // Invalid error object
    json bad_error = {{"jsonrpc", "2.0"}, {"id", 1}, {"error", "string"}};
    ASSERT(!validate_response(bad_error, &why));

    return true;
}

TEST(is_request_detection)
{
    json req = {{"jsonrpc", "2.0"}, {"method", "test"}, {"id", 1}};
    ASSERT(is_request(req));

    json notif = {{"jsonrpc", "2.0"}, {"method", "test"}};
    ASSERT(is_request(notif));

    json response = {{"jsonrpc", "2.0"}, {"id", 1}, {"result", "test"}};
    ASSERT(!is_request(response));

    return true;
}

TEST(is_notification_detection)
{
    json notif = {{"jsonrpc", "2.0"}, {"method", "test"}};
    ASSERT(is_notification(notif));

    json req = {{"jsonrpc", "2.0"}, {"method", "test"}, {"id", 1}};
    ASSERT(!is_notification(req));

    return true;
}

TEST(is_response_detection)
{
    json response = {{"jsonrpc", "2.0"}, {"id", 1}, {"result", "test"}};
    ASSERT(is_response(response));

    json error_resp = {{"jsonrpc", "2.0"},
                       {"id", 1},
                       {"error", {{"code", -32600}, {"message", "Invalid Request"}}}};
    ASSERT(is_response(error_resp));

    json req = {{"jsonrpc", "2.0"}, {"method", "test"}, {"id", 1}};
    ASSERT(!is_response(req));

    return true;
}

// ============================================================================
// Builder Tests
// ============================================================================

TEST(make_request_with_id)
{
    json req = make_request(1, "test_method", json::array({1, 2, 3}));
    ASSERT(req["jsonrpc"] == "2.0");
    ASSERT(req["method"] == "test_method");
    ASSERT(req["id"] == 1);
    ASSERT(req["params"].is_array());
    ASSERT(req["params"].size() == 3);
    return true;
}

TEST(make_request_with_string_id)
{
    json req = make_request(std::string("test-id"), "method", json::object());
    ASSERT(req["id"] == "test-id");
    return true;
}

TEST(make_notification_test)
{
    json notif = make_notification("notify", json::object({{"key", "value"}}));
    ASSERT(notif["jsonrpc"] == "2.0");
    ASSERT(notif["method"] == "notify");
    ASSERT(!notif.contains("id"));
    ASSERT(notif["params"]["key"] == "value");
    return true;
}

TEST(make_result_test)
{
    json result = make_result(42, json("success"));
    ASSERT(result["jsonrpc"] == "2.0");
    ASSERT(result["id"] == 42);
    ASSERT(result["result"] == "success");
    ASSERT(!result.contains("error"));
    return true;
}

TEST(make_error_test)
{
    error err{-32600, "Invalid Request", json{{"detail", "bad format"}}};
    json error_resp = make_error(1, err);
    ASSERT(error_resp["jsonrpc"] == "2.0");
    ASSERT(error_resp["id"] == 1);
    ASSERT(error_resp["error"]["code"] == -32600);
    ASSERT(error_resp["error"]["message"] == "Invalid Request");
    ASSERT(error_resp["error"]["data"]["detail"] == "bad format");
    return true;
}

// ============================================================================
// Dispatcher Tests
// ============================================================================

TEST(dispatcher_simple_method)
{
    dispatcher d;
    d.add("add",
          [](const json &params) -> json
          {
              int a = params[0].get<int>();
              int b = params[1].get<int>();
              return a + b;
          });

    json req = make_request(1, "add", json::array({5, 3}));
    auto resp = d.handle_single(req);
    ASSERT(resp.has_value());
    ASSERT((*resp)["result"] == 8);
    ASSERT((*resp)["id"] == 1);
    return true;
}

TEST(dispatcher_notification)
{
    dispatcher d;
    bool called = false;
    d.add("log",
          [&called](const json &params) -> json
          {
              called = true;
              return nullptr;
          });

    json notif = make_notification("log", json::object({{"message", "test"}}));
    auto resp = d.handle_single(notif);
    ASSERT(!resp.has_value()); // notifications return no response
    ASSERT(called);
    return true;
}

TEST(dispatcher_method_not_found)
{
    dispatcher d;
    json req = make_request(1, "unknown_method", json::array());
    auto resp = d.handle_single(req);
    ASSERT(resp.has_value());
    ASSERT((*resp)["error"]["code"] == -32601);
    return true;
}

TEST(dispatcher_rpc_exception)
{
    dispatcher d;
    d.add("fail", [](const json &) -> json
          { throw rpc_exception(error{-32000, "Custom error", json{{"info", "test"}}}); });

    json req = make_request(1, "fail", json::array());
    auto resp = d.handle_single(req);
    ASSERT(resp.has_value());
    ASSERT((*resp)["error"]["code"] == -32000);
    ASSERT((*resp)["error"]["message"] == "Custom error");
    return true;
}

TEST(dispatcher_standard_exception)
{
    dispatcher d;
    d.add("crash", [](const json &) -> json { throw std::runtime_error("Something went wrong"); });

    json req = make_request(1, "crash", json::array());
    auto resp = d.handle_single(req);
    ASSERT(resp.has_value());
    ASSERT((*resp)["error"]["code"] == -32603); // internal error
    return true;
}

TEST(dispatcher_batch_requests)
{
    dispatcher d;
    d.add("add",
          [](const json &params) -> json { return params[0].get<int>() + params[1].get<int>(); });
    d.add("subtract",
          [](const json &params) -> json { return params[0].get<int>() - params[1].get<int>(); });

    json batch = json::array({make_request(1, "add", json::array({5, 3})),
                              make_request(2, "subtract", json::array({10, 4}))});

    auto resp = d.handle(batch);
    ASSERT(resp.has_value());
    ASSERT((*resp).is_array());
    ASSERT((*resp).size() == 2);
    return true;
}

TEST(dispatcher_empty_batch)
{
    dispatcher d;
    json empty_batch = json::array();
    auto resp = d.handle(empty_batch);
    ASSERT(resp.has_value());
    ASSERT((*resp)["error"]["code"] == -32600); // invalid request
    return true;
}

TEST(dispatcher_all_notifications_batch)
{
    dispatcher d;
    int count = 0;
    d.add("log",
          [&count](const json &) -> json
          {
              count++;
              return nullptr;
          });

    json batch = json::array(
        {make_notification("log", json::object()), make_notification("log", json::object())});

    auto resp = d.handle(batch);
    ASSERT(!resp.has_value()); // all notifications, no response
    ASSERT(count == 2);
    return true;
}

// ============================================================================
// Endpoint Tests
// ============================================================================

TEST(endpoint_basic_communication)
{
    std::vector<json> sent_messages;
    endpoint ep([&sent_messages](const json &msg) { sent_messages.push_back(msg); });

    // Add a simple method
    ep.add("echo", [](const json &params) -> json { return params; });

    // Test request with array params
    json req = make_request(1, "echo", json::array({"hello"}));
    ep.receive(req);

    ASSERT(sent_messages.size() == 1);
    ASSERT(sent_messages[0]["result"].is_array());
    ASSERT(sent_messages[0]["result"][0] == "hello");
    return true;
}

TEST(endpoint_client_request)
{
    json last_sent;
    endpoint ep([&last_sent](const json &msg) { last_sent = msg; });

    bool result_received = false;
    ep.send_request(
        "test_method", json::array({1, 2, 3}),
        [&result_received](const json &result) { result_received = true; }, [](const json &) {});

    ASSERT(last_sent["method"] == "test_method");
    ASSERT(last_sent.contains("id"));
    return true;
}

TEST(endpoint_notification_send)
{
    json last_sent;
    endpoint ep([&last_sent](const json &msg) { last_sent = msg; });

    ep.send_notification("notify", json::object({{"key", "value"}}));

    ASSERT(last_sent["method"] == "notify");
    ASSERT(!last_sent.contains("id"));
    return true;
}

TEST(endpoint_initialize)
{
    json last_sent;
    endpoint ep([&last_sent](const json &msg) { last_sent = msg; });

    ep.set_server_capabilities(json{{"textDocumentSync", 1}});

    json init_req = make_request(1, "initialize", json::object());
    ep.receive(init_req);

    ASSERT(last_sent["result"]["capabilities"]["textDocumentSync"] == 1);
    ASSERT(ep.is_initialized());
    return true;
}

TEST(endpoint_cancellation)
{
    std::vector<json> sent;
    endpoint ep([&sent](const json &msg) { sent.push_back(msg); });

    ep.cancel(42);

    ASSERT(sent.size() == 1);
    ASSERT(sent[0]["method"] == "$/cancelRequest");
    ASSERT(sent[0]["params"]["id"] == 42);
    return true;
}

TEST(endpoint_progress)
{
    std::vector<json> sent;
    endpoint ep([&sent](const json &msg) { sent.push_back(msg); });

    std::string token = ep.create_progress_token();
    ep.send_progress(token, json{{"percentage", 50}});

    ASSERT(sent.size() == 1);
    ASSERT(sent[0]["method"] == "$/progress");
    ASSERT(sent[0]["params"]["token"] == token);
    ASSERT(sent[0]["params"]["value"]["percentage"] == 50);
    return true;
}

TEST(endpoint_response_callback)
{
    endpoint ep([](const json &) {});

    bool result_called = false;
    json received_result;

    ep.send_request(
        "test", json::array(),
        [&](const json &result)
        {
            result_called = true;
            received_result = result;
        },
        [](const json &) {});

    // Simulate receiving a response
    json response = {{"jsonrpc", "2.0"}, {"id", "req-1"}, {"result", "success"}};
    ep.receive(response);

    ASSERT(result_called);
    ASSERT(received_result == "success");
    return true;
}

// ============================================================================
// Error Object Tests
// ============================================================================

TEST(standard_errors)
{
    ASSERT(parse_error.code == -32700);
    ASSERT(invalid_request.code == -32600);
    ASSERT(method_not_found.code == -32601);
    ASSERT(invalid_params.code == -32602);
    ASSERT(internal_error.code == -32603);
    return true;
}

TEST(make_error_object_test)
{
    error e{-32000, "Custom", json{{"detail", "info"}}};
    json obj = make_error_object(e);
    ASSERT(obj["code"] == -32000);
    ASSERT(obj["message"] == "Custom");
    ASSERT(obj["data"]["detail"] == "info");
    return true;
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(full_request_response_cycle)
{
    // Setup two endpoints communicating with each other
    std::vector<json> client_to_server;
    std::vector<json> server_to_client;

    endpoint server([&server_to_client](const json &msg) { server_to_client.push_back(msg); });
    endpoint client([&client_to_server](const json &msg) { client_to_server.push_back(msg); });

    // Server implements a method
    server.add("multiply", [](const json &params) -> json
               { return params[0].get<int>() * params[1].get<int>(); });

    // Client sends request
    bool got_result = false;
    int result_value = 0;
    client.send_request(
        "multiply", json::array({6, 7}),
        [&](const json &result)
        {
            got_result = true;
            result_value = result.get<int>();
        },
        [](const json &) {});

    // Server receives and processes
    ASSERT(client_to_server.size() == 1);
    server.receive(client_to_server[0]);

    // Client receives response
    ASSERT(server_to_client.size() == 1);
    client.receive(server_to_client[0]);

    ASSERT(got_result);
    ASSERT(result_value == 42);
    return true;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int run_unit_tests()
{
    int total = 0, passed = 0, failed = 0;

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Running Unit Tests\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    // Validation tests
    std::cout << "Validation Tests:\n";
    RUN_TEST(valid_id_types);
    RUN_TEST(validate_request_valid);
    RUN_TEST(validate_request_invalid);
    RUN_TEST(validate_response_valid);
    RUN_TEST(validate_response_invalid);
    RUN_TEST(is_request_detection);
    RUN_TEST(is_notification_detection);
    RUN_TEST(is_response_detection);

    // Builder tests
    std::cout << "\nBuilder Tests:\n";
    RUN_TEST(make_request_with_id);
    RUN_TEST(make_request_with_string_id);
    RUN_TEST(make_notification_test);
    RUN_TEST(make_result_test);
    RUN_TEST(make_error_test);

    // Dispatcher tests
    std::cout << "\nDispatcher Tests:\n";
    RUN_TEST(dispatcher_simple_method);
    RUN_TEST(dispatcher_notification);
    RUN_TEST(dispatcher_method_not_found);
    RUN_TEST(dispatcher_rpc_exception);
    RUN_TEST(dispatcher_standard_exception);
    RUN_TEST(dispatcher_batch_requests);
    RUN_TEST(dispatcher_empty_batch);
    RUN_TEST(dispatcher_all_notifications_batch);

    // Endpoint tests
    std::cout << "\nEndpoint Tests:\n";
    RUN_TEST(endpoint_basic_communication);
    RUN_TEST(endpoint_client_request);
    RUN_TEST(endpoint_notification_send);
    RUN_TEST(endpoint_initialize);
    RUN_TEST(endpoint_cancellation);
    RUN_TEST(endpoint_progress);
    RUN_TEST(endpoint_response_callback);

    // Error tests
    std::cout << "\nError Object Tests:\n";
    RUN_TEST(standard_errors);
    RUN_TEST(make_error_object_test);

    // Integration tests
    std::cout << "\nIntegration Tests:\n";
    RUN_TEST(full_request_response_cycle);

    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Test Results:\n";
    std::cout << "  Total:  " << total << "\n";
    std::cout << "  Passed: " << passed << "\n";
    std::cout << "  Failed: " << failed << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    return (failed == 0) ? 0 : 1;
}
