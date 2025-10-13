/*
 * JSON-RPC 2.0 Advanced Features Tutorial
 *
 * Demonstrates advanced features including:
 * - Endpoint-based client/server communication
 * - Progress reporting
 * - Request cancellation
 * - Context and state management
 * - Batch processing optimization
 */

#include "../include/jsonrpc.hpp"
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace pooriayousefi;
using json = nlohmann::json;

static void print_section(const std::string &title)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << title << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
}

void endpoint_communication()
{
    print_section("1. Endpoint-based Communication");

    std::cout << "\nSetting up client-server communication using endpoints:\n\n";

    // Message queues for communication
    std::vector<json> client_to_server_queue;
    std::vector<json> server_to_client_queue;

    // Create server endpoint
    endpoint server(
        [&server_to_client_queue](const json &msg)
        {
            std::cout << "[Server → Client] Sending: " << msg.dump() << "\n";
            server_to_client_queue.push_back(msg);
        });

    // Create client endpoint
    endpoint client(
        [&client_to_server_queue](const json &msg)
        {
            std::cout << "[Client → Server] Sending: " << msg.dump() << "\n";
            client_to_server_queue.push_back(msg);
        });

    // Server implements methods
    std::cout << "Registering server methods...\n\n";
    server.add("echo",
               [](const json &params) -> json
               {
                   std::cout << "[Server] Processing echo request\n";
                   return params;
               });

    server.add("reverse",
               [](const json &params) -> json
               {
                   std::cout << "[Server] Processing reverse request\n";
                   std::string input = params[0].get<std::string>();
                   std::reverse(input.begin(), input.end());
                   return input;
               });

    server.add("multiply",
               [](const json &params) -> json
               {
                   std::cout << "[Server] Processing multiply request\n";
                   return params[0].get<int>() * params[1].get<int>();
               });

    // Client sends requests
    std::cout << "Client sending requests...\n\n";

    bool echo_received = false;
    json echo_result;
    client.send_request(
        "echo", json::array({"Hello, Server!"}),
        [&](const json &result)
        {
            echo_received = true;
            echo_result = result;
            std::cout << "[Client] Received echo result: " << result.dump() << "\n";
        },
        [](const json &error) { std::cout << "[Client] Error: " << error.dump() << "\n"; });

    bool reverse_received = false;
    json reverse_result;
    client.send_request(
        "reverse", json::array({"JSONRPC"}),
        [&](const json &result)
        {
            reverse_received = true;
            reverse_result = result;
            std::cout << "[Client] Received reverse result: " << result.dump() << "\n";
        },
        [](const json &error) { std::cout << "[Client] Error: " << error.dump() << "\n"; });

    // Process messages
    std::cout << "\nProcessing messages...\n\n";
    for (const auto &msg : client_to_server_queue)
    {
        server.receive(msg);
    }
    client_to_server_queue.clear();

    for (const auto &msg : server_to_client_queue)
    {
        client.receive(msg);
    }
    server_to_client_queue.clear();

    std::cout << "\nResults:\n";
    std::cout << "  Echo received: " << (echo_received ? "YES" : "NO") << "\n";
    if (echo_received)
        std::cout << "  Echo result: " << echo_result.dump() << "\n";
    std::cout << "  Reverse received: " << (reverse_received ? "YES" : "NO") << "\n";
    if (reverse_received)
        std::cout << "  Reverse result: " << reverse_result.dump() << "\n";
}

void progress_reporting()
{
    print_section("2. Progress Reporting");

    std::cout << "\nDemonstrating progress reporting for long-running operations:\n\n";

    std::vector<json> messages;
    endpoint server([&messages](const json &msg) { messages.push_back(msg); });

    std::vector<json> progress_updates;

    // Register a long-running operation
    server.add("process_data",
               [](const json &params) -> json
               {
                   std::cout << "[Server] Starting long operation...\n";
                   int total = params.value("count", 10);

                   for (int i = 1; i <= total; ++i)
                   {
                       // Simulate work
                       std::cout << "[Server] Processing item " << i << " of " << total << "\n";

                       // Report progress
                       report_progress(json{
                           {"current", i}, {"total", total}, {"percentage", (i * 100) / total}});

                       // Check for cancellation
                       if (is_canceled())
                       {
                           std::cout << "[Server] Operation cancelled!\n";
                           throw rpc_exception(error{-32800, "Request cancelled"});
                       }
                   }

                   return json{{"status", "completed"}, {"processed", total}};
               });

    // Setup progress handler
    endpoint client([&messages](const json &msg) { messages.push_back(msg); });

    std::string progress_token = client.create_progress_token();
    client.on_progress(progress_token,
                       [&progress_updates](const json &value)
                       {
                           std::cout << "[Client] Progress update: " << value.dump() << "\n";
                           progress_updates.push_back(value);
                       });

    // Send request with progress token
    std::cout << "Client requesting long operation with progress tracking...\n\n";
    json req =
        make_request(1, "process_data", json{{"count", 5}, {"progressToken", progress_token}});

    // Simulate server receiving and processing
    server.receive(req);

    // Process progress notifications
    for (const auto &msg : messages)
    {
        if (msg.contains("method") && msg["method"] == "$/progress")
        {
            client.receive(msg);
        }
    }

    std::cout << "\nSummary:\n";
    std::cout << "  Total progress updates: " << progress_updates.size() << "\n";
}

void request_cancellation()
{
    print_section("3. Request Cancellation");

    std::cout << "\nDemonstrating request cancellation:\n\n";

    std::vector<json> messages;
    endpoint server(
        [&messages](const json &msg)
        {
            std::cout << "[Server] Outgoing: " << msg.dump() << "\n";
            messages.push_back(msg);
        });

    // Register a cancellable operation
    server.add("long_task",
               [](const json &params) -> json
               {
                   std::cout << "[Server] Starting cancellable task...\n";
                   int iterations = params.value("iterations", 100);

                   for (int i = 0; i < iterations; ++i)
                   {
                       if (is_canceled())
                       {
                           std::cout << "[Server] Task cancelled at iteration " << i << "\n";
                           throw rpc_exception(error{-32800, "Request cancelled",
                                                     json{{"completed_iterations", i}}});
                       }

                       std::cout << "[Server] Iteration " << i << "\n";
                       report_progress(json{{"iteration", i}, {"total", iterations}});
                   }

                   return json{{"status", "completed"}, {"iterations", iterations}};
               });

    endpoint client(
        [&messages](const json &msg)
        {
            std::cout << "[Client] Outgoing: " << msg.dump() << "\n";
            messages.push_back(msg);
        });

    // Send request
    std::cout << "Client sending cancellable request...\n";
    std::string req_id = "cancel-demo-1";
    bool result_received = false;
    json final_result;

    client.send_request_with_id(
        req_id, "long_task", json{{"iterations", 10}},
        [&](const json &result)
        {
            result_received = true;
            final_result = result;
            std::cout << "[Client] Received result: " << result.dump() << "\n";
        },
        [&](const json &error)
        {
            result_received = true;
            final_result = error;
            std::cout << "[Client] Received error: " << error.dump() << "\n";
        });

    // Simulate: send cancel after a delay
    std::cout << "\nClient sending cancel request...\n";
    client.cancel(json(req_id));

    std::cout << "\nTotal messages exchanged: " << messages.size() << "\n";
}

void batch_processing()
{
    print_section("4. Batch Processing Optimization");

    std::cout << "\nDemonstrating efficient batch request processing:\n\n";

    dispatcher d;

    // Register methods
    d.add("add",
          [](const json &params) -> json { return params[0].get<int>() + params[1].get<int>(); });

    d.add("subtract",
          [](const json &params) -> json { return params[0].get<int>() - params[1].get<int>(); });

    d.add("multiply",
          [](const json &params) -> json { return params[0].get<int>() * params[1].get<int>(); });

    d.add("divide",
          [](const json &params) -> json
          {
              int b = params[1].get<int>();
              if (b == 0)
                  throw rpc_exception(error{-32000, "Division by zero"});
              return params[0].get<int>() / b;
          });

    // Create a large batch
    std::cout << "Creating batch of 10 requests...\n";
    json batch = json::array();
    batch.push_back(make_request(1, "add", json::array({10, 5})));
    batch.push_back(make_request(2, "subtract", json::array({20, 8})));
    batch.push_back(make_request(3, "multiply", json::array({7, 6})));
    batch.push_back(make_request(4, "divide", json::array({100, 4})));
    batch.push_back(make_request(5, "add", json::array({1, 1})));
    batch.push_back(
        make_notification("log", json{{"msg", "Processing batch"}})); // Notification - no response
    batch.push_back(make_request(6, "multiply", json::array({3, 3})));
    batch.push_back(make_request(7, "divide", json::array({50, 2})));
    batch.push_back(make_request(8, "unknown_method", json::array())); // Error case
    batch.push_back(make_request(9, "divide", json::array({10, 0})));  // Error case

    std::cout << "\nBatch request:\n" << batch.dump(2) << "\n";

    // Process batch
    std::cout << "\nProcessing batch...\n";
    auto batch_response = d.handle(batch);

    if (batch_response)
    {
        std::cout << "\nBatch response:\n" << batch_response->dump(2) << "\n";
        std::cout << "\nAnalysis:\n";
        std::cout << "  Requests in batch: " << batch.size() << "\n";
        std::cout << "  Responses received: " << batch_response->size() << "\n";
        std::cout << "  (Note: Notifications don't get responses)\n\n";

        int successes = 0;
        int errors = 0;
        for (const auto &resp : *batch_response)
        {
            if (resp.contains("result"))
                successes++;
            else if (resp.contains("error"))
                errors++;
        }
        std::cout << "  Successful: " << successes << "\n";
        std::cout << "  Errors: " << errors << "\n";
    }
}

void context_management()
{
    print_section("5. Context and State Management");

    std::cout << "\nDemonstrating context-aware request handling:\n\n";

    std::vector<json> messages;
    endpoint server([&messages](const json &msg) { messages.push_back(msg); });

    // Add a method that uses context
    server.add("context_aware_task",
               [](const json &params) -> json
               {
                   std::cout << "[Server] Executing context-aware task\n";

                   const call_context *ctx = current_context();
                   if (ctx)
                   {
                       std::cout << "[Server] Context available:\n";
                       std::cout << "  Request ID: " << ctx->id.dump() << "\n";

                       // Report progress using context
                       for (int i = 1; i <= 5; ++i)
                       {
                           report_progress(json{{"step", i}, {"total", 5}});
                           std::cout << "[Server] Step " << i << " of 5\n";

                           if (is_canceled())
                           {
                               std::cout << "[Server] Detected cancellation!\n";
                               throw rpc_exception(error{-32800, "Cancelled"});
                           }
                       }
                   }

                   return json{{"status", "completed"}};
               });

    // Send request
    json req = make_request(1, "context_aware_task", json::object());
    std::cout << "Processing request with context...\n";
    server.receive(req);

    std::cout << "\nMessages generated: " << messages.size() << "\n";
}

void error_recovery_patterns()
{
    print_section("6. Error Recovery Patterns");

    std::cout << "\nDemonstrating error handling and recovery:\n\n";

    dispatcher d;

    // Retry-able operation
    int attempt_count = 0;
    d.add("unstable_operation",
          [&attempt_count](const json &params) -> json
          {
              attempt_count++;
              int max_attempts = params.value("max_attempts", 3);

              std::cout << "[Server] Attempt #" << attempt_count << "\n";

              if (attempt_count < max_attempts)
              {
                  throw rpc_exception(error{-32001, "Temporary failure",
                                            json{{"attempt", attempt_count}, {"retry", true}}});
              }

              return json{{"status", "success"}, {"attempts", attempt_count}};
          });

    // Simulate retries
    std::cout << "Testing retry pattern:\n";
    for (int i = 1; i <= 3; ++i)
    {
        std::cout << "\nRetry #" << i << ":\n";
        json req = make_request(i, "unstable_operation", json{{"max_attempts", 3}});
        auto resp = d.handle(req);
        if (resp)
        {
            std::cout << "Response: " << resp->dump(2) << "\n";
            if (resp->contains("result"))
            {
                std::cout << "Success!\n";
                break;
            }
            else if (resp->contains("error"))
            {
                json err = (*resp)["error"];
                if (err.contains("data") && err["data"].value("retry", false))
                {
                    std::cout << "Retryable error, will try again...\n";
                }
            }
        }
    }
}

void initialization_protocol()
{
    print_section("7. Initialization Protocol");

    std::cout << "\nDemonstrating initialization handshake:\n\n";

    std::vector<json> messages;
    endpoint server(
        [&messages](const json &msg)
        {
            std::cout << "[Server] Sending: " << msg.dump(2) << "\n";
            messages.push_back(msg);
        });

    endpoint client(
        [&messages](const json &msg)
        {
            std::cout << "[Client] Sending: " << msg.dump(2) << "\n";
            messages.push_back(msg);
        });

    // Server sets capabilities
    server.set_server_capabilities(json{{"textDocumentSync", 1},
                                        {"completionProvider", {{"resolveProvider", true}}},
                                        {"hoverProvider", true}});

    std::cout << "Client initializing...\n\n";
    json init_params = {{"processId", 12345},
                        {"clientInfo", {{"name", "example-client"}, {"version", "1.0.0"}}},
                        {"capabilities", json::object()}};

    bool initialized = false;
    json server_capabilities;

    client.initialize(
        init_params,
        [&](const json &result)
        {
            initialized = true;
            server_capabilities = result["capabilities"];
            std::cout << "\n[Client] Initialization successful!\n";
            std::cout << "[Client] Server capabilities:\n" << server_capabilities.dump(2) << "\n";
        },
        [](const json &error)
        { std::cout << "\n[Client] Initialization failed: " << error.dump() << "\n"; });

    // Process messages
    for (const auto &msg : messages)
    {
        if (msg.contains("method") && msg["method"] == "initialize")
        {
            server.receive(msg);
        }
        else if (msg.contains("result"))
        {
            client.receive(msg);
        }
    }

    std::cout << "\nInitialization complete: " << (initialized ? "YES" : "NO") << "\n";
    std::cout << "Server initialized: " << (server.is_initialized() ? "YES" : "NO") << "\n";
}

// ============================================================================
// Main Entry Point
// ============================================================================

int run_advanced_features()
{
    try
    {
        std::cout << "\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";
        std::cout << "  JSON-RPC 2.0 Advanced Features Tutorial\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";

        endpoint_communication();
        progress_reporting();
        request_cancellation();
        batch_processing();
        context_management();
        error_recovery_patterns();
        initialization_protocol();

        std::cout << "\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";
        std::cout << "  Advanced Features Tutorial Completed!\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in advanced features tutorial: " << e.what() << std::endl;
        return 1;
    }
}
