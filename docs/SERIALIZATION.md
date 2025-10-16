# Serialization/Deserialization Feature

## Overview

The JSON-RPC 2.0 library now supports **automatic serialization and deserialization** of C++ types! Instead of manually converting between JSON and C++ types, you can now register handlers that work directly with C++ types.

## Key Benefits

1. **Type Safety**: Compile-time type checking
2. **Less Boilerplate**: No manual JSON conversion
3. **Better Error Messages**: Automatic invalid params detection
4. **Custom Types**: Works with any type that can be serialized/deserialized by nlohmann::json

## Basic Usage

### Before (Raw JSON)

```cpp
dispatcher disp;
disp.add("add", [](const json &params) -> json {
    int a = params["a"].get<int>();
    int b = params["b"].get<int>();
    return a + b;
});
```

### After (Typed)

```cpp
struct AddParams {
    int a;
    int b;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AddParams, a, b)

dispatcher disp;
disp.add_typed<AddParams, int>("add", [](AddParams params) {
    return params.a + params.b;
});
```

## Features

### 1. Basic Types

```cpp
// int -> int
disp.add_typed<int, int>("double", [](int x) {
    return x * 2;
});

// string -> string
disp.add_typed<std::string, std::string>("greet", [](std::string name) {
    return "Hello, " + name + "!";
});
```

### 2. Custom Structs

```cpp
struct Point {
    double x;
    double y;
};

// Define serialization
namespace nlohmann {
    template <>
    struct adl_serializer<Point> {
        static void to_json(json &j, const Point &p) {
            j = json{{"x", p.x}, {"y", p.y}};
        }
        
        static void from_json(const json &j, Point &p) {
            j.at("x").get_to(p.x);
            j.at("y").get_to(p.y);
        }
    };
}

// Use it
disp.add_typed<Point, Point>("move_point", [](Point p) {
    p.x += 10;
    p.y += 20;
    return p;
});
```

### 3. STL Containers

```cpp
// vector<int> -> vector<int>
disp.add_typed<std::vector<int>, std::vector<int>>("double_all", 
    [](std::vector<int> nums) {
        for (auto &n : nums) n *= 2;
        return nums;
    }
);
```

### 4. Void Returns

```cpp
int counter = 0;

// int -> void (notification-style)
disp.add_typed<int, void>("increment", [&](int x) {
    counter += x;
});
```

### 5. No Parameters

```cpp
// void -> int
disp.add_no_params<int>("get_random", []() {
    return rand() % 100;
});
```

### 6. Complex Types

```cpp
struct User {
    std::string name;
    int age;
    std::vector<std::string> hobbies;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(User, name, age, hobbies)

disp.add_typed<User, User>("update_user", [](User u) {
    u.age += 1;  // Birthday!
    return u;
});
```

## JSON-RPC 2.0 Compliance

The library automatically handles JSON-RPC 2.0 parameter format requirements:

- **Single parameters** are wrapped in an array: `[5]`
- **Struct parameters** are serialized as objects: `{"a": 1, "b": 2}`
- **Array parameters** are passed as-is: `[1, 2, 3]`

### Example Requests

```json
// Single int parameter
{
  "jsonrpc": "2.0",
  "method": "double",
  "params": [5],
  "id": 1
}

// Struct parameter
{
  "jsonrpc": "2.0",
  "method": "add",
  "params": [{"a": 10, "b": 20}],
  "id": 2
}

// Array parameter
{
  "jsonrpc": "2.0",
  "method": "sum",
  "params": [[1, 2, 3, 4, 5]],
  "id": 3
}
```

## Client-Side Typed Requests

You can also use typed methods on the client side:

```cpp
endpoint ep(sender);

// Send typed request
ep.send_request_typed<std::string, std::string>(
    "greet",
    std::string("Alice"),
    [](std::string result) {
        std::cout << "Got: " << result << "\n";
    },
    [](const json &error) {
        std::cerr << "Error: " << error["message"] << "\n";
    }
);

// Send typed notification
ep.send_notification_typed<int>("log_value", 42);
```

## Error Handling

Invalid parameters are automatically detected:

```cpp
disp.add_typed<int, int>("need_int", [](int x) {
    return x * 2;
});

// If client sends: {"jsonrpc": "2.0", "method": "need_int", "params": ["not_a_number"], "id": 1}
// Response: {
//   "jsonrpc": "2.0",
//   "id": 1,
//   "error": {
//     "code": -32602,
//     "message": "Invalid params",
//     "data": {"what": "type must be number, but is string"}
//   }
// }
```

## Mixing Raw JSON and Typed Handlers

You can use both styles in the same dispatcher:

```cpp
dispatcher disp;

// Raw JSON handler
disp.add("raw_method", [](const json &params) -> json {
    return params["value"].get<int>() * 3;
});

// Typed handler
disp.add_typed<int, int>("typed_method", [](int x) {
    return x * 3;
});

// Both work!
```

## Complete Example

```cpp
#include "include/jsonrpc.hpp"
#include <iostream>

using namespace pooriayousefi;

struct Calculator {
    double memory = 0.0;
};

struct CalcRequest {
    std::string operation;
    double a;
    double b;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CalcRequest, operation, a, b)

int main() {
    Calculator calc;
    dispatcher disp;

    // Typed method: CalcRequest -> double
    disp.add_typed<CalcRequest, double>("calculate", 
        [&](CalcRequest req) -> double {
            if (req.operation == "add") return req.a + req.b;
            if (req.operation == "sub") return req.a - req.b;
            if (req.operation == "mul") return req.a * req.b;
            if (req.operation == "div") {
                if (req.b == 0) {
                    throw_rpc_error(error{-32000, "Division by zero"});
                }
                return req.a / req.b;
            }
            throw_rpc_error(error{-32001, "Unknown operation"});
        });

    // Typed method: double -> void (store in memory)
    disp.add_typed<double, void>("store", [&](double value) {
        calc.memory = value;
    });

    // No params: return stored value
    disp.add_no_params<double>("recall", [&]() {
        return calc.memory;
    });

    // Test it
    json req1 = make_request(1, "calculate", 
        json::array({json{{"operation", "add"}, {"a", 10.0}, {"b", 20.0}}}));
    
    auto resp1 = disp.handle(req1);
    std::cout << "Result: " << (*resp1)["result"] << "\n";  // 30.0

    json req2 = make_request(2, "store", json::array({42.5}));
    disp.handle(req2);

    json req3 = make_request(3, "recall", json::object());
    auto resp3 = disp.handle(req3);
    std::cout << "Memory: " << (*resp3)["result"] << "\n";  // 42.5

    return 0;
}
```

## API Reference

### Dispatcher Methods

- `add_typed<ParamsT, ResultT>(method, handler)` - Register typed handler
- `add_no_params<ResultT>(method, handler)` - Register handler with no parameters
- `add(method, handler)` - Register raw JSON handler (original method)

### Endpoint Methods

- `add_typed<ParamsT, ResultT>(method, handler)` - Server-side typed handler
- `add_no_params<ResultT>(method, handler)` - Server-side no-params handler
- `send_request_typed<ParamsT, ResultT>(method, params, on_result, on_error)` - Client-side typed request
- `send_notification_typed<ParamsT>(method, params)` - Client-side typed notification

## Notes

1. **Parameter Format**: Single parameters are wrapped in arrays for JSON-RPC 2.0 compliance
2. **Type Safety**: Invalid params trigger `-32602` error with details
3. **Performance**: Zero overhead - templates resolve at compile time
4. **Compatibility**: Works alongside existing raw JSON handlers
5. **Flexibility**: Supports any type that nlohmann::json can serialize/deserialize

## Migration Guide

To migrate existing code:

1. **Identify handlers** that work with structured data
2. **Define C++ structs** for your parameter types
3. **Add serialization** using `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE` or custom `adl_serializer`
4. **Replace** `add(...)` with `add_typed<ParamsT, ResultT>(...)`
5. **Simplify** handler logic by removing manual `.get<>()` calls

### Example Migration

**Before:**

```cpp
disp.add("create_user", [](const json &params) -> json {
    std::string name = params["name"].get<std::string>();
    int age = params["age"].get<int>();
    
    User u{name, age, {}};
    users.push_back(u);
    
    return json{{"id", users.size() - 1}};
});
```

**After:**

```cpp
struct CreateUserParams {
    std::string name;
    int age;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CreateUserParams, name, age)

disp.add_typed<CreateUserParams, int>("create_user", 
    [](CreateUserParams params) {
        User u{params.name, params.age, {}};
        users.push_back(u);
        return users.size() - 1;
    }
);
```

Much cleaner! 🎉
