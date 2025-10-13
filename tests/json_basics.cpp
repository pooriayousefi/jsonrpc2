/*
 * JSON Basics Tutorial
 *
 * Interactive tutorial demonstrating JSON fundamentals using nlohmann/json.
 * Covers all basic operations with JSON data types.
 */

#include "../include/json.hpp"
#include <iostream>
#include <string>
#include <vector>

using json = nlohmann::json;

static void print_section(const std::string &title)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << title << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
}

void json_basic_types()
{
    print_section("1. JSON Basic Types");

    // Null
    std::cout << "\nNull:\n";
    json j_null = nullptr;
    std::cout << "  Value: " << j_null << "\n";
    std::cout << "  Type: " << j_null.type_name() << "\n";

    // Boolean
    std::cout << "\nBoolean:\n";
    json j_bool_true = true;
    json j_bool_false = false;
    std::cout << "  True:  " << j_bool_true << "\n";
    std::cout << "  False: " << j_bool_false << "\n";

    // Numbers
    std::cout << "\nNumbers:\n";
    json j_int = 42;
    json j_float = 3.14159;
    json j_negative = -100;
    std::cout << "  Integer: " << j_int << "\n";
    std::cout << "  Float:   " << j_float << "\n";
    std::cout << "  Negative: " << j_negative << "\n";

    // Strings
    std::cout << "\nStrings:\n";
    json j_string = "Hello, JSON!";
    std::cout << "  String: " << j_string << "\n";
    std::cout << "  Escaped: " << json("Line 1\\nLine 2") << "\n";
}

void json_arrays()
{
    print_section("2. JSON Arrays");

    // Creating arrays
    std::cout << "\nCreating Arrays:\n";
    json j_array1 = json::array({1, 2, 3, 4, 5});
    std::cout << "  Array literal: " << j_array1 << "\n";

    json j_array2 = json::array();
    j_array2.push_back("apple");
    j_array2.push_back("banana");
    j_array2.push_back("cherry");
    std::cout << "  Built array: " << j_array2 << "\n";

    // Mixed types
    json j_mixed = json::array({1, "two", 3.0, true, nullptr});
    std::cout << "  Mixed types: " << j_mixed << "\n";

    // Accessing elements
    std::cout << "\nAccessing Elements:\n";
    std::cout << "  j_array1[0] = " << j_array1[0] << "\n";
    std::cout << "  j_array1[2] = " << j_array1[2] << "\n";
    std::cout << "  j_array2[1] = " << j_array2[1] << "\n";

    // Array operations
    std::cout << "\nArray Operations:\n";
    std::cout << "  Size: " << j_array1.size() << "\n";
    std::cout << "  Is empty? " << (j_array1.empty() ? "yes" : "no") << "\n";

    // Iteration
    std::cout << "\nIterating:\n  ";
    for (const auto &elem : j_array2)
    {
        std::cout << elem << " ";
    }
    std::cout << "\n";
}

void json_objects()
{
    print_section("3. JSON Objects");

    // Creating objects
    std::cout << "\nCreating Objects:\n";
    json j_obj1 = {{"name", "John Doe"}, {"age", 30}, {"city", "New York"}};
    std::cout << "  Object literal:\n" << j_obj1.dump(2) << "\n";

    json j_obj2 = json::object();
    j_obj2["title"] = "Software Engineer";
    j_obj2["years_experience"] = 5;
    j_obj2["remote"] = true;
    std::cout << "\n  Built object:\n" << j_obj2.dump(2) << "\n";

    // Accessing fields
    std::cout << "\nAccessing Fields:\n";
    std::cout << "  Name: " << j_obj1["name"] << "\n";
    std::cout << "  Age: " << j_obj1["age"] << "\n";
    std::cout << "  City: " << j_obj1["city"] << "\n";

    // Checking for keys
    std::cout << "\nChecking Keys:\n";
    std::cout << "  Has 'name'? " << (j_obj1.contains("name") ? "yes" : "no") << "\n";
    std::cout << "  Has 'email'? " << (j_obj1.contains("email") ? "yes" : "no") << "\n";

    // Iterating
    std::cout << "\nIterating Over Fields:\n";
    for (auto &[key, value] : j_obj1.items())
    {
        std::cout << "  " << key << ": " << value << "\n";
    }
}

void json_nested_structures()
{
    print_section("4. Nested Structures");

    // Complex nested structure
    json person = {{"name", "Alice Johnson"},
                   {"age", 28},
                   {"address", {{"street", "123 Main St"}, {"city", "Boston"}, {"zip", "02101"}}},
                   {"phone_numbers", json::array({"555-1234", "555-5678"})},
                   {"skills", json::array({"C++", "Python", "JavaScript"})},
                   {"employed", true}};

    std::cout << "\nComplex Object:\n" << person.dump(2) << "\n";

    // Accessing nested data
    std::cout << "\nAccessing Nested Data:\n";
    std::cout << "  Name: " << person["name"] << "\n";
    std::cout << "  City: " << person["address"]["city"] << "\n";
    std::cout << "  First phone: " << person["phone_numbers"][0] << "\n";
    std::cout << "  Second skill: " << person["skills"][1] << "\n";

    // Modifying nested data
    person["address"]["zip"] = "02102";
    person["skills"].push_back("Rust");
    std::cout << "\nAfter Modifications:\n" << person.dump(2) << "\n";
}

void json_parsing_serialization()
{
    print_section("5. Parsing and Serialization");

    // Parse from string
    std::cout << "\nParsing from String:\n";
    std::string json_str = R"({"name":"Bob","age":35,"active":true})";
    std::cout << "  Input string: " << json_str << "\n";
    json parsed = json::parse(json_str);
    std::cout << "  Parsed object: " << parsed.dump(2) << "\n";

    // Serialize to string
    std::cout << "\nSerialization:\n";
    json data = {{"user", "charlie"}, {"level", 42}, {"premium", false}};
    std::cout << "  Compact: " << data.dump() << "\n";
    std::cout << "  Pretty (indent 2):\n" << data.dump(2) << "\n";
    std::cout << "  Pretty (indent 4):\n" << data.dump(4) << "\n";

    // Parse array
    std::cout << "\nParsing Arrays:\n";
    std::string array_str = R"([1, 2, 3, "four", 5.0])";
    json parsed_array = json::parse(array_str);
    std::cout << "  Parsed array: " << parsed_array << "\n";
}

void json_type_checking()
{
    print_section("6. Type Checking and Conversion");

    json values[] = {json(42),
                     json(3.14),
                     json("text"),
                     json(true),
                     json(nullptr),
                     json::array({1, 2, 3}),
                     json::object({{"key", "value"}})};

    std::cout << "\nType Checking:\n";
    for (size_t i = 0; i < 7; ++i)
    {
        const auto &j = values[i];
        std::cout << "  Value: " << j << "\n";
        std::cout << "    Type: " << j.type_name() << "\n";
        std::cout << "    is_null: " << j.is_null() << "\n";
        std::cout << "    is_boolean: " << j.is_boolean() << "\n";
        std::cout << "    is_number: " << j.is_number() << "\n";
        std::cout << "    is_string: " << j.is_string() << "\n";
        std::cout << "    is_array: " << j.is_array() << "\n";
        std::cout << "    is_object: " << j.is_object() << "\n\n";
    }

    // Type conversion
    std::cout << "Type Conversion:\n";
    json j_num = 42;
    int i = j_num.get<int>();
    double d = j_num.get<double>();
    std::string s = j_num.dump();
    std::cout << "  JSON: " << j_num << "\n";
    std::cout << "  As int: " << i << "\n";
    std::cout << "  As double: " << d << "\n";
    std::cout << "  As string: " << s << "\n";
}

void json_error_handling()
{
    print_section("7. Error Handling");

    // Invalid JSON
    std::cout << "\nParsing Invalid JSON:\n";
    try
    {
        std::string bad_json = "{invalid json}";
        json parsed = json::parse(bad_json);
        std::cout << "  Unexpectedly succeeded!\n";
    }
    catch (const json::parse_error &e)
    {
        std::cout << "  Caught parse error: " << e.what() << "\n";
    }

    // Type mismatch
    std::cout << "\nType Mismatch:\n";
    try
    {
        json j = "not a number";
        int value = j.get<int>(); // Will throw
        std::cout << "  Value: " << value << "\n";
    }
    catch (const json::type_error &e)
    {
        std::cout << "  Caught type error: " << e.what() << "\n";
    }

    // Safe access with defaults
    std::cout << "\nSafe Access with Defaults:\n";
    json obj = {{"name", "Alice"}, {"age", 28}};
    std::string name = obj.value("name", "Unknown");
    int age = obj.value("age", 0);
    std::string email = obj.value("email", "not@provided.com");
    std::cout << "  Name: " << name << "\n";
    std::cout << "  Age: " << age << "\n";
    std::cout << "  Email: " << email << " (default)\n";
}

void json_practical_examples()
{
    print_section("8. Practical Examples");

    // Configuration file
    std::cout << "\nConfiguration File:\n";
    json config = {
        {"server", {{"host", "localhost"}, {"port", 8080}, {"ssl", true}, {"timeout", 30}}},
        {"database",
         {{"host", "db.example.com"}, {"port", 5432}, {"name", "myapp"}, {"pool_size", 10}}},
        {"logging", {{"level", "info"}, {"file", "/var/log/app.log"}}}};
    std::cout << config.dump(2) << "\n";

    // API response
    std::cout << "\nAPI Response:\n";
    json api_response = {{"status", "success"},
                         {"data",
                          {{"users", json::array({{{"id", 1}, {"name", "Alice"}},
                                                  {{"id", 2}, {"name", "Bob"}},
                                                  {{"id", 3}, {"name", "Charlie"}}})},
                           {"total", 3},
                           {"page", 1}}},
                         {"timestamp", "2025-10-13T10:00:00Z"}};
    std::cout << api_response.dump(2) << "\n";

    // Building JSON programmatically
    std::cout << "\nBuilding JSON Programmatically:\n";
    json report = json::object();
    report["title"] = "Monthly Report";
    report["month"] = "October";
    report["year"] = 2025;
    report["metrics"] = json::object();
    report["metrics"]["users"] = 1250;
    report["metrics"]["revenue"] = 45000.50;
    report["metrics"]["growth"] = 12.5;
    std::cout << report.dump(2) << "\n";
}

// ============================================================================
// Main Entry Point
// ============================================================================

int run_json_basics()
{
    try
    {
        std::cout << "\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";
        std::cout << "  JSON Basics Tutorial\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";

        json_basic_types();
        json_arrays();
        json_objects();
        json_nested_structures();
        json_parsing_serialization();
        json_type_checking();
        json_error_handling();
        json_practical_examples();

        std::cout << "\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";
        std::cout << "  JSON Basics Tutorial Completed!\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in JSON basics tutorial: " << e.what() << std::endl;
        return 1;
    }
}
