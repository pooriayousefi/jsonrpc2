# JSON-RPC 2.0 C++ Library

A complete, header-only C++ implementation of the [JSON-RPC 2.0 specification](https://www.jsonrpc.org/specification).

## Features

- ✅ **Header-only library** - Just include and use
- ✅ **Full JSON-RPC 2.0 compliance** - Supports all specification features
- ✅ **Modern C++23** - Uses latest C++ standards
- ✅ **Comprehensive test suite** - 31 unit tests + integration tests
- ✅ **Real-world examples** - Calculator, CRUD database, and more
- ✅ **Complete tutorials** - Learn JSON and JSON-RPC from scratch
- ✅ **Advanced features** - Progress reporting, cancellation, batch processing
- ✅ **Zero external dependencies** - Only uses nlohmann/json (included)

## Table of Contents

- [Quick Start](#quick-start)
- [Building](#building)
- [Usage](#usage)
- [Examples](#examples)
- [Tutorials](#tutorials)
- [API Documentation](#api-documentation)
- [Testing](#testing)
- [Project Structure](#project-structure)
- [License](#license)

## Quick Start

### 1. Include the Library

```cpp
#include "include/jsonrpc.hpp"
using namespace pooriayousefi;
using json = nlohmann::json;
```

### 2. Create a Simple Server

```cpp
dispatcher server;

// Register a method
server.add("add", [](const json& params) -> json {
    return params[0].get<int>() + params[1].get<int>();
});

// Handle a request
json request = make_request(1, "add", json::array({5, 3}));
auto response = server.handle(request);

std::cout << response->dump() << "\n";
// Output: {"jsonrpc":"2.0","result":8,"id":1}
```

### 3. Create a Client

```cpp
endpoint client([](const json& msg) {
    // Send message to server
    send_to_server(msg);
});

// Send a request
client.send_request("add", json::array({5, 3}),
    [](const json& result) {
        std::cout << "Result: " << result << "\n";
    },
    [](const json& error) {
        std::cerr << "Error: " << error << "\n";
    }
);
```

## Building

### Prerequisites

- C++23 compatible compiler (GCC 13+, Clang 16+, or MSVC 2022+)
- No external dependencies required

### Build System

The project includes a custom C++ build system:

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

### Manual Build

```bash
g++ -std=c++23 -Wall -Wextra -Wpedantic -Iinclude \
    src/main.cpp \
    tests/*.cpp \
    -o jsonrpc2
```

### Run All Tests and Tutorials

```bash
./build/release/jsonrpc2
```

## Usage

### Basic Request/Response

```cpp
#include "include/jsonrpc.hpp"

using namespace pooriayousefi;
using json = nlohmann::json;

int main() {
    dispatcher d;
    
    // Register methods
    d.add("subtract", [](const json& params) -> json {
        return params[0].get<int>() - params[1].get<int>();
    });
    
    // Create and handle request
    json req = make_request(1, "subtract", json::array({42, 23}));
    auto resp = d.handle(req);
    
    std::cout << resp->dump(2) << "\n";
    return 0;
}
```

### Notifications (Fire-and-Forget)

```cpp
// Create notification (no ID = no response expected)
json notif = make_notification("log", json{
    {"level", "info"},
    {"message", "Server started"}
});

dispatcher d;
d.add("log", [](const json& params) -> json {
    std::cout << "Log: " << params["message"] << "\n";
    return nullptr; // Won't be sent anyway
});

d.handle(notif); // No response returned
```

### Error Handling

```cpp
d.add("divide", [](const json& params) -> json {
    int a = params[0].get<int>();
    int b = params[1].get<int>();
    
    if (b == 0) {
        throw rpc_exception(error{
            -32000, 
            "Division by zero",
            json{{"dividend", a}}
        });
    }
    
    return a / b;
});
```

### Batch Requests

```cpp
json batch = json::array({
    make_request(1, "add", json::array({1, 2})),
    make_request(2, "multiply", json::array({3, 4})),
    make_notification("log", json{{"msg", "Processing"}})
});

auto responses = d.handle(batch);
// Returns array with 2 responses (notification gets no response)
```

## Examples

The project includes several real-world examples in the `tests/` directory:

### 1. Unit Tests (`tests/unit_tests.cpp`)

Comprehensive test suite covering:

- Request/response validation
- Message builders
- Dispatcher functionality
- Endpoint communication
- Error handling

### 2. Calculator Service (`tests/calculator_service.cpp`)

A complete calculator with:

- Basic operations: add, subtract, multiply, divide
- Advanced operations: power, sqrt, factorial
- Memory operations: store, recall, clear
- Error handling (division by zero, invalid inputs)

### 3. Database/CRUD Service (`tests/database_service.cpp`)

In-memory user database with:

- Create, Read, Update, Delete operations
- Query and filtering
- Batch operations
- Statistics

### 4. JSON Basics (`tests/json_basics.cpp`)

Interactive tutorial covering:

- JSON data types
- Arrays and objects
- Nested structures
- Parsing and serialization

### 5. JSON-RPC Fundamentals (`tests/jsonrpc_fundamentals.cpp`)

Comprehensive JSON-RPC 2.0 tutorial:

- Request/response structure
- Error handling
- Notifications
- Batch requests
- Best practices

### 6. Advanced Features (`tests/advanced_features.cpp`)

Advanced topics:

- Endpoint-based communication
- Progress reporting
- Request cancellation
- Batch processing optimization
- Context management

## Tutorials

The project includes two comprehensive markdown tutorials:

### [JSON Tutorial](JSON.md)

Learn JSON from scratch:

- What is JSON?
- JSON syntax and data types
- Working with JSON in C++
- Common use cases
- Best practices

### [JSON-RPC 2.0 Tutorial](JSONRPC2.md)

Master JSON-RPC 2.0:

- Specification overview
- Message types
- Request/response objects
- Error handling
- Notifications and batch requests
- Using this library
- Real-world examples
- Best practices

## API Documentation

### Core Functions

#### Message Builders

```cpp
// Create request with ID
json make_request(
    const variant<nullptr_t, string, int64_t, uint64_t>& id,
    const string& method,
    const json& params = json{}
);

// Create notification (no ID)
json make_notification(
    const string& method,
    const json& params = json{}
);

// Create success response
json make_result(const json& id, const json& result);

// Create error response
json make_error(const json& id, const error& e);
```

#### Validation

```cpp
bool validate_request(const json& j, string* why = nullptr);
bool validate_response(const json& j, string* why = nullptr);
bool is_request(const json& j);
bool is_notification(const json& j);
bool is_response(const json& j);
```

### Dispatcher Class

```cpp
class dispatcher {
    using handler_t = function<json(const json& params)>;
    
    // Register method handler
    void add(const string& method, handler_t fn);
    
    // Handle single request/notification
    optional<json> handle_single(const json& msg) const;
    
    // Handle single or batch request
    optional<json> handle(const json& input) const;
};
```

### Endpoint Class

```cpp
class endpoint {
    // Constructor with message sender
    explicit endpoint(send_fn sender);
    
    // Server side: register methods
    void add(const string& method, handler_t fn);
    
    // Client side: send requests
    string send_request(
        const string& method,
        const json& params,
        result_cb on_result,
        error_cb on_error
    );
    
    // Send notifications
    void send_notification(const string& method, const json& params);
    
    // Progress reporting
    void send_progress(const string& token, const json& value);
    
    // Cancellation
    void cancel(const json& id);
    
    // Handle incoming messages
    void receive(const json& msg);
};
```

### Error Codes

Standard JSON-RPC 2.0 error codes:

| Code | Error |
|------|-------|
| -32700 | Parse error |
| -32600 | Invalid Request |
| -32601 | Method not found |
| -32602 | Invalid params |
| -32603 | Internal error |
| -32000 to -32099 | Server errors |

## Testing

### Run All Tests

```bash
# Build and run
./builder --release --executable
./build/release/jsonrpc2
```

### Test Results

The test suite includes:

- **31 unit tests** covering all library functions
- **5 integration examples** demonstrating real-world usage
- **100% pass rate** on all tests

Expected output:

```
═══════════════════════════════════════════════════════════
  Final Results
═══════════════════════════════════════════════════════════
  ✓ Passed: 6
  ✗ Failed: 0
═══════════════════════════════════════════════════════════

All tutorials completed successfully! ✓
```

## Project Structure

```
jsonrpc2/
├── include/
│   ├── json.hpp           # nlohmann/json library
│   └── jsonrpc.hpp        # JSON-RPC 2.0 implementation (header-only)
├── src/
│   └── main.cpp           # Tutorial runner
├── tests/
│   ├── unit_tests.cpp             # Comprehensive unit tests
│   ├── calculator_service.cpp     # Calculator example
│   ├── database_service.cpp       # CRUD database example
│   ├── json_basics.cpp            # JSON tutorial
│   ├── jsonrpc_fundamentals.cpp   # JSON-RPC tutorial
│   └── advanced_features.cpp      # Advanced features demo
├── build/
│   ├── debug/             # Debug builds
│   └── release/           # Release builds
├── builder.cpp            # Custom build system
├── JSON.md                # JSON tutorial
├── JSONRPC2.md            # JSON-RPC 2.0 tutorial
└── README.md              # This file
```

## Features Detail

### ✅ Full JSON-RPC 2.0 Compliance

- Request/response protocol
- Notifications (fire-and-forget)
- Batch requests
- Standard error codes
- Parameter validation
- ID handling (string, number, null)

### ✅ Advanced Features

- **Progress Reporting**: Send progress updates for long-running operations
- **Request Cancellation**: Cancel in-flight requests
- **Context Management**: Access request context in handlers
- **Batch Processing**: Efficiently handle multiple requests
- **Initialization Protocol**: LSP-style initialization handshake

### ✅ Error Handling

- Standard error codes
- Custom error data
- Exception-based error throwing
- Automatic error wrapping

### ✅ Validation

- Request validation
- Response validation
- Parameter type checking
- ID type validation

## Performance

- **Header-only**: No compilation overhead
- **Zero-copy**: Efficient JSON handling with nlohmann/json
- **Modern C++**: Takes advantage of C++23 features
- **Optimized**: Release builds use -O3 optimization

## Use Cases

This library is perfect for:

- **Language Server Protocol (LSP)** implementations
- **Blockchain RPC** interfaces
- **Microservices** communication
- **Remote API** servers
- **IoT device** control
- **Game server** protocols
- **Any RPC** needs

## Contributing

Contributions are welcome! Areas for improvement:

- WebSocket transport layer
- HTTP transport layer
- Asynchronous request handling
- More examples
- Additional validation
- Performance benchmarks

## License

This project is open source. See the individual file headers for licensing information.

## Author

**Pooria Yousefi**

- GitHub: [@pooriayousefi](https://github.com/pooriayousefi)

## Acknowledgments

- [nlohmann/json](https://github.com/nlohmann/json) - JSON for Modern C++
- [JSON-RPC 2.0 Specification](https://www.jsonrpc.org/specification)

## Resources

- [JSON Tutorial](JSON.md) - Learn JSON
- [JSON-RPC 2.0 Tutorial](JSONRPC2.md) - Learn JSON-RPC
- [Official JSON-RPC Spec](https://www.jsonrpc.org/specification)
- [nlohmann/json Documentation](https://json.nlohmann.me/)

---

**Made with ❤️ using C++23**

For questions or support, please open an issue on GitHub.
