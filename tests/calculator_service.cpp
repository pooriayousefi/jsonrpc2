/*
 * JSON-RPC 2.0 Calculator Service Example
 *
 * Demonstrates a real-world calculator service using JSON-RPC 2.0.
 * Includes:
 * - Basic arithmetic operations (add, subtract, multiply, divide)
 * - Advanced operations (power, sqrt, factorial)
 * - Memory operations (store, recall, clear)
 * - Error handling for invalid operations
 */

#include "../include/jsonrpc.hpp"
#include <cmath>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

using namespace pooriayousefi;
using json = nlohmann::json;

class CalculatorService
{
  private:
    std::map<std::string, double> memory_;
    dispatcher dispatcher_;
    std::vector<json> message_log_;

    // Helper to get number from params
    double get_param(const json &params, size_t index, const std::string &name = "")
    {
        if (params.is_array() && index < params.size())
        {
            if (!params[index].is_number())
            {
                throw rpc_exception(
                    error{-32602, "Invalid params", json{{"detail", name + " must be a number"}}});
            }
            return params[index].get<double>();
        }
        throw rpc_exception(
            error{-32602, "Invalid params", json{{"detail", "Missing parameter: " + name}}});
    }

    // Factorial helper
    int64_t factorial(int n)
    {
        if (n < 0)
            throw rpc_exception(error{-32000, "Invalid argument",
                                      json{{"detail", "Factorial requires non-negative integer"}}});
        if (n > 20)
            throw rpc_exception(
                error{-32000, "Overflow", json{{"detail", "Factorial too large (max 20)"}}});
        int64_t result = 1;
        for (int i = 2; i <= n; ++i)
            result *= i;
        return result;
    }

  public:
    CalculatorService()
    {
        // Basic arithmetic operations
        dispatcher_.add("add",
                        [this](const json &params) -> json
                        {
                            double a = get_param(params, 0, "a");
                            double b = get_param(params, 1, "b");
                            return a + b;
                        });

        dispatcher_.add("subtract",
                        [this](const json &params) -> json
                        {
                            double a = get_param(params, 0, "minuend");
                            double b = get_param(params, 1, "subtrahend");
                            return a - b;
                        });

        dispatcher_.add("multiply",
                        [this](const json &params) -> json
                        {
                            double a = get_param(params, 0, "a");
                            double b = get_param(params, 1, "b");
                            return a * b;
                        });

        dispatcher_.add("divide",
                        [this](const json &params) -> json
                        {
                            double a = get_param(params, 0, "dividend");
                            double b = get_param(params, 1, "divisor");
                            if (b == 0.0)
                            {
                                throw rpc_exception(
                                    error{-32000, "Division by zero",
                                          json{{"detail", "Cannot divide by zero"}}});
                            }
                            return a / b;
                        });

        // Advanced operations
        dispatcher_.add("power",
                        [this](const json &params) -> json
                        {
                            double base = get_param(params, 0, "base");
                            double exponent = get_param(params, 1, "exponent");
                            return std::pow(base, exponent);
                        });

        dispatcher_.add(
            "sqrt",
            [this](const json &params) -> json
            {
                double value = get_param(params, 0, "value");
                if (value < 0)
                {
                    throw rpc_exception(
                        error{-32000, "Invalid argument",
                              json{{"detail", "Cannot take square root of negative number"}}});
                }
                return std::sqrt(value);
            });

        dispatcher_.add("factorial",
                        [this](const json &params) -> json
                        {
                            double value = get_param(params, 0, "n");
                            int n = static_cast<int>(value);
                            if (value != n)
                            {
                                throw rpc_exception(
                                    error{-32602, "Invalid params",
                                          json{{"detail", "Factorial requires integer"}}});
                            }
                            return factorial(n);
                        });

        // Memory operations
        dispatcher_.add("memory_store",
                        [this](const json &params) -> json
                        {
                            std::string key = "default";
                            if (params.is_object() && params.contains("key"))
                            {
                                key = params["key"].get<std::string>();
                            }
                            double value = params.is_object() ? params["value"].get<double>()
                                                              : get_param(params, 0, "value");
                            memory_[key] = value;
                            return json{{"stored", value}, {"key", key}};
                        });

        dispatcher_.add("memory_recall",
                        [this](const json &params) -> json
                        {
                            std::string key = "default";
                            if (params.is_object() && params.contains("key"))
                            {
                                key = params["key"].get<std::string>();
                            }
                            else if (params.is_array() && params.size() > 0)
                            {
                                key = params[0].get<std::string>();
                            }
                            auto it = memory_.find(key);
                            if (it == memory_.end())
                            {
                                throw rpc_exception(
                                    error{-32001, "Memory not found", json{{"key", key}}});
                            }
                            return it->second;
                        });

        dispatcher_.add("memory_clear",
                        [this](const json &params) -> json
                        {
                            (void)params;
                            size_t count = memory_.size();
                            memory_.clear();
                            return json{{"cleared", count}};
                        });

        // Utility methods
        dispatcher_.add("get_constants",
                        [](const json &params) -> json
                        {
                            (void)params;
                            return json{
                                {"pi", M_PI}, {"e", M_E}, {"sqrt2", M_SQRT2}, {"ln2", M_LN2}};
                        });

        dispatcher_.add(
            "get_capabilities",
            [](const json &params) -> json
            {
                (void)params;
                return json{
                    {"operations",
                     {"add", "subtract", "multiply", "divide", "power", "sqrt", "factorial"}},
                    {"memory", {"memory_store", "memory_recall", "memory_clear"}},
                    {"utility", {"get_constants", "get_capabilities"}}};
            });
    }

    json handle_request(const json &request)
    {
        message_log_.push_back(request);
        auto response = dispatcher_.handle(request);
        if (response)
        {
            message_log_.push_back(*response);
            return *response;
        }
        return json(); // notification, no response
    }

    const std::vector<json> &get_log() const { return message_log_; }
};

// ============================================================================
// Demo and Test Functions
// ============================================================================

void run_calculator_demo(CalculatorService &calc)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Calculator Service Demo\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    // Basic arithmetic
    std::cout << "1. Basic Arithmetic Operations:\n";
    {
        json req = make_request(1, "add", json::array({10, 5}));
        std::cout << "  Request:  " << req.dump() << "\n";
        json resp = calc.handle_request(req);
        std::cout << "  Response: " << resp.dump() << "\n";
        std::cout << "  Result: 10 + 5 = " << resp["result"] << "\n\n";
    }

    {
        json req = make_request(2, "multiply", json::array({7, 6}));
        std::cout << "  Request:  " << req.dump() << "\n";
        json resp = calc.handle_request(req);
        std::cout << "  Response: " << resp.dump() << "\n";
        std::cout << "  Result: 7 × 6 = " << resp["result"] << "\n\n";
    }

    {
        json req = make_request(3, "divide", json::array({100, 4}));
        std::cout << "  Request:  " << req.dump() << "\n";
        json resp = calc.handle_request(req);
        std::cout << "  Response: " << resp.dump() << "\n";
        std::cout << "  Result: 100 ÷ 4 = " << resp["result"] << "\n\n";
    }

    // Advanced operations
    std::cout << "2. Advanced Operations:\n";
    {
        json req = make_request(4, "power", json::array({2, 10}));
        std::cout << "  Request:  " << req.dump() << "\n";
        json resp = calc.handle_request(req);
        std::cout << "  Response: " << resp.dump() << "\n";
        std::cout << "  Result: 2^10 = " << resp["result"] << "\n\n";
    }

    {
        json req = make_request(5, "sqrt", json::array({144}));
        std::cout << "  Request:  " << req.dump() << "\n";
        json resp = calc.handle_request(req);
        std::cout << "  Response: " << resp.dump() << "\n";
        std::cout << "  Result: √144 = " << resp["result"] << "\n\n";
    }

    {
        json req = make_request(6, "factorial", json::array({5}));
        std::cout << "  Request:  " << req.dump() << "\n";
        json resp = calc.handle_request(req);
        std::cout << "  Response: " << resp.dump() << "\n";
        std::cout << "  Result: 5! = " << resp["result"] << "\n\n";
    }

    // Memory operations
    std::cout << "3. Memory Operations:\n";
    {
        json req =
            make_request(7, "memory_store", json::object({{"key", "result"}, {"value", 42.5}}));
        std::cout << "  Storing value in memory...\n";
        std::cout << "  Request:  " << req.dump() << "\n";
        json resp = calc.handle_request(req);
        std::cout << "  Response: " << resp.dump() << "\n\n";
    }

    {
        json req = make_request(8, "memory_recall", json::object({{"key", "result"}}));
        std::cout << "  Recalling value from memory...\n";
        std::cout << "  Request:  " << req.dump() << "\n";
        json resp = calc.handle_request(req);
        std::cout << "  Response: " << resp.dump() << "\n";
        std::cout << "  Recalled: " << resp["result"] << "\n\n";
    }

    // Error handling
    std::cout << "4. Error Handling:\n";
    {
        json req = make_request(9, "divide", json::array({10, 0}));
        std::cout << "  Attempting division by zero...\n";
        std::cout << "  Request:  " << req.dump() << "\n";
        json resp = calc.handle_request(req);
        std::cout << "  Response: " << resp.dump() << "\n";
        std::cout << "  Error: " << resp["error"]["message"] << "\n\n";
    }

    // Batch request
    std::cout << "5. Batch Request:\n";
    {
        json batch = json::array({make_request(10, "add", json::array({1, 2})),
                                  make_request(11, "multiply", json::array({3, 4})),
                                  make_request(12, "subtract", json::array({10, 5}))});
        std::cout << "  Batch Request: " << batch.dump() << "\n";
        json resp = calc.handle_request(batch);
        std::cout << "  Batch Response: " << resp.dump() << "\n\n";
    }

    // Constants
    std::cout << "6. Mathematical Constants:\n";
    {
        json req = make_request(13, "get_constants", json::object());
        std::cout << "  Request:  " << req.dump() << "\n";
        json resp = calc.handle_request(req);
        std::cout << "  Response: " << resp.dump() << "\n\n";
    }
}

// ============================================================================
// Main Entry Point
// ============================================================================

int run_calculator_service()
{
    try
    {
        CalculatorService calc;
        run_calculator_demo(calc);

        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "Calculator service completed successfully!\n";
        std::cout << "Total messages processed: " << calc.get_log().size() << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in calculator service: " << e.what() << std::endl;
        return 1;
    }
}
