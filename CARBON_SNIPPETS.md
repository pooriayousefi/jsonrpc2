# Carbon.sh Code Snippets for Medium Article

This file contains all 21 code snippets you need to create using Carbon.sh (https://carbon.now.sh)

**Carbon.sh Settings Recommendation:**
- Theme: Dracula or Monokai
- Language: JSON or C++ (as indicated)
- Font: Fira Code or JetBrains Mono
- Background: Yes (with padding)
- Drop shadow: Yes
- Export as: PNG (2x resolution)

---

## SNIPPET 1: Request Example
**Language**: JSON

```json
{
  "jsonrpc": "2.0",
  "method": "subtract",
  "params": [42, 23],
  "id": 1
}
```

---

## SNIPPET 2: Success Response
**Language**: JSON

```json
{
  "jsonrpc": "2.0",
  "result": 19,
  "id": 1
}
```

---

## SNIPPET 3: Error Response
**Language**: JSON

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

---

## SNIPPET 4: Notification Example
**Language**: JSON

```json
{
  "jsonrpc": "2.0",
  "method": "update",
  "params": ["some_data"]
}
```

---

## SNIPPET 5: Modern C++ Features
**Language**: C++

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

---

## SNIPPET 6: Architecture
**Language**: C++

```cpp
// Message builders
json request = make_request(1, "add", json::array({5, 3}));
json response = make_result(1, 8);
json error = make_error(1, {-32601, "Not found"});

// Dispatcher (server)
dispatcher server;
server.add("add", [](const json& p) { 
    return p[0].get<int>() + p[1].get<int>(); 
});
auto response = server.handle(request);

// Endpoint (client/server)
endpoint client([](const json& msg) { 
    send_over_network(msg); 
});
client.send_request("add", json::array({5, 3}), 
    [](const json& result) { 
        std::cout << "Result: " << result << "\n"; 
    });
```

---

## SNIPPET 7: Validation
**Language**: C++

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

---

## SNIPPET 8: Dispatcher Implementation
**Language**: C++

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

---

## SNIPPET 9: Error Handling
**Language**: C++

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

---

## SNIPPET 10: Batch Processing
**Language**: C++

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

## SNIPPET 11: Calculator Service
**Language**: C++

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
        
        // Memory operations
        calc_.add("memory_store", [this](const json& p) -> json {
            memory_ = p[0].get<double>();
            return json{{"stored", memory_}};
        });
    }
    
    std::optional<json> handle(const json& request) {
        return calc_.handle(request);
    }
};
```

---

## SNIPPET 12: Calculator Usage
**Language**: C++

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
// Response: {"jsonrpc":"2.0","error":{"code":-32000,
//           "message":"Division by zero"},"id":3}
```

---

## SNIPPET 13: Database Service
**Language**: C++

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
            User user{next_id_++, p["name"].get<std::string>(),
                     p["email"].get<std::string>(), p["age"].get<int>()};
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
    }
};
```

---

## SNIPPET 14: Progress Reporting
**Language**: C++

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

---

## SNIPPET 15: Cancellation
**Language**: C++

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

---

## SNIPPET 16: Bidirectional Communication
**Language**: C++

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
```

---

## SNIPPET 17: Test Example
**Language**: C++

```cpp
TEST(validation_valid_request) {
    json req = make_request(1, "test", json::array());
    std::string why;
    ASSERT(validate_request(req, &why));
    ASSERT(why.empty());
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
    
    ASSERT(received);
}
```

---

## SNIPPET 18: Build System
**Language**: C++

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

---

## SNIPPET 19: Installation
**Language**: bash

```bash
# Clone the repository
git clone https://github.com/pooriayousefi/jsonrpc2.git

# Copy headers to your project
cp jsonrpc2/include/jsonrpc.hpp your_project/include/
cp jsonrpc2/include/json.hpp your_project/include/

# Include in your code
#include "jsonrpc.hpp"
```

---

## SNIPPET 20: Quick Start
**Language**: C++

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

---

## SNIPPET 21: Compilation
**Language**: bash

```bash
# Using g++
g++ -std=c++23 main.cpp -o my_app

# Using clang++
clang++ -std=c++23 main.cpp -o my_app

# With the included builder
./builder --release --executable
```

---

## Instructions for Creating Images:

1. Go to https://carbon.now.sh
2. For each snippet above:
   - Copy the code
   - Paste into Carbon
   - Select the appropriate language (JSON, C++, or bash)
   - Choose theme (Dracula recommended)
   - Choose font (Fira Code recommended)
   - Enable background and drop shadow
   - Export as PNG (2x resolution)
   - Name the file: `snippet_01.png`, `snippet_02.png`, etc.

3. When writing the Medium article:
   - Replace `**[CARBON SNIPPET 1: Request Example]**` with the image
   - Use Medium's image upload feature
   - Add captions if needed

**Pro Tip**: Create all 21 images in one session to maintain consistency in styling!
