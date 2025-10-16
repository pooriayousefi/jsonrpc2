#include "../include/jsonrpc.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

using namespace pooriayousefi;

// Example custom struct for demonstrating serialization
struct Point
{
    double x;
    double y;

    bool operator==(const Point &other) const { return x == other.x && y == other.y; }
};

// Define how to serialize/deserialize Point using nlohmann::json
namespace nlohmann
{
    template <> struct adl_serializer<Point>
    {
        static void to_json(json &j, const Point &p) { j = json{{"x", p.x}, {"y", p.y}}; }

        static void from_json(const json &j, Point &p)
        {
            j.at("x").get_to(p.x);
            j.at("y").get_to(p.y);
        }
    };
} // namespace nlohmann

// Another example struct
struct Person
{
    std::string name;
    int age;
    std::vector<std::string> hobbies;
};

// Serialization for Person using NLOHMANN macro
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Person, name, age, hobbies)

void test_basic_types()
{
    std::cout << "Test 1: Basic types (int -> int)\n";
    dispatcher disp;

    // Add a method that takes int and returns int
    disp.add_typed<int, int>("double", [](int x) { return x * 2; });

    // Params should be an array with the single value for JSON-RPC 2.0 compliance
    json req = make_request(1, "double", json::array({5}));
    auto resp = disp.handle(req);
    assert(resp.has_value());
    assert((*resp)["result"] == 10);
    std::cout << "  ✓ Passed\n\n";
}

void test_string_types()
{
    std::cout << "Test 2: String types (string -> string)\n";
    dispatcher disp;

    disp.add_typed<std::string, std::string>("greet", [](std::string name)
                                             { return "Hello, " + name + "!"; });

    json req = make_request(1, "greet", json::array({std::string("Alice")}));
    auto resp = disp.handle(req);
    assert(resp.has_value());
    assert((*resp)["result"] == "Hello, Alice!");
    std::cout << "  ✓ Passed\n\n";
}

void test_custom_struct()
{
    std::cout << "Test 3: Custom struct (Point -> Point)\n";
    dispatcher disp;

    // Method that moves a point
    disp.add_typed<Point, Point>("move",
                                 [](Point p)
                                 {
                                     p.x += 10.0;
                                     p.y += 20.0;
                                     return p;
                                 });

    Point input{5.0, 10.0};
    json req = make_request(1, "move", json::array({input}));
    auto resp = disp.handle(req);
    assert(resp.has_value());

    Point result = (*resp)["result"].get<Point>();
    assert(result.x == 15.0 && result.y == 30.0);
    std::cout << "  ✓ Passed\n\n";
}

void test_complex_struct()
{
    std::cout << "Test 4: Complex struct (Person -> Person)\n";
    dispatcher disp;

    disp.add_typed<Person, Person>("update_person",
                                   [](Person u)
                                   {
                                       u.age += 1;
                                       u.hobbies.push_back("reading");
                                       return u;
                                   });

    Person input{"Bob", 25, {"gaming", "coding"}};
    json req = make_request(1, "update_person", json::array({input}));
    auto resp = disp.handle(req);
    assert(resp.has_value());

    Person result = (*resp)["result"].get<Person>();
    assert(result.name == "Bob");
    assert(result.age == 26);
    assert(result.hobbies.size() == 3);
    assert(result.hobbies[2] == "reading");
    std::cout << "  ✓ Passed\n\n";
}

void test_void_return()
{
    std::cout << "Test 5: Void return type (int -> void)\n";
    dispatcher disp;

    int counter = 0;
    disp.add_typed<int, void>("increment", [&counter](int x) { counter += x; });

    json req = make_request(1, "increment", json::array({5}));
    auto resp = disp.handle(req);
    assert(resp.has_value());
    assert((*resp)["result"].is_null());
    assert(counter == 5);
    std::cout << "  ✓ Passed\n\n";
}

void test_no_params()
{
    std::cout << "Test 6: No parameters (void -> int)\n";
    dispatcher disp;

    disp.add_no_params<int>("get_magic", []() { return 42; });

    json req = make_request(1, "get_magic", json{});
    auto resp = disp.handle(req);
    assert(resp.has_value());
    assert((*resp)["result"] == 42);
    std::cout << "  ✓ Passed\n\n";
}

void test_vector_types()
{
    std::cout << "Test 7: Vector types (vector<int> -> vector<int>)\n";
    dispatcher disp;

    disp.add_typed<std::vector<int>, std::vector<int>>(
        "sum_pairs",
        [](std::vector<int> nums)
        {
            std::vector<int> result;
            for (size_t i = 0; i + 1 < nums.size(); i += 2)
            {
                result.push_back(nums[i] + nums[i + 1]);
            }
            return result;
        });

    std::vector<int> input{1, 2, 3, 4, 5, 6};
    json req = make_request(1, "sum_pairs", json::array({input}));
    auto resp = disp.handle(req);
    assert(resp.has_value());

    std::vector<int> result = (*resp)["result"].get<std::vector<int>>();
    assert(result.size() == 3);
    assert(result[0] == 3 && result[1] == 7 && result[2] == 11);
    std::cout << "  ✓ Passed\n\n";
}

void test_error_handling()
{
    std::cout << "Test 8: Error handling with typed params\n";
    dispatcher disp;

    disp.add_typed<int, int>("divide",
                             [](int x)
                             {
                                 if (x == 0)
                                 {
                                     throw_rpc_error(
                                         error{-32000, "Division by zero", json{{"attempted", x}}});
                                 }
                                 return 100 / x;
                             });

    json req = make_request(1, "divide", json::array({0}));
    auto resp = disp.handle(req);
    assert(resp.has_value());
    assert((*resp).contains("error"));
    assert((*resp)["error"]["code"] == -32000);
    assert((*resp)["error"]["message"] == "Division by zero");
    std::cout << "  ✓ Passed\n\n";
}

void test_invalid_params()
{
    std::cout << "Test 9: Invalid params detection\n";
    dispatcher disp;

    disp.add_typed<int, int>("need_int", [](int x) { return x * 2; });

    // Send string instead of int (wrapped in array)
    json req = make_request(1, "need_int", json::array({std::string("not_a_number")}));
    auto resp = disp.handle(req);
    assert(resp.has_value());
    assert((*resp).contains("error"));
    assert((*resp)["error"]["code"] == invalid_params.code);
    std::cout << "  ✓ Passed\n\n";
}

void test_endpoint_typed()
{
    std::cout << "Test 10: Endpoint with typed methods\n";

    std::vector<json> sent_messages;
    auto sender = [&sent_messages](const json &msg) { sent_messages.push_back(msg); };

    endpoint ep(sender);

    // Add typed method
    ep.add_typed<std::string, std::string>("echo", [](std::string msg) { return msg; });

    // Client sends request (wrapped in array for JSON-RPC 2.0)
    json req = make_request(1, "echo", json::array({std::string("Hello World")}));
    ep.receive(req);

    assert(sent_messages.size() == 1);
    assert(sent_messages[0]["result"] == "Hello World");
    std::cout << "  ✓ Passed\n\n";
}

void test_endpoint_client_typed()
{
    std::cout << "Test 11: Endpoint typed client request\n";

    std::vector<json> sent_messages;
    auto sender = [&sent_messages](const json &msg) { sent_messages.push_back(msg); };

    endpoint ep(sender);

    bool callback_called = false;
    std::string received_result;

    // Send typed request (automatically wrapped in array)
    ep.send_request_typed<std::string, std::string>("greet", std::string("World"),
                                                    [&](std::string result)
                                                    {
                                                        callback_called = true;
                                                        received_result = result;
                                                    });

    assert(sent_messages.size() == 1);
    assert(sent_messages[0]["method"] == "greet");
    // Params should be wrapped in array
    assert(sent_messages[0]["params"].is_array());
    assert(sent_messages[0]["params"][0] == "World");

    // Simulate response
    json response = make_result(sent_messages[0]["id"], std::string("Hello, World!"));
    ep.receive(response);

    assert(callback_called);
    assert(received_result == "Hello, World!");
    std::cout << "  ✓ Passed\n\n";
}

void test_mixed_json_and_typed()
{
    std::cout << "Test 12: Mix raw JSON and typed methods\n";
    dispatcher disp;

    // Raw JSON handler
    disp.add("raw_method",
             [](const json &params) -> json { return params["value"].get<int>() * 3; });

    // Typed handler
    disp.add_typed<int, int>("typed_method", [](int x) { return x * 3; });

    // Both should work
    json req1 = make_request(1, "raw_method", json{{"value", 5}});
    auto resp1 = disp.handle(req1);
    assert(resp1.has_value());
    assert((*resp1)["result"] == 15);

    json req2 = make_request(2, "typed_method", json::array({5}));
    auto resp2 = disp.handle(req2);
    assert(resp2.has_value());
    assert((*resp2)["result"] == 15);

    std::cout << "  ✓ Passed\n\n";
}

// Run all serialization tests - called from main.cpp
void run_serialization_tests()
{
    std::cout << "\n=== JSON-RPC 2.0 Serialization/Deserialization Tests ===\n\n";

    test_basic_types();
    test_string_types();
    test_custom_struct();
    test_complex_struct();
    test_void_return();
    test_no_params();
    test_vector_types();
    test_error_handling();
    test_invalid_params();
    test_endpoint_typed();
    test_endpoint_client_typed();
    test_mixed_json_and_typed();

    std::cout << "=== All serialization tests passed! ===\n\n";
}
