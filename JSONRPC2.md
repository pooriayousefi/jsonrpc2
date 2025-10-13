# JSON-RPC 2.0 Tutorial

## Table of Contents

1. [Introduction](#introduction)
2. [What is JSON-RPC?](#what-is-json-rpc)
3. [Specification Overview](#specification-overview)
4. [Message Types](#message-types)
5. [Request Objects](#request-objects)
6. [Response Objects](#response-objects)
7. [Error Handling](#error-handling)
8. [Notifications](#notifications)
9. [Batch Requests](#batch-requests)
10. [Using This Library](#using-this-library)
11. [Examples](#examples)
12. [Advanced Features](#advanced-features)
13. [Best Practices](#best-practices)
14. [Resources](#resources)

---

## Introduction

**JSON-RPC** is a stateless, light-weight remote procedure call (RPC) protocol. It uses JSON (RFC 4627) as its data format and can be used over any transport layer (HTTP, WebSocket, TCP, etc.).

### Why JSON-RPC 2.0?

- **Simple**: Minimal overhead, easy to understand
- **Language-agnostic**: Works with any programming language
- **Transport-agnostic**: Use HTTP, WebSocket, or any other transport
- **Widely adopted**: Used in blockchain, LSP (Language Server Protocol), and many APIs
- **Standardized**: Clear specification (JSON-RPC 2.0 Specification)

---

## What is JSON-RPC?

JSON-RPC is a **remote procedure call** protocol encoded in JSON. It allows:

1. **Calling methods remotely**: Invoke functions on a remote server
2. **Receiving results**: Get return values from remote functions
3. **Error reporting**: Handle errors in a standardized way
4. **Notifications**: Send one-way messages without expecting responses

### Key Concepts

- **Client**: Sends requests to invoke remote methods
- **Server**: Receives requests, executes methods, returns responses
- **Method**: A function name to be invoked on the server
- **Parameters**: Arguments passed to the method
- **ID**: Identifier to match responses with requests

---

## Specification Overview

JSON-RPC 2.0 is defined by a simple specification:

### Core Principles

1. **Version Field**: All messages must include `"jsonrpc": "2.0"`
2. **Three Message Types**: Request, Response, Notification
3. **Request-Response Pattern**: Requests with IDs receive responses
4. **Fire-and-Forget**: Notifications without IDs receive no response
5. **Error Handling**: Standardized error codes and structure

### Transport Independence

JSON-RPC doesn't specify a transport layer. Common choices:

- **HTTP/HTTPS**: Most common, RESTful-style
- **WebSocket**: For persistent connections
- **TCP/Unix Sockets**: For local or high-performance communication
- **Message Queues**: For asynchronous processing

---

## Message Types

JSON-RPC 2.0 defines three message types:

### 1. Request

A request to invoke a method on the server.

```json
{
  "jsonrpc": "2.0",
  "method": "subtract",
  "params": [42, 23],
  "id": 1
}
```

### 2. Response

A response from the server (success or error).

```json
{
  "jsonrpc": "2.0",
  "result": 19,
  "id": 1
}
```

### 3. Notification

A request without an ID, requiring no response.

```json
{
  "jsonrpc": "2.0",
  "method": "update",
  "params": {"status": "running"}
}
```

---

## Request Objects

A request object represents a call to a remote method.

### Required Fields

| Field | Type | Description |
|-------|------|-------------|
| `jsonrpc` | String | Must be exactly `"2.0"` |
| `method` | String | Name of the method to invoke |
| `id` | String/Number/Null | Request identifier (omit for notifications) |

### Optional Fields

| Field | Type | Description |
|-------|------|-------------|
| `params` | Array/Object | Method parameters |

### Examples

#### Request with Positional Parameters

```json
{
  "jsonrpc": "2.0",
  "method": "subtract",
  "params": [42, 23],
  "id": 1
}
```

The server interprets: `subtract(42, 23)`

#### Request with Named Parameters

```json
{
  "jsonrpc": "2.0",
  "method": "subtract",
  "params": {
    "minuend": 42,
    "subtrahend": 23
  },
  "id": 2
}
```

The server interprets: `subtract(minuend=42, subtrahend=23)`

#### Request with No Parameters

```json
{
  "jsonrpc": "2.0",
  "method": "getServerTime",
  "id": 3
}
```

#### Request with String ID

```json
{
  "jsonrpc": "2.0",
  "method": "getData",
  "params": {"key": "user123"},
  "id": "request-abc-123"
}
```

### Valid ID Types

- **String**: `"abc-123"`, `"request-1"`
- **Number (integer)**: `1`, `42`, `1000`
- **Number (unsigned)**: `0`, `100`
- **Null**: Only for error responses to invalid requests

**Invalid** (will cause errors):

- Floating point numbers: `3.14`
- Objects: `{"id": 1}`
- Arrays: `[1, 2]`

---

## Response Objects

A response object is returned by the server for every request (except notifications).

### Success Response

Contains the result of the method call.

#### Required Fields

| Field | Type | Description |
|-------|------|-------------|
| `jsonrpc` | String | Must be exactly `"2.0"` |
| `result` | Any | The result value (can be any JSON type, including `null`) |
| `id` | String/Number | Must match the request ID |

#### Example

```json
{
  "jsonrpc": "2.0",
  "result": 19,
  "id": 1
}
```

### Error Response

Contains error information when a method call fails.

#### Required Fields

| Field | Type | Description |
|-------|------|-------------|
| `jsonrpc` | String | Must be exactly `"2.0"` |
| `error` | Object | Error object (see below) |
| `id` | String/Number/Null | Request ID, or `null` if ID couldn't be determined |

#### Error Object Structure

| Field | Type | Description |
|-------|------|-------------|
| `code` | Integer | Error code (see standard codes below) |
| `message` | String | Short error description |
| `data` | Any | Optional additional error information |

#### Example

```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -32601,
    "message": "Method not found",
    "data": {
      "attempted": "unknownMethod"
    }
  },
  "id": 1
}
```

### Important Rules

1. A response **MUST** contain either `result` or `error`, **never both**
2. The `id` **MUST** match the request `id`
3. The `result` can be `null` (it's a valid result value)

---

## Error Handling

JSON-RPC 2.0 defines standard error codes.

### Standard Error Codes

| Code | Message | Meaning |
|------|---------|---------|
| `-32700` | Parse error | Invalid JSON received |
| `-32600` | Invalid Request | Request doesn't conform to specification |
| `-32601` | Method not found | Method doesn't exist or isn't available |
| `-32602` | Invalid params | Invalid method parameters |
| `-32603` | Internal error | Internal JSON-RPC error |

### Server Error Range

Codes from `-32000` to `-32099` are reserved for application-defined server errors.

```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -32000,
    "message": "Database connection failed",
    "data": {
      "host": "db.example.com",
      "error": "timeout after 30s"
    }
  },
  "id": 5
}
```

### Error Examples

#### Parse Error

Sent when the server receives invalid JSON.

```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -32700,
    "message": "Parse error"
  },
  "id": null
}
```

#### Method Not Found

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

#### Invalid Parameters

```json
{
  "jsonrpc": "2.0",
  "error": {
    "code": -32602,
    "message": "Invalid params",
    "data": {
      "expected": "array of 2 numbers",
      "received": "string"
    }
  },
  "id": 2
}
```

---

## Notifications

Notifications are requests **without an `id` field**. The server **MUST NOT** respond to notifications.

### Use Cases

- Logging and monitoring
- Event notifications
- Progress updates
- One-way messages

### Example

```json
{
  "jsonrpc": "2.0",
  "method": "log",
  "params": {
    "level": "info",
    "message": "User logged in",
    "userId": 12345
  }
}
```

**Key Point**: No `id` field means no response expected!

### Multiple Notifications

```json
{
  "jsonrpc": "2.0",
  "method": "notify_hello",
  "params": [7]
}

{
  "jsonrpc": "2.0",
  "method": "notify_status",
  "params": {"status": "running"}
}
```

Server processes both but sends no responses.

---

## Batch Requests

Send multiple requests in a single message using a JSON array.

### Batch Request Example

```json
[
  {"jsonrpc": "2.0", "method": "sum", "params": [1,2,4], "id": "1"},
  {"jsonrpc": "2.0", "method": "subtract", "params": [42,23], "id": "2"},
  {"jsonrpc": "2.0", "method": "get_data", "id": "9"}
]
```

### Batch Response Example

```json
[
  {"jsonrpc": "2.0", "result": 7, "id": "1"},
  {"jsonrpc": "2.0", "result": 19, "id": "2"},
  {"jsonrpc": "2.0", "result": ["hello", 5], "id": "9"}
]
```

### Important Rules

1. **Empty array is invalid**: `[]` returns an error
2. **Response order doesn't matter**: Responses may be in any order
3. **Notifications don't get responses**: Only requests with IDs get responses
4. **All notifications = no response**: If batch contains only notifications, no response at all

### Mixed Batch (Requests + Notifications)

```json
[
  {"jsonrpc": "2.0", "method": "sum", "params": [1,2], "id": "1"},
  {"jsonrpc": "2.0", "method": "notify", "params": {"msg": "processing"}},
  {"jsonrpc": "2.0", "method": "subtract", "params": [10,5], "id": "2"}
]
```

Response (only for requests):

```json
[
  {"jsonrpc": "2.0", "result": 3, "id": "1"},
  {"jsonrpc": "2.0", "result": 5, "id": "2"}
]
```

---

## Using This Library

This library provides a complete C++ implementation of JSON-RPC 2.0.

### Basic Components

1. **Builder Functions**: Create requests, responses, errors
2. **Validator Functions**: Validate JSON-RPC messages
3. **Dispatcher**: Route and handle method calls
4. **Endpoint**: Complete client/server communication

### Including the Library

```cpp
#include "jsonrpc.hpp"
using namespace pooriayousefi;
using json = nlohmann::json;
```

### Creating Messages

#### Create Request

```cpp
// With positional parameters
json req = make_request(1, "add", json::array({5, 3}));

// With named parameters
json req = make_request(2, "greet", json::object({{"name", "Alice"}}));

// With string ID
json req = make_request(std::string("req-123"), "getData", json::array());
```

#### Create Notification

```cpp
json notif = make_notification("log", json::object({
    {"level", "info"},
    {"message", "Server started"}
}));
```

#### Create Success Response

```cpp
json resp = make_result(1, json(42));
```

#### Create Error Response

```cpp
error err{-32601, "Method not found"};
json resp = make_error(1, err);

// With additional data
error err_detail{-32000, "Database error", json{{"host", "db.example.com"}}};
json resp = make_error(2, err_detail);
```

### Validating Messages

```cpp
json msg = /* ... */;
std::string why;

if (validate_request(msg, &why)) {
    std::cout << "Valid request\n";
} else {
    std::cout << "Invalid: " << why << "\n";
}
```

### Using the Dispatcher

```cpp
dispatcher d;

// Register methods
d.add("add", [](const json& params) -> json {
    return params[0].get<int>() + params[1].get<int>();
});

d.add("greet", [](const json& params) -> json {
    std::string name = params["name"].get<std::string>();
    return "Hello, " + name + "!";
});

// Handle requests
json request = make_request(1, "add", json::array({5, 3}));
auto response = d.handle(request);

if (response) {
    std::cout << "Response: " << response->dump() << "\n";
}
```

### Throwing Errors in Handlers

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

### Using Endpoints

```cpp
// Create server endpoint
endpoint server([](const json& msg) {
    // Send message to client
    send_to_client(msg);
});

// Register methods
server.add("echo", [](const json& params) -> json {
    return params;
});

// Handle incoming messages
server.receive(request_from_client);

// Client side
endpoint client([](const json& msg) {
    // Send message to server
    send_to_server(msg);
});

// Send request
client.send_request("echo", json::array({"test"}),
    [](const json& result) {
        std::cout << "Result: " << result << "\n";
    },
    [](const json& error) {
        std::cout << "Error: " << error << "\n";
    }
);
```

---

## Examples

### Example 1: Simple Calculator

**Server:**

```cpp
dispatcher calc;

calc.add("add", [](const json& params) -> json {
    return params[0].get<int>() + params[1].get<int>();
});

calc.add("subtract", [](const json& params) -> json {
    return params[0].get<int>() - params[1].get<int>();
});

calc.add("multiply", [](const json& params) -> json {
    return params[0].get<int>() * params[1].get<int>();
});

// Handle request
json req = make_request(1, "multiply", json::array({6, 7}));
auto resp = calc.handle(req);
// Response: {"jsonrpc":"2.0","result":42,"id":1}
```

### Example 2: User Management

**Server:**

```cpp
std::map<int, std::string> users;
int next_id = 1;

dispatcher user_service;

user_service.add("user.create", [&](const json& params) -> json {
    std::string name = params["name"].get<std::string>();
    int id = next_id++;
    users[id] = name;
    return json{{"id", id}, {"name", name}};
});

user_service.add("user.get", [&](const json& params) -> json {
    int id = params["id"].get<int>();
    if (users.find(id) == users.end()) {
        throw rpc_exception(error{-32001, "User not found"});
    }
    return json{{"id", id}, {"name", users[id]}};
});

user_service.add("user.list", [&](const json& params) -> json {
    json result = json::array();
    for (const auto& [id, name] : users) {
        result.push_back(json{{"id", id}, {"name", name}});
    }
    return result;
});
```

### Example 3: Event Notifications

**Server:**

```cpp
dispatcher event_service;

event_service.add("log", [](const json& params) -> json {
    std::string level = params["level"].get<std::string>();
    std::string message = params["message"].get<std::string>();
    std::cout << "[" << level << "] " << message << "\n";
    return nullptr; // Won't be sent (notification)
});

// Client sends notification
json notif = make_notification("log", json{
    {"level", "info"},
    {"message", "Application started"}
});

event_service.handle(notif); // No response returned
```

---

## Advanced Features

### 1. Progress Reporting

```cpp
server.add("long_task", [](const json& params) -> json {
    int total = params["count"].get<int>();
    
    for (int i = 1; i <= total; ++i) {
        // Do work...
        
        // Report progress
        report_progress(json{
            {"current", i},
            {"total", total},
            {"percentage", (i * 100) / total}
        });
    }
    
    return json{{"status", "completed"}};
});
```

### 2. Request Cancellation

```cpp
server.add("cancellable_task", [](const json& params) -> json {
    for (int i = 0; i < 1000; ++i) {
        // Check for cancellation
        if (is_canceled()) {
            throw rpc_exception(error{-32800, "Request cancelled"});
        }
        
        // Do work...
    }
    
    return json{{"status", "completed"}};
});

// Client cancels request
client.cancel(request_id);
```

### 3. Initialization Protocol

```cpp
// Server sets capabilities
server.set_server_capabilities(json{
    {"textDocumentSync", 1},
    {"completionProvider", true},
    {"hoverProvider", true}
});

// Client initializes
client.initialize(
    json{{"processId", 12345}},
    [](const json& result) {
        // Server capabilities received
        auto caps = result["capabilities"];
    },
    [](const json& error) {
        // Initialization failed
    }
);
```

### 4. Context-Aware Handlers

```cpp
server.add("context_method", [](const json& params) -> json {
    const call_context* ctx = current_context();
    
    if (ctx) {
        // Access request ID
        json id = ctx->id;
        
        // Report progress
        report_progress(json{{"status", "processing"}});
        
        // Check cancellation
        if (is_canceled()) {
            throw rpc_exception(error{-32800, "Cancelled"});
        }
    }
    
    return json{{"status", "ok"}};
});
```

---

## Best Practices

### 1. Use Meaningful Method Names

**Good:**

- `user.create`
- `order.list`
- `payment.process`

**Bad:**

- `method1`
- `doStuff`
- `func`

### 2. Validate Parameters

```cpp
d.add("divide", [](const json& params) -> json {
    if (!params.is_array() || params.size() != 2) {
        throw rpc_exception(error{
            -32602, 
            "Invalid params",
            json{{"expected", "array of 2 numbers"}}
        });
    }
    
    // ... perform division
});
```

### 3. Provide Detailed Error Information

```cpp
throw rpc_exception(error{
    -32000,
    "Database connection failed",
    json{
        {"host", "db.example.com"},
        {"port", 5432},
        {"error", "connection timeout"},
        {"retry_after", 30}
    }
});
```

### 4. Use Notifications Appropriately

- **Use for**: Logging, events, one-way updates
- **Don't use for**: Operations requiring confirmation
- **Remember**: No response means no error reporting

### 5. Handle Batch Requests Efficiently

```cpp
// Process batch requests in parallel when possible
auto handle_batch_parallel(const json& batch) {
    std::vector<std::future<json>> futures;
    
    for (const auto& req : batch) {
        futures.push_back(std::async([&]() {
            return handle_single_request(req);
        }));
    }
    
    json responses = json::array();
    for (auto& fut : futures) {
        responses.push_back(fut.get());
    }
    
    return responses;
}
```

### 6. Document Your API

Provide clear documentation:

```markdown
### user.create

Creates a new user.

**Parameters:**
- `name` (string, required): User's full name
- `email` (string, required): User's email address
- `age` (integer, optional): User's age

**Returns:**
```json
{
  "id": 123,
  "name": "John Doe",
  "email": "john@example.com"
}
```

**Errors:**

- `-32602`: Invalid parameters
- `-32000`: Email already exists

```

### 7. Version Your API

Include version in method names or capabilities:

```json
{
  "jsonrpc": "2.0",
  "method": "v2.user.create",
  "params": {...}
}
```

### 8. Log All Requests and Responses

```cpp
auto logged_handler = [original_handler](const json& params) -> json {
    log_request(params);
    try {
        auto result = original_handler(params);
        log_response(result);
        return result;
    } catch (const rpc_exception& e) {
        log_error(e);
        throw;
    }
};
```

### 9. Set Appropriate Timeouts

```cpp
// Client-side timeout
auto timeout = std::chrono::seconds(30);
auto future = std::async([&]() { return send_request(...); });

if (future.wait_for(timeout) == std::future_status::timeout) {
    // Handle timeout
    cancel_request(request_id);
}
```

### 10. Test Error Cases

Test all error scenarios:

- Invalid JSON
- Missing required fields
- Wrong parameter types
- Method not found
- Internal errors
- Cancellation
- Timeouts

---

## Resources

### Official Specification

- [JSON-RPC 2.0 Specification](https://www.jsonrpc.org/specification)

### Related Protocols

- [Language Server Protocol (LSP)](https://microsoft.github.io/language-server-protocol/) - Uses JSON-RPC 2.0
- [Ethereum JSON-RPC](https://ethereum.org/en/developers/docs/apis/json-rpc/) - Blockchain API

### Tools

- [Postman](https://www.postman.com/) - Test JSON-RPC APIs over HTTP
- [wscat](https://github.com/websockets/wscat) - WebSocket client for testing

### Libraries

- This library: `pooriayousefi/jsonrpc2`
- [nlohmann/json](https://github.com/nlohmann/json) - JSON for Modern C++

### Further Reading

- [JSON Tutorial](JSON.md) - Understanding JSON basics
- Examples in `tests/` directory
- Source code in `include/jsonrpc.hpp`

---

## Conclusion

JSON-RPC 2.0 provides a simple, standardized way to implement remote procedure calls. Its simplicity makes it easy to implement, while its flexibility makes it suitable for a wide range of applications.

**Key Takeaways:**

1. Always include `"jsonrpc": "2.0"`
2. Requests with IDs get responses, notifications don't
3. Use standard error codes when applicable
4. Batch requests improve efficiency
5. Validate inputs and handle errors gracefully

This library provides everything you need to build robust JSON-RPC 2.0 services in C++!

---

**Next Steps:**

- Run the tutorial programs in `tests/`
- Build your own JSON-RPC service
- Explore advanced features like progress and cancellation
- Read the examples for real-world use cases
