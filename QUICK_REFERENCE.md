# JSON-RPC 2.0 Quick Reference

A quick reference guide for using the jsonrpc2 C++ library.

## Include

```cpp
#include "include/jsonrpc.hpp"
using namespace pooriayousefi;
using json = nlohmann::json;
```

## Creating Messages

### Request with ID

```cpp
json req = make_request(1, "method_name", json::array({param1, param2}));
json req = make_request("id-123", "method_name", json{{"key", "value"}});
```

### Notification (no ID)

```cpp
json notif = make_notification("method_name", json{{"key", "value"}});
```

### Success Response

```cpp
json resp = make_result(request_id, result_value);
```

### Error Response

```cpp
error err{-32601, "Method not found"};
json resp = make_error(request_id, err);

// With data
error err{-32000, "Custom error", json{{"detail", "info"}}};
```

## Validation

```cpp
std::string why;
if (validate_request(msg, &why)) {
    // Valid request
} else {
    std::cerr << "Invalid: " << why << "\n";
}

if (is_request(msg)) { /* ... */ }
if (is_notification(msg)) { /* ... */ }
if (is_response(msg)) { /* ... */ }
```

## Dispatcher (Server)

### Basic Usage

```cpp
dispatcher server;

// Register methods
server.add("add", [](const json& params) -> json {
    return params[0].get<int>() + params[1].get<int>();
});

server.add("greet", [](const json& params) -> json {
    return "Hello, " + params["name"].get<std::string>() + "!";
});

// Handle request
json req = /* ... */;
auto resp = server.handle(req);
if (resp) {
    std::cout << resp->dump() << "\n";
}
```

### Error Handling

```cpp
server.add("divide", [](const json& params) -> json {
    int b = params[1].get<int>();
    if (b == 0) {
        throw rpc_exception(error{-32000, "Division by zero"});
    }
    return params[0].get<int>() / b;
});
```

## Endpoint (Client/Server)

### Server Setup

```cpp
endpoint server([](const json& msg) {
    send_to_client(msg);  // Your transport
});

// Register methods
server.add("echo", [](const json& params) -> json {
    return params;
});

// Handle incoming messages
server.receive(message_from_client);
```

### Client Usage

```cpp
endpoint client([](const json& msg) {
    send_to_server(msg);  // Your transport
});

// Send request
client.send_request("method", json::array({arg1, arg2}),
    [](const json& result) {
        std::cout << "Result: " << result << "\n";
    },
    [](const json& error) {
        std::cerr << "Error: " << error << "\n";
    }
);

// Send notification
client.send_notification("log", json{{"msg", "test"}});
```

## Batch Requests

```cpp
json batch = json::array({
    make_request(1, "method1", json::array()),
    make_request(2, "method2", json::array()),
    make_notification("log", json{{"msg", "processing"}})
});

auto responses = dispatcher.handle(batch);
// Returns array with responses (notifications don't get responses)
```

## Advanced Features

### Progress Reporting

```cpp
server.add("long_task", [](const json& params) -> json {
    for (int i = 0; i < 100; ++i) {
        // Do work
        report_progress(json{{"current", i}, {"total", 100}});
    }
    return json{{"status", "completed"}};
});
```

### Cancellation

```cpp
server.add("cancellable", [](const json& params) -> json {
    for (int i = 0; i < 1000; ++i) {
        if (is_canceled()) {
            throw rpc_exception(error{-32800, "Cancelled"});
        }
        // Do work
    }
    return json{{"status", "completed"}};
});

// Client side
client.cancel(request_id);
```

### Context Access

```cpp
server.add("context_method", [](const json& params) -> json {
    const call_context* ctx = current_context();
    if (ctx) {
        json id = ctx->id;
        report_progress(json{{"status", "working"}});
        if (is_canceled()) {
            throw rpc_exception(error{-32800, "Cancelled"});
        }
    }
    return json{{"status", "ok"}};
});
```

## Standard Error Codes

| Code | Message | Use Case |
|------|---------|----------|
| -32700 | Parse error | Invalid JSON received |
| -32600 | Invalid Request | Request doesn't match spec |
| -32601 | Method not found | Unknown method |
| -32602 | Invalid params | Wrong parameter types |
| -32603 | Internal error | Server internal error |
| -32000 to -32099 | Server error | Custom application errors |

## Example: Simple Calculator

```cpp
#include "include/jsonrpc.hpp"
using namespace pooriayousefi;
using json = nlohmann::json;

int main() {
    dispatcher calc;
    
    calc.add("add", [](const json& p) -> json {
        return p[0].get<int>() + p[1].get<int>();
    });
    
    calc.add("sub", [](const json& p) -> json {
        return p[0].get<int>() - p[1].get<int>();
    });
    
    calc.add("mul", [](const json& p) -> json {
        return p[0].get<int>() * p[1].get<int>();
    });
    
    calc.add("div", [](const json& p) -> json {
        int b = p[1].get<int>();
        if (b == 0) throw rpc_exception(error{-32000, "Div by zero"});
        return p[0].get<int>() / b;
    });
    
    // Test it
    auto r1 = calc.handle(make_request(1, "add", json::array({10, 5})));
    auto r2 = calc.handle(make_request(2, "mul", json::array({6, 7})));
    
    std::cout << r1->dump() << "\n";  // {"jsonrpc":"2.0","result":15,"id":1}
    std::cout << r2->dump() << "\n";  // {"jsonrpc":"2.0","result":42,"id":2}
    
    return 0;
}
```

## Build and Run

```bash
# Compile builder
g++ -std=c++23 builder.cpp -o builder

# Build your project
./builder --release --executable

# Run
./build/release/jsonrpc2
```

## Tips

1. **Always validate** requests before processing
2. **Use try-catch** to handle JSON parsing errors
3. **Provide clear error messages** with data field
4. **Document your methods** with expected parameters
5. **Test error cases** not just happy paths
6. **Use batch requests** for multiple operations
7. **Handle cancellation** for long operations
8. **Report progress** for better UX

## More Information

- **Full Tutorial**: See [JSONRPC2.md](JSONRPC2.md)
- **JSON Basics**: See [JSON.md](JSON.md)
- **API Documentation**: See [README.md](README.md)
- **Examples**: Check `tests/` directory
- **Specification**: <https://www.jsonrpc.org/specification>

---

**Quick Links**:

- [JSON-RPC 2.0 Spec](https://www.jsonrpc.org/specification)
- [nlohmann/json docs](https://json.nlohmann.me/)
- [Project README](README.md)
