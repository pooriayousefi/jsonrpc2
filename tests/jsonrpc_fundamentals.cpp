/*
 * JSON-RPC 2.0 Fundamentals Tutorial
 *
 * Comprehensive tutorial covering JSON-RPC 2.0 specification fundamentals.
 * Demonstrates requests, responses, notifications, errors, and batch operations.
 */

#include "../include/jsonrpc.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace pooriayousefi;
using json = nlohmann::json;

static void print_section(const std::string &title)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << title << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
}

void jsonrpc_request_structure()
{
    print_section("1. Request Structure");

    std::cout << "\nBasic Request with Integer ID:\n";
    json req1 = make_request(1, "subtract", json::array({42, 23}));
    std::cout << req1.dump(2) << "\n";
    std::cout << "\nRequired fields:\n";
    std::cout << "  - jsonrpc: \"2.0\" (protocol version)\n";
    std::cout << "  - method: \"subtract\" (method name)\n";
    std::cout << "  - params: [42, 23] (parameters)\n";
    std::cout << "  - id: 1 (request identifier)\n";

    std::cout << "\n\nRequest with String ID:\n";
    json req2 =
        make_request(std::string("req-abc-123"), "getData", json::object({{"key", "value"}}));
    std::cout << req2.dump(2) << "\n";

    std::cout << "\n\nRequest with Positional Parameters:\n";
    json req3 = make_request(2, "add", json::array({5, 3}));
    std::cout << req3.dump(2) << "\n";

    std::cout << "\n\nRequest with Named Parameters:\n";
    json req4 = make_request(3, "divide", json::object({{"dividend", 20}, {"divisor", 4}}));
    std::cout << req4.dump(2) << "\n";

    std::cout << "\n\nRequest without Parameters:\n";
    json req5 = make_request(4, "getServerTime", json::object());
    std::cout << req5.dump(2) << "\n";
}

void jsonrpc_response_structure()
{
    print_section("2. Response Structure");

    std::cout << "\nSuccess Response:\n";
    json resp1 = make_result(1, json(19));
    std::cout << resp1.dump(2) << "\n";
    std::cout << "\nRequired fields:\n";
    std::cout << "  - jsonrpc: \"2.0\"\n";
    std::cout << "  - result: 19 (computation result)\n";
    std::cout << "  - id: 1 (matches request id)\n";

    std::cout << "\n\nResponse with Complex Result:\n";
    json complex_result = {{"status", "ok"}, {"data", json::array({1, 2, 3})}, {"count", 3}};
    json resp2 = make_result(2, complex_result);
    std::cout << resp2.dump(2) << "\n";

    std::cout << "\n\nResponse with Null Result:\n";
    json resp3 = make_result(3, nullptr);
    std::cout << resp3.dump(2) << "\n";
    std::cout << "\nNote: null is a valid result value!\n";
}

void jsonrpc_error_structure()
{
    print_section("3. Error Responses");

    std::cout << "\nError Response Structure:\n";
    error err1{-32601, "Method not found", json{{"attempted", "unknownMethod"}}};
    json err_resp1 = make_error(1, err1);
    std::cout << err_resp1.dump(2) << "\n";
    std::cout << "\nError object fields:\n";
    std::cout << "  - code: -32601 (error code)\n";
    std::cout << "  - message: \"Method not found\" (human-readable)\n";
    std::cout << "  - data: additional information (optional)\n";

    std::cout << "\n\nStandard Error Codes:\n";
    std::cout << "  -32700: Parse error (invalid JSON)\n";
    std::cout << "  -32600: Invalid Request\n";
    std::cout << "  -32601: Method not found\n";
    std::cout << "  -32602: Invalid params\n";
    std::cout << "  -32603: Internal error\n";
    std::cout << "  -32000 to -32099: Server-defined errors\n";

    std::cout << "\n\nCustom Error Example:\n";
    error custom_err{-32000, "Database connection failed",
                     json{{"host", "db.example.com"}, {"reason", "timeout"}}};
    json err_resp2 = make_error(5, custom_err);
    std::cout << err_resp2.dump(2) << "\n";

    std::cout << "\n\nError with Minimal Data:\n";
    error minimal_err{-32602, "Invalid params"};
    json err_resp3 = make_error(6, minimal_err);
    std::cout << err_resp3.dump(2) << "\n";
}

void jsonrpc_notifications()
{
    print_section("4. Notifications");

    std::cout << "\nNotification (no ID field):\n";
    json notif1 =
        make_notification("log", json::object({{"level", "info"}, {"message", "Server started"}}));
    std::cout << notif1.dump(2) << "\n";
    std::cout << "\nKey characteristics:\n";
    std::cout << "  - No 'id' field (this is the key difference!)\n";
    std::cout << "  - Server MUST NOT respond\n";
    std::cout << "  - Used for fire-and-forget messages\n";

    std::cout << "\n\nAnother Notification Example:\n";
    json notif2 = make_notification(
        "userConnected", json::object({{"userId", 12345}, {"timestamp", "2025-10-13T10:00:00Z"}}));
    std::cout << notif2.dump(2) << "\n";

    std::cout << "\n\nNotification with Array Parameters:\n";
    json notif3 = make_notification("updateProgress", json::array({50, "Processing..."}));
    std::cout << notif3.dump(2) << "\n";

    std::cout << "\n\nUse cases for notifications:\n";
    std::cout << "  - Logging and monitoring\n";
    std::cout << "  - Event broadcasting\n";
    std::cout << "  - Progress updates\n";
    std::cout << "  - One-way messages\n";
}

void jsonrpc_batch_requests()
{
    print_section("5. Batch Requests");

    std::cout << "\nBatch Request (multiple operations):\n";
    json batch = json::array({make_request(1, "sum", json::array({1, 2, 4})),
                              make_request(2, "subtract", json::array({42, 23})),
                              make_request(3, "get_data", json::object())});
    std::cout << batch.dump(2) << "\n";

    std::cout << "\n\nBatch with Mixed Requests and Notifications:\n";
    json mixed_batch =
        json::array({make_request(1, "getData", json::array()),
                     make_notification("log", json::object({{"msg", "Fetching data"}})),
                     make_request(2, "processData", json::array({42})),
                     make_notification("log", json::object({{"msg", "Processing complete"}}))});
    std::cout << mixed_batch.dump(2) << "\n";

    std::cout << "\n\nExpected Batch Response:\n";
    std::cout << "  - Array of response objects\n";
    std::cout << "  - One response per request (not notifications!)\n";
    std::cout << "  - Responses may be in any order\n";
    std::cout << "  - If all are notifications, no response at all\n";

    json batch_response =
        json::array({make_result(1, json::array({1, 2, 3})), make_result(2, json(42))});
    std::cout << "\nExample batch response:\n" << batch_response.dump(2) << "\n";
}

void jsonrpc_validation_examples()
{
    print_section("6. Validation");

    std::cout << "\nValidating Requests:\n";

    json valid_req = make_request(1, "test", json::array());
    std::string why;
    bool is_valid = validate_request(valid_req, &why);
    std::cout << "Valid request: " << valid_req.dump() << "\n";
    std::cout << "Is valid? " << (is_valid ? "YES" : "NO") << "\n";

    json invalid_req1 = {{"jsonrpc", "1.0"}, {"method", "test"}, {"id", 1}};
    is_valid = validate_request(invalid_req1, &why);
    std::cout << "\nInvalid request (wrong version): " << invalid_req1.dump() << "\n";
    std::cout << "Is valid? " << (is_valid ? "YES" : "NO") << " - Reason: " << why << "\n";

    json invalid_req2 = {{"jsonrpc", "2.0"}, {"id", 1}};
    is_valid = validate_request(invalid_req2, &why);
    std::cout << "\nInvalid request (missing method): " << invalid_req2.dump() << "\n";
    std::cout << "Is valid? " << (is_valid ? "YES" : "NO") << " - Reason: " << why << "\n";

    std::cout << "\n\nValidating Responses:\n";

    json valid_resp = make_result(1, "success");
    is_valid = validate_response(valid_resp, &why);
    std::cout << "Valid response: " << valid_resp.dump() << "\n";
    std::cout << "Is valid? " << (is_valid ? "YES" : "NO") << "\n";

    json invalid_resp = {{"jsonrpc", "2.0"}, {"result", "ok"}, {"error", json::object()}};
    is_valid = validate_response(invalid_resp, &why);
    std::cout << "\nInvalid response (both result and error): " << invalid_resp.dump() << "\n";
    std::cout << "Is valid? " << (is_valid ? "YES" : "NO") << " - Reason: " << why << "\n";
}

void jsonrpc_dispatcher_demo()
{
    print_section("7. Dispatcher in Action");

    dispatcher d;

    // Register methods
    d.add("add",
          [](const json &params) -> json { return params[0].get<int>() + params[1].get<int>(); });

    d.add("greet",
          [](const json &params) -> json
          {
              std::string name = params.is_array() ? params[0].get<std::string>()
                                                   : params["name"].get<std::string>();
              return "Hello, " + name + "!";
          });

    d.add("fail", [](const json &) -> json
          { throw rpc_exception(error{-32000, "Intentional failure", json{{"reason", "demo"}}}); });

    std::cout << "\nTesting Dispatcher:\n";

    // Test 1: Successful request
    std::cout << "\n1. Successful request (add):\n";
    json req1 = make_request(1, "add", json::array({5, 7}));
    std::cout << "   Request:  " << req1.dump() << "\n";
    auto resp1 = d.handle(req1);
    if (resp1)
        std::cout << "   Response: " << resp1->dump() << "\n";

    // Test 2: Named parameters
    std::cout << "\n2. Named parameters (greet):\n";
    json req2 = make_request(2, "greet", json::object({{"name", "Alice"}}));
    std::cout << "   Request:  " << req2.dump() << "\n";
    auto resp2 = d.handle(req2);
    if (resp2)
        std::cout << "   Response: " << resp2->dump() << "\n";

    // Test 3: Method not found
    std::cout << "\n3. Method not found:\n";
    json req3 = make_request(3, "unknown", json::array());
    std::cout << "   Request:  " << req3.dump() << "\n";
    auto resp3 = d.handle(req3);
    if (resp3)
        std::cout << "   Response: " << resp3->dump(2) << "\n";

    // Test 4: Intentional error
    std::cout << "\n4. Handler throwing error:\n";
    json req4 = make_request(4, "fail", json::array());
    std::cout << "   Request:  " << req4.dump() << "\n";
    auto resp4 = d.handle(req4);
    if (resp4)
        std::cout << "   Response: " << resp4->dump(2) << "\n";

    // Test 5: Notification (no response)
    std::cout << "\n5. Notification (no response):\n";
    json notif = make_notification("greet", json::object({{"name", "Bob"}}));
    std::cout << "   Notification: " << notif.dump() << "\n";
    auto resp5 = d.handle(notif);
    std::cout << "   Response: " << (resp5 ? resp5->dump() : "null (no response)") << "\n";

    // Test 6: Batch request
    std::cout << "\n6. Batch request:\n";
    json batch = json::array({make_request(10, "add", json::array({1, 2})),
                              make_request(11, "greet", json::object({{"name", "Charlie"}})),
                              make_notification("add", json::array({99, 1}))});
    std::cout << "   Batch: " << batch.dump(2) << "\n";
    auto resp6 = d.handle(batch);
    if (resp6)
        std::cout << "   Response: " << resp6->dump(2) << "\n";
}

void jsonrpc_best_practices()
{
    print_section("8. Best Practices");

    std::cout << "\n1. Always use jsonrpc: \"2.0\"\n";
    std::cout << "   This is required by the specification.\n";

    std::cout << "\n2. Use meaningful method names\n";
    std::cout << "   Good: \"user.create\", \"order.list\", \"payment.process\"\n";
    std::cout << "   Bad:  \"method1\", \"doStuff\", \"x\"\n";

    std::cout << "\n3. Choose appropriate ID types\n";
    std::cout << "   - Numbers: simple, sequential\n";
    std::cout << "   - Strings: UUIDs, semantic identifiers\n";
    std::cout << "   - null: only for error responses to invalid requests\n";

    std::cout << "\n4. Use notifications wisely\n";
    std::cout << "   - For events that don't need confirmation\n";
    std::cout << "   - For performance (no response overhead)\n";
    std::cout << "   - When you don't care about errors\n";

    std::cout << "\n5. Provide detailed error information\n";
    std::cout << "   - Use standard error codes when applicable\n";
    std::cout << "   - Include helpful 'data' field\n";
    std::cout << "   - Write clear error messages\n";

    std::cout << "\n6. Handle batch requests efficiently\n";
    std::cout << "   - Process in parallel when possible\n";
    std::cout << "   - Remember: response order doesn't matter\n";
    std::cout << "   - Don't respond to notifications in batch\n";

    std::cout << "\n7. Validate all inputs\n";
    std::cout << "   - Check jsonrpc version\n";
    std::cout << "   - Verify required fields\n";
    std::cout << "   - Validate parameter types\n";

    std::cout << "\n8. Document your API\n";
    std::cout << "   - List all methods\n";
    std::cout << "   - Describe parameters (type, required/optional)\n";
    std::cout << "   - Document possible errors\n";
    std::cout << "   - Provide examples\n";
}

// ============================================================================
// Main Entry Point
// ============================================================================

int run_jsonrpc_fundamentals()
{
    try
    {
        std::cout << "\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";
        std::cout << "  JSON-RPC 2.0 Fundamentals Tutorial\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";

        jsonrpc_request_structure();
        jsonrpc_response_structure();
        jsonrpc_error_structure();
        jsonrpc_notifications();
        jsonrpc_batch_requests();
        jsonrpc_validation_examples();
        jsonrpc_dispatcher_demo();
        jsonrpc_best_practices();

        std::cout << "\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";
        std::cout << "  JSON-RPC 2.0 Fundamentals Tutorial Completed!\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in JSON-RPC fundamentals tutorial: " << e.what() << std::endl;
        return 1;
    }
}
