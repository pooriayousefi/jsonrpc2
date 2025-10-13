# Building a Modern C++23 JSON-RPC 2.0 Library: A Complete Journey

## From specification to production-ready code with comprehensive tests and tutorials

---

**TL;DR**: I built a complete, header-only C++23 library implementing the JSON-RPC 2.0 specification, with 31 unit tests, real-world examples, and comprehensive tutorials. This article walks through the design decisions, implementation challenges, and lessons learned. [GitHub Repository](https://github.com/pooriayousefi/jsonrpc2)

---

## The Problem: Why Another RPC Library?

Remote Procedure Call (RPC) protocols are the backbone of modern distributed systems. Whether you're building microservices, implementing Language Server Protocol (LSP) for IDE extensions, or creating blockchain nodes, you need a reliable way for programs to communicate.

**JSON-RPC 2.0** is one of the most elegant RPC protocols out there:
- Simple specification (one page!)
- Transport-agnostic (HTTP, WebSocket, TCP, IPC)
- Language-neutral (JSON-based)
- Widely adopted (Ethereum, Bitcoin, VSCode, Vim)

But here's the catch: most C++ JSON-RPC libraries are either:
- ❌ Outdated (using C++11 or older)
- ❌ Tightly coupled to specific transports (HTTP-only)
- ❌ Lacking proper error handling
- ❌ Missing advanced features (batching, progress reporting)

**I wanted something different**: a modern, header-only library that you can drop into any project and start using in minutes.

---

## What is JSON-RPC 2.0?

Before diving into implementation, let's understand what we're building.

JSON-RPC 2.0 defines three message types:

### 1. Request (expects a response)

**[CARBON SNIPPET 1: Request Example]**
```json
{
  "jsonrpc": "2.0",
  "method": "subtract",
  "params": [42, 23],
  "id": 1
}
```

### 2. Response (success or error)

**[CARBON SNIPPET 2: Success Response]**
```json
{
  "jsonrpc": "2.0",
  "result": 19,
  "id": 1
}
```

**[CARBON SNIPPET 3: Error Response]**
```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -32601,
    "message": "Method not found"
  },
  "id": 1
}
```

### 3. Notification (no response expected)

**[CARBON SNIPPET 4: Notification Example]**
```json
{
  "jsonrpc": "2.0",
  "method": "update",
  "params": ["some_data"]
}
```

That's it! Simple, elegant, powerful.

---

## Design Decisions

### Why Header-Only?

Header-only libraries are a gift to C++ developers:
- ✅ No linking headaches
- ✅ No ABI compatibility issues
- ✅ Easy to integrate (just `#include`)
- ✅ Compiler can optimize across boundaries

The trade-off? Longer compile times. But for a library this size (~550 lines), it's negligible.

### Why C++23?

Modern C++ isn't just about new syntax—it's about expressiveness and safety:

**[CARBON SNIPPET 5: Modern C++ Features]**
```cpp
// Structured bindings
auto [success, value] = parse_json(data);

// Lambda improvements
auto handler = [this](const json& params) -> json {
    return process(params);
};

// Optional with monadic operations
return validate(req)
    .and_then([](auto r) { return process(r); })
    .or_else([](auto e) { return handle_error(e); });
```

### Architecture Overview

The library has three main components:

1. **Message Builders** — Create valid JSON-RPC messages
2. **Dispatcher** — Route requests to handlers (server-side)
3. **Endpoint** — Bidirectional client/server with callbacks

**[CARBON SNIPPET 6: Architecture]**
```cpp
// Message builders
json request = make_request(1, "add", json::array({5, 3}));
json response = make_result(1, 8);
json error = make_error(1, {-32601, "Not found"});

// Dispatcher (server)
dispatcher server;
server.add("add", [](const json& p) { return p[0].get<int>() + p[1].get<int>(); });
auto response = server.handle(request);

// Endpoint (client/server)
endpoint client([](const json& msg) { send_over_network(msg); });
client.send_request("add", json::array({5, 3}), 
    [](const json& result) { std::cout << "Result: " << result << "\n"; });
```

---

## Implementation Highlights

### 1. Request Validation

The JSON-RPC spec is strict. A valid request MUST have:
- `jsonrpc: "2.0"`
- `method` (string)
- `id` (string, number, or null)
- `params` (optional array or object)

**[CARBON SNIPPET 7: Validation]**
```cpp
bool validate_request(const json& req, std::string* why = nullptr) {
    // Check version
    if (!req.contains("jsonrpc") || req["jsonrpc"] != "2.0") {
        if (why) *why = "Missing or invalid jsonrpc version";
        return false;
    }
    
    // Check method
    if (!req.contains("method") || !req["method"].is_string()) {
        if (why) *why = "Missing or invalid method";
        return false;
    }
    
    // Check ID (if present)
    if (req.contains("id")) {
        auto& id = req["id"];
        if (!id.is_string() && !id.is_number() && !id.is_null()) {
            if (why) *why = "Invalid id type";
            return false;
        }
    }
    
    return true;
}
```

### 2. The Dispatcher Pattern

The dispatcher is the heart of the server-side implementation. It maps method names to handler functions:

**[CARBON SNIPPET 8: Dispatcher Implementation]**
```cpp
class dispatcher {
    std::unordered_map<std::string, handler_func> handlers_;
    
public:
    void add(const std::string& method, handler_func handler) {
        handlers_[method] = std::move(handler);
    }
    
    std::optional<json> handle(const json& request) {
        // Validate request
        std::string why;
        if (!validate_request(request, &why)) {
            return make_error(request["id"], 
                error{-32600, "Invalid Request", why});
        }
        
        // Find handler
        auto method = request["method"].get<std::string>();
        auto it = handlers_.find(method);
        if (it == handlers_.end()) {
            return make_error(request["id"], 
                error{-32601, "Method not found"});
        }
        
        // Execute handler
        try {
            auto result = it->second(request["params"]);
            return make_result(request["id"], result);
        } catch (const rpc_exception& e) {
            return make_error(request["id"], e.err);
        } catch (const std::exception& e) {
            return make_error(request["id"], 
                error{-32603, "Internal error", e.what()});
        }
    }
};
```

### 3. Error Handling

JSON-RPC defines standard error codes. We use custom exceptions to propagate them:

**[CARBON SNIPPET 9: Error Handling]**
```cpp
struct error {
    int code;
    std::string message;
    std::optional<json> data;
};

struct rpc_exception : std::exception {
    error err;
    explicit rpc_exception(error e) : err(std::move(e)) {}
};

// Usage in handlers
server.add("divide", [](const json& params) -> json {
    int a = params[0].get<int>();
    int b = params[1].get<int>();
    
    if (b == 0) {
        throw rpc_exception(error{
            -32000,  // Server error code
            "Division by zero",
            json{{"numerator", a}}
        });
    }
    
    return a / b;
});
```

### 4. Batch Requests

One of JSON-RPC's powerful features is batch processing. Send multiple requests in one message:

**[CARBON SNIPPET 10: Batch Processing]**
```cpp
json batch = json::array({
    make_request(1, "add", json::array({5, 3})),
    make_request(2, "subtract", json::array({10, 4})),
    make_notification("log", json{{"message", "Processing batch"}})
});

// Handle returns array of responses (no response for notifications)
auto responses = server.handle(batch);
// Result: [{"jsonrpc":"2.0","result":8,"id":1}, 
//          {"jsonrpc":"2.0","result":6,"id":2}]
```

---

## Real-World Examples

Theory is great, but let's see real code.

### Example 1: Calculator Service

A simple RPC calculator that handles math operations:

**[CARBON SNIPPET 11: Calculator Service]**
```cpp
class CalculatorService {
    dispatcher calc_;
    double memory_ = 0.0;
    
public:
    CalculatorService() {
        // Basic operations
        calc_.add("add", [](const json& p) -> json {
            return p[0].get<double>() + p[1].get<double>();
        });
        
        calc_.add("subtract", [](const json& p) -> json {
            return p[0].get<double>() - p[1].get<double>();
        });
        
        calc_.add("multiply", [](const json& p) -> json {
            return p[0].get<double>() * p[1].get<double>();
        });
        
        calc_.add("divide", [](const json& p) -> json {
            double b = p[1].get<double>();
            if (b == 0) {
                throw rpc_exception(error{-32000, "Division by zero"});
            }
            return p[0].get<double>() / b;
        });
        
        // Advanced operations
        calc_.add("power", [](const json& p) -> json {
            return std::pow(p[0].get<double>(), p[1].get<double>());
        });
        
        calc_.add("sqrt", [](const json& p) -> json {
            double val = p[0].get<double>();
            if (val < 0) {
                throw rpc_exception(error{-32000, "Cannot sqrt negative"});
            }
            return std::sqrt(val);
        });
        
        // Memory operations
        calc_.add("memory_store", [this](const json& p) -> json {
            memory_ = p[0].get<double>();
            return json{{"stored", memory_}};
        });
        
        calc_.add("memory_recall", [this](const json& p) -> json {
            return memory_;
        });
    }
    
    std::optional<json> handle(const json& request) {
        return calc_.handle(request);
    }
};
```

**Usage:**

**[CARBON SNIPPET 12: Calculator Usage]**
```cpp
CalculatorService calc;

// Simple math
auto r1 = calc.handle(make_request(1, "add", json::array({10, 5})));
// Response: {"jsonrpc":"2.0","result":15,"id":1}

// Power operation
auto r2 = calc.handle(make_request(2, "power", json::array({2, 8})));
// Response: {"jsonrpc":"2.0","result":256,"id":2}

// Error handling
auto r3 = calc.handle(make_request(3, "divide", json::array({10, 0})));
// Response: {"jsonrpc":"2.0","error":{"code":-32000,"message":"Division by zero"},"id":3}
```

### Example 2: Database CRUD Service

A more complex example showing CRUD operations:

**[CARBON SNIPPET 13: Database Service]**
```cpp
class DatabaseService {
    struct User {
        int id;
        std::string name;
        std::string email;
        int age;
    };
    
    std::unordered_map<int, User> users_;
    int next_id_ = 1;
    dispatcher db_;
    
public:
    DatabaseService() {
        // Create
        db_.add("user.create", [this](const json& p) -> json {
            User user{
                next_id_++,
                p["name"].get<std::string>(),
                p["email"].get<std::string>(),
                p["age"].get<int>()
            };
            users_[user.id] = user;
            return user_to_json(user);
        });
        
        // Read
        db_.add("user.get", [this](const json& p) -> json {
            int id = p["id"].get<int>();
            auto it = users_.find(id);
            if (it == users_.end()) {
                throw rpc_exception(error{-32001, "User not found"});
            }
            return user_to_json(it->second);
        });
        
        // Update
        db_.add("user.update", [this](const json& p) -> json {
            int id = p["id"].get<int>();
            auto it = users_.find(id);
            if (it == users_.end()) {
                throw rpc_exception(error{-32001, "User not found"});
            }
            
            if (p.contains("name")) it->second.name = p["name"];
            if (p.contains("email")) it->second.email = p["email"];
            if (p.contains("age")) it->second.age = p["age"];
            
            return user_to_json(it->second);
        });
        
        // Delete
        db_.add("user.delete", [this](const json& p) -> json {
            int id = p["id"].get<int>();
            if (users_.erase(id) == 0) {
                throw rpc_exception(error{-32001, "User not found"});
            }
            return json{{"deleted", true}};
        });
        
        // List all
        db_.add("user.list", [this](const json& p) -> json {
            json users = json::array();
            for (const auto& [id, user] : users_) {
                users.push_back(user_to_json(user));
            }
            return users;
        });
    }
    
private:
    json user_to_json(const User& u) {
        return json{
            {"id", u.id},
            {"name", u.name},
            {"email", u.email},
            {"age", u.age}
        };
    }
};
```

---

## Advanced Features

### 1. Progress Reporting

For long-running operations, you can report progress:

**[CARBON SNIPPET 14: Progress Reporting]**
```cpp
server.add("process_large_file", [](const json& params) -> json {
    std::string file = params["file"].get<std::string>();
    int total_lines = count_lines(file);
    
    for (int i = 0; i < total_lines; ++i) {
        process_line(i);
        
        // Report progress every 100 lines
        if (i % 100 == 0) {
            report_progress(json{
                {"current", i},
                {"total", total_lines},
                {"percentage", (i * 100.0) / total_lines}
            });
        }
    }
    
    return json{{"status", "completed"}, {"lines", total_lines}};
});
```

### 2. Request Cancellation

Cancel long-running operations:

**[CARBON SNIPPET 15: Cancellation]**
```cpp
server.add("cancellable_task", [](const json& params) -> json {
    for (int i = 0; i < 10000; ++i) {
        // Check for cancellation
        if (is_canceled()) {
            throw rpc_exception(error{
                -32800,
                "Request cancelled by client"
            });
        }
        
        do_work(i);
    }
    
    return json{{"status", "completed"}};
});

// Client side
int request_id = client.send_request("cancellable_task", json::array());
// ... later ...
client.cancel(request_id);
```

### 3. Bidirectional Communication

The `endpoint` class supports full client/server capabilities:

**[CARBON SNIPPET 16: Bidirectional Communication]**
```cpp
// Server endpoint
endpoint server([](const json& msg) {
    send_to_client_over_network(msg);
});

server.add("ping", [](const json& p) -> json {
    return json{{"pong", true}};
});

// Server can also send requests to client!
server.send_request("get_user_input", json::array(),
    [](const json& result) {
        std::cout << "User said: " << result << "\n";
    }
);

// Client endpoint
endpoint client([](const json& msg) {
    send_to_server_over_network(msg);
});

// Client handles requests from server
client.add("get_user_input", [](const json& p) -> json {
    return json{{"input", read_user_input()}};
});

// Client sends requests to server
client.send_request("ping", json::array(),
    [](const json& result) {
        std::cout << "Received: " << result << "\n";
    }
);
```

---

## Testing: The Foundation of Quality

### Unit Tests

The library includes 31 comprehensive unit tests covering:

**[CARBON SNIPPET 17: Test Example]**
```cpp
TEST(validation_valid_request) {
    json req = make_request(1, "test", json::array());
    std::string why;
    ASSERT(validate_request(req, &why));
    ASSERT(why.empty());
}

TEST(validation_invalid_version) {
    json req = {
        {"jsonrpc", "1.0"},
        {"method", "test"},
        {"id", 1}
    };
    std::string why;
    ASSERT(!validate_request(req, &why));
    ASSERT(!why.empty());
}

TEST(dispatcher_method_not_found) {
    dispatcher d;
    json req = make_request(1, "unknown", json::array());
    auto resp = d.handle(req);
    
    ASSERT(resp.has_value());
    ASSERT(resp->contains("error"));
    ASSERT((*resp)["error"]["code"] == -32601);
}

TEST(endpoint_basic_communication) {
    std::queue<json> server_queue, client_queue;
    
    endpoint server([&](const json& msg) { client_queue.push(msg); });
    endpoint client([&](const json& msg) { server_queue.push(msg); });
    
    server.add("echo", [](const json& p) -> json { return p; });
    
    bool received = false;
    client.send_request("echo", json::array({"hello"}),
        [&](const json& result) {
            ASSERT(result.is_array());
            ASSERT(result[0] == "hello");
            received = true;
        }
    );
    
    // Process messages
    while (!server_queue.empty()) {
        server.receive(server_queue.front());
        server_queue.pop();
    }
    while (!client_queue.empty()) {
        client.receive(client_queue.front());
        client_queue.pop();
    }
    
    ASSERT(received);
}
```

**Test Results**: 31/31 passed ✅

---

## The Build System

Instead of using CMake or Make, I built a custom C++ build system:

**[CARBON SNIPPET 18: Build System]**
```cpp
class BuildSystem {
    std::string mode_;        // debug or release
    std::string output_type_; // executable, static, dynamic
    
public:
    void build() {
        std::vector<std::string> sources = collect_sources();
        std::string compiler = "g++";
        std::vector<std::string> flags = {"-std=c++23"};
        
        if (mode_ == "debug") {
            flags.push_back("-g");
            flags.push_back("-O0");
        } else {
            flags.push_back("-O3");
        }
        
        // Compile
        std::string cmd = compiler;
        for (const auto& flag : flags) cmd += " " + flag;
        for (const auto& src : sources) cmd += " " + src;
        cmd += " -o build/" + mode_ + "/jsonrpc2";
        
        std::system(cmd.c_str());
    }
};
```

**Usage:**
```bash
# Compile the builder
g++ -std=c++23 builder.cpp -o builder

# Build debug executable
./builder --debug --executable

# Build release executable
./builder --release --executable

# Build static library
./builder --release --static
```

---

## Lessons Learned

### 1. The Spec is Your Friend

The JSON-RPC 2.0 specification is only about 5 pages. I read it at least 20 times. Every time I was unsure about something, I went back to the spec. This prevented many bugs.

### 2. Error Handling is Hard

Getting error handling right took the most time. Questions like:
- What if the request is invalid JSON?
- What if the method exists but params are wrong?
- What if the handler throws an unknown exception?

Each scenario needs careful handling to maintain spec compliance.

### 3. Testing is Essential

Writing tests forced me to think about edge cases:
- Batch requests with all notifications (no responses)
- Invalid JSON in requests
- Circular callback scenarios
- Memory management in long-running handlers

The 100% pass rate gave me confidence to share this library.

### 4. Modern C++ is a Joy

C++23 features made the code cleaner:
- `std::optional` for "maybe" responses
- Lambda captures for handlers
- `auto` for type deduction
- Structured bindings for clean unpacking

### 5. Documentation is Part of the Product

I spent as much time on documentation as on code:
- 730 lines: JSON tutorial
- 989 lines: JSON-RPC tutorial
- 514 lines: API documentation
- 480 lines: Project summary

Users shouldn't have to read the source to understand how to use it.

---

## Performance Considerations

While I didn't focus on micro-optimizations, some design choices help performance:

1. **Header-only**: Enables inlining and cross-TU optimization
2. **Move semantics**: All handlers use `std::move` where possible
3. **String views**: Could use `std::string_view` for method names (future)
4. **nlohmann/json**: Fast, widely-tested JSON library
5. **Exception handling**: Only for error cases, not control flow

For high-performance scenarios, consider:
- Custom allocators for JSON objects
- Object pooling for repeated operations
- Zero-copy for large payloads (pass by reference)

---

## Use Cases

Where can you use this library?

### 1. Microservices Communication
```cpp
// Service A
endpoint service_a([](const json& msg) {
    send_via_http(msg, "http://service-b:8080/rpc");
});

service_a.send_request("user.get", json{{"id", 123}},
    [](const json& user) {
        process_user(user);
    }
);
```

### 2. IDE Language Server Protocol
```cpp
// LSP server
endpoint lsp_server([](const json& msg) {
    write_to_stdout(msg);
});

lsp_server.add("textDocument/completion", [](const json& p) -> json {
    auto completions = get_completions(
        p["textDocument"]["uri"],
        p["position"]
    );
    return completions;
});
```

### 3. Blockchain Node Communication
```cpp
// Ethereum-style RPC
dispatcher eth_node;

eth_node.add("eth_blockNumber", [](const json& p) -> json {
    return get_latest_block_number();
});

eth_node.add("eth_getBalance", [](const json& p) -> json {
    return get_balance(p[0].get<std::string>());
});
```

### 4. Plugin Systems
```cpp
// Host application
endpoint host([](const json& msg) {
    send_to_plugin_process(msg);
});

// Plugins register capabilities
host.add("plugin.register", [](const json& p) -> json {
    register_plugin(p["name"], p["capabilities"]);
    return json{{"registered", true}};
});
```

---

## Future Enhancements

Some ideas for future development:

1. **Async/Await Support**: C++20 coroutines for cleaner async code
2. **Schema Validation**: JSON Schema for parameter validation
3. **Transport Adapters**: Built-in HTTP, WebSocket, TCP adapters
4. **Metrics**: Built-in performance monitoring
5. **Middleware**: Hooks for logging, auth, rate limiting
6. **Compression**: Optional gzip/zstd for large payloads
7. **Type-safe Parameters**: Template magic for compile-time checking

---

## Getting Started

Ready to use it in your project?

### Installation

**[CARBON SNIPPET 19: Installation]**
```bash
# Clone the repository
git clone https://github.com/pooriayousefi/jsonrpc2.git

# Copy headers to your project
cp jsonrpc2/include/jsonrpc.hpp your_project/include/
cp jsonrpc2/include/json.hpp your_project/include/

# Include in your code
#include "jsonrpc.hpp"
```

### Quick Example

**[CARBON SNIPPET 20: Quick Start]**
```cpp
#include "jsonrpc.hpp"
using namespace pooriayousefi;
using json = nlohmann::json;

int main() {
    // Create a simple server
    dispatcher server;
    
    server.add("greet", [](const json& params) -> json {
        std::string name = params["name"].get<std::string>();
        return "Hello, " + name + "!";
    });
    
    // Handle a request
    json request = make_request(1, "greet", json{{"name", "World"}});
    auto response = server.handle(request);
    
    std::cout << response->dump(2) << "\n";
    // Output:
    // {
    //   "jsonrpc": "2.0",
    //   "result": "Hello, World!",
    //   "id": 1
    // }
    
    return 0;
}
```

### Compilation

**[CARBON SNIPPET 21: Compilation]**
```bash
# Using g++
g++ -std=c++23 main.cpp -o my_app

# Using clang++
clang++ -std=c++23 main.cpp -o my_app

# With the included builder
./builder --release --executable
```

---

## Project Statistics

**By the Numbers:**

- 📊 **3,872 lines** of C++ code
- 📚 **2,489 lines** of documentation
- ✅ **31 unit tests** (100% pass rate)
- 📝 **6 comprehensive examples**
- ⭐ **4 tutorial documents**
- 🔧 **3 main components** (builders, dispatcher, endpoint)
- 🚀 **1 header file** to include

---

## Conclusion

Building this library was an incredible learning experience. From studying the JSON-RPC specification to implementing every edge case, from writing comprehensive tests to creating tutorials, every step taught me something new about C++, API design, and software engineering.

**What I'm most proud of:**
- ✨ Clean, readable code that follows the spec
- 🧪 Comprehensive testing (100% pass rate)
- 📖 Extensive documentation (tutorials included!)
- 🎯 Real-world examples that actually work
- 🚀 Production-ready quality

**The library is:**
- ✅ Free and open-source
- ✅ Header-only (easy integration)
- ✅ Well-tested (31 unit tests)
- ✅ Well-documented (2,489 lines of docs)
- ✅ Modern C++23
- ✅ Ready for production use

---

## Resources

- **GitHub Repository**: https://github.com/pooriayousefi/jsonrpc2
- **JSON-RPC 2.0 Specification**: https://www.jsonrpc.org/specification
- **nlohmann/json Library**: https://github.com/nlohmann/json
- **Full Documentation**: See README.md in the repository
- **Tutorials**: JSON.md and JSONRPC2.md in the repository
- **Quick Reference**: QUICK_REFERENCE.md for common tasks

---

## Get Involved

Found a bug? Have a feature request? Want to contribute?

- 🐛 **Report issues**: GitHub Issues
- 💡 **Suggest features**: GitHub Discussions
- 🤝 **Contribute**: Pull requests welcome!
- ⭐ **Star the repo**: If you find it useful
- 📢 **Share**: Help others discover it

---

## Final Thoughts

In a world of complex frameworks and heavy dependencies, sometimes what you need is a simple, well-crafted tool that does one thing really well. That's what I aimed to create with this library.

Whether you're building microservices, implementing LSP for your editor, creating a blockchain node, or just need reliable RPC communication, I hope this library serves you well.

**Happy coding!** 🚀

---

**About the Author**: I'm a C++ developer passionate about clean code, modern practices, and building tools that make developers' lives easier. You can find more of my projects on [GitHub](https://github.com/pooriayousefi).

**Tags**: #cpp #cplusplus #cpp23 #jsonrpc #rpc #programming #softwaredevelopment #opensource #moderncpp #systemsprogramming

---

*If you found this article helpful, please give it a clap 👏 and share it with your fellow developers!*
