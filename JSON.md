# JSON Tutorial

## Table of Contents

1. [Introduction](#introduction)
2. [What is JSON?](#what-is-json)
3. [JSON Syntax](#json-syntax)
4. [Data Types](#data-types)
5. [JSON Structure](#json-structure)
6. [Working with JSON in C++](#working-with-json-in-c)
7. [Common Use Cases](#common-use-cases)
8. [Best Practices](#best-practices)
9. [Resources](#resources)

---

## Introduction

**JSON** (JavaScript Object Notation) is a lightweight, text-based data interchange format. It's easy for humans to read and write, and easy for machines to parse and generate. Despite its name, JSON is language-independent and can be used with virtually any programming language.

### Key Features

- **Lightweight**: Minimal syntax overhead
- **Human-readable**: Easy to understand at a glance
- **Language-independent**: Supported by virtually all modern programming languages
- **Self-describing**: Structure is inherent in the data
- **Widely adopted**: The de facto standard for web APIs

---

## What is JSON?

JSON was derived from JavaScript but has become a universal data format. It's primarily used for:

- **Web APIs**: Exchanging data between client and server
- **Configuration files**: Application settings and preferences
- **Data storage**: NoSQL databases like MongoDB
- **Message passing**: Communication between services
- **Serialization**: Converting objects to transmittable format

### JSON vs. XML

| Feature | JSON | XML |
|---------|------|-----|
| Readability | Very high | Moderate |
| Verbosity | Minimal | High |
| Data types | Native support | Everything is text |
| Arrays | Native support | Must be emulated |
| Parsing speed | Fast | Slower |
| File size | Smaller | Larger |

---

## JSON Syntax

JSON has a simple and strict syntax:

### Basic Rules

1. **Data is in name/value pairs**: `"name": "value"`
2. **Data is separated by commas**: `"name1": "value1", "name2": "value2"`
3. **Curly braces hold objects**: `{ "name": "value" }`
4. **Square brackets hold arrays**: `[ "value1", "value2" ]`
5. **Strings use double quotes**: `"text"` (single quotes are NOT valid)
6. **No trailing commas**: Last item must not have a comma
7. **No comments**: JSON does not support comments

### Valid JSON Example

```json
{
  "name": "John Doe",
  "age": 30,
  "city": "New York",
  "hobbies": ["reading", "gaming", "coding"],
  "employed": true
}
```

### Invalid JSON Examples

```json
// This is invalid (comments not allowed)
{
  'name': 'John',        // Single quotes not allowed
  "age": 30,
  "city": "New York",    // Trailing comma not allowed
}
```

---

## Data Types

JSON supports six data types:

### 1. String

Text enclosed in double quotes.

```json
{
  "name": "Alice",
  "message": "Hello, World!",
  "escaped": "Line 1\nLine 2",
  "unicode": "Emoji: \u1F600"
}
```

**Escape sequences**:

- `\"` - Double quote
- `\\` - Backslash
- `\/` - Forward slash
- `\b` - Backspace
- `\f` - Form feed
- `\n` - Newline
- `\r` - Carriage return
- `\t` - Tab
- `\uXXXX` - Unicode character

### 2. Number

Integer or floating-point, positive or negative.

```json
{
  "integer": 42,
  "negative": -17,
  "float": 3.14159,
  "scientific": 1.23e-4,
  "zero": 0
}
```

**Note**: JSON doesn't distinguish between integers and floats; that's left to the parser.

### 3. Boolean

True or false (lowercase only).

```json
{
  "isActive": true,
  "isDeleted": false
}
```

### 4. Null

Represents an empty or non-existent value.

```json
{
  "middleName": null,
  "comment": null
}
```

### 5. Object

Collection of key-value pairs enclosed in curly braces.

```json
{
  "person": {
    "firstName": "John",
    "lastName": "Doe",
    "age": 30
  }
}
```

**Rules**:

- Keys must be strings
- Values can be any JSON type
- Keys must be unique within an object

### 6. Array

Ordered list of values enclosed in square brackets.

```json
{
  "numbers": [1, 2, 3, 4, 5],
  "mixed": [1, "two", true, null, {"key": "value"}],
  "nested": [[1, 2], [3, 4], [5, 6]]
}
```

**Rules**:

- Values can be any JSON type
- Values can be mixed types
- Arrays can be nested

---

## JSON Structure

### Simple Object

```json
{
  "id": 1,
  "username": "johndoe",
  "email": "john@example.com"
}
```

### Nested Objects

```json
{
  "user": {
    "id": 1,
    "profile": {
      "firstName": "John",
      "lastName": "Doe",
      "address": {
        "street": "123 Main St",
        "city": "Boston",
        "zip": "02101"
      }
    }
  }
}
```

### Arrays of Objects

```json
{
  "users": [
    {
      "id": 1,
      "name": "Alice"
    },
    {
      "id": 2,
      "name": "Bob"
    },
    {
      "id": 3,
      "name": "Charlie"
    }
  ]
}
```

### Complex Structure

```json
{
  "company": "Tech Corp",
  "founded": 2010,
  "departments": [
    {
      "name": "Engineering",
      "employees": [
        {
          "id": 1,
          "name": "Alice",
          "role": "Senior Engineer",
          "skills": ["C++", "Python", "Go"],
          "remote": true
        },
        {
          "id": 2,
          "name": "Bob",
          "role": "Tech Lead",
          "skills": ["JavaScript", "React", "Node.js"],
          "remote": false
        }
      ]
    },
    {
      "name": "Marketing",
      "employees": [
        {
          "id": 3,
          "name": "Carol",
          "role": "Marketing Manager",
          "skills": ["SEO", "Content Marketing"],
          "remote": true
        }
      ]
    }
  ],
  "metadata": {
    "lastUpdated": "2025-10-13T10:00:00Z",
    "version": "1.0"
  }
}
```

---

## Working with JSON in C++

This project uses [nlohmann/json](https://github.com/nlohmann/json) library, which provides an intuitive API for working with JSON.

### Including the Library

```cpp
#include "json.hpp"
using json = nlohmann::json;
```

### Creating JSON Objects

```cpp
// From literals
json j1 = {
  {"name", "Alice"},
  {"age", 28},
  {"city", "Boston"}
};

// Programmatically
json j2 = json::object();
j2["name"] = "Bob";
j2["age"] = 35;
j2["active"] = true;

// From string
std::string json_str = R"({"name":"Charlie","age":42})";
json j3 = json::parse(json_str);
```

### Creating Arrays

```cpp
// From literals
json arr1 = json::array({1, 2, 3, 4, 5});

// Programmatically
json arr2 = json::array();
arr2.push_back("apple");
arr2.push_back("banana");
arr2.push_back("cherry");

// Mixed types
json mixed = json::array({1, "two", 3.0, true, nullptr});
```

### Accessing Data

```cpp
json data = {
  {"name", "Alice"},
  {"age", 28},
  {"skills", {"C++", "Python", "Rust"}}
};

// Access values
std::string name = data["name"];
int age = data["age"];
std::string first_skill = data["skills"][0];

// Safe access with defaults
int level = data.value("level", 1);  // Returns 1 if "level" doesn't exist
```

### Type Checking

```cpp
json value = 42;

if (value.is_number()) {
  std::cout << "It's a number: " << value.get<int>() << "\n";
}

// Other checks
value.is_null();
value.is_boolean();
value.is_string();
value.is_array();
value.is_object();
```

### Serialization

```cpp
json data = {
  {"user", "alice"},
  {"level", 42}
};

// Compact
std::string compact = data.dump();
// {"user":"alice","level":42}

// Pretty-print with indentation
std::string pretty = data.dump(2);
// {
//   "user": "alice",
//   "level": 42
// }
```

### Iteration

```cpp
// Iterate over array
json arr = json::array({"a", "b", "c"});
for (const auto& elem : arr) {
  std::cout << elem << "\n";
}

// Iterate over object
json obj = {{"a", 1}, {"b", 2}, {"c", 3}};
for (auto& [key, value] : obj.items()) {
  std::cout << key << ": " << value << "\n";
}
```

### Error Handling

```cpp
try {
  // Parse invalid JSON
  json j = json::parse("{invalid json}");
} catch (const json::parse_error& e) {
  std::cerr << "Parse error: " << e.what() << "\n";
}

try {
  // Type mismatch
  json j = "not a number";
  int value = j.get<int>();
} catch (const json::type_error& e) {
  std::cerr << "Type error: " << e.what() << "\n";
}
```

---

## Common Use Cases

### 1. Configuration Files

```json
{
  "database": {
    "host": "localhost",
    "port": 5432,
    "username": "admin",
    "password": "secret",
    "database": "myapp"
  },
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "ssl": true
  },
  "logging": {
    "level": "info",
    "file": "/var/log/app.log"
  }
}
```

### 2. API Responses

```json
{
  "status": "success",
  "data": {
    "user": {
      "id": 123,
      "username": "johndoe",
      "email": "john@example.com"
    },
    "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
  },
  "timestamp": "2025-10-13T10:00:00Z"
}
```

### 3. Data Exchange

```json
{
  "transaction": {
    "id": "TX123456",
    "type": "purchase",
    "amount": 49.99,
    "currency": "USD",
    "items": [
      {"id": "ITEM001", "quantity": 2, "price": 19.99},
      {"id": "ITEM002", "quantity": 1, "price": 9.99}
    ],
    "customer": {
      "id": "CUST789",
      "name": "Jane Smith"
    }
  }
}
```

---

## Best Practices

### 1. Use Consistent Naming Conventions

Choose one and stick with it:

- **camelCase**: `firstName`, `lastName`, `emailAddress`
- **snake_case**: `first_name`, `last_name`, `email_address`
- **PascalCase**: `FirstName`, `LastName`, `EmailAddress`

### 2. Keep Structure Flat When Possible

**Bad** (overly nested):

```json
{
  "data": {
    "user": {
      "info": {
        "name": "John"
      }
    }
  }
}
```

**Good** (flatter):

```json
{
  "user": {
    "name": "John"
  }
}
```

### 3. Use Arrays for Lists

**Bad**:

```json
{
  "item1": "apple",
  "item2": "banana",
  "item3": "cherry"
}
```

**Good**:

```json
{
  "items": ["apple", "banana", "cherry"]
}
```

### 4. Include Metadata

```json
{
  "data": [...],
  "metadata": {
    "timestamp": "2025-10-13T10:00:00Z",
    "version": "1.0",
    "count": 100,
    "page": 1
  }
}
```

### 5. Use Null for Missing Values

**Good**:

```json
{
  "name": "John",
  "middleName": null,
  "lastName": "Doe"
}
```

**Also good** (omit optional fields):

```json
{
  "name": "John",
  "lastName": "Doe"
}
```

### 6. Validate JSON

Always validate JSON before processing:

- Check structure matches expected schema
- Verify required fields exist
- Validate data types
- Check for reasonable value ranges

### 7. Handle Errors Gracefully

```cpp
try {
  json data = json::parse(input);
  // Process data
} catch (const json::parse_error& e) {
  // Handle parse error
} catch (const json::type_error& e) {
  // Handle type error
} catch (const json::out_of_range& e) {
  // Handle missing keys
}
```

### 8. Use Pretty-Printing for Debugging

```cpp
// Development/debugging
std::cout << data.dump(2) << "\n";

// Production
std::string output = data.dump();  // Compact
```

### 9. Consider File Size

For large datasets:

- Use compact format (no indentation)
- Consider compression (gzip)
- Stream large files rather than loading entirely

### 10. Document Your JSON Schema

Use tools like JSON Schema to document expected structure:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "name": {"type": "string"},
    "age": {"type": "integer", "minimum": 0}
  },
  "required": ["name"]
}
```

---

## Resources

### Official

- [JSON.org](https://www.json.org/) - Official JSON website
- [ECMA-404](https://www.ecma-international.org/publications-and-standards/standards/ecma-404/) - JSON Data Interchange Syntax
- [RFC 8259](https://tools.ietf.org/html/rfc8259) - The JavaScript Object Notation (JSON) Data Interchange Format

### Libraries

- [nlohmann/json](https://github.com/nlohmann/json) - JSON for Modern C++
- [RapidJSON](https://rapidjson.org/) - Fast JSON parser/generator
- [simdjson](https://github.com/simdjson/simdjson) - Parsing gigabytes of JSON per second

### Tools

- [JSONLint](https://jsonlint.com/) - JSON validator
- [JSON Schema](https://json-schema.org/) - Vocabulary for annotating and validating JSON
- [jq](https://stedolan.github.io/jq/) - Command-line JSON processor

### Tutorials

- [MDN Web Docs - JSON](https://developer.mozilla.org/en-US/docs/Learn/JavaScript/Objects/JSON)
- [W3Schools JSON Tutorial](https://www.w3schools.com/js/js_json_intro.asp)

---

## Conclusion

JSON is a simple yet powerful data format that has become ubiquitous in modern software development. Understanding JSON is essential for:

- Building web APIs
- Configuring applications
- Exchanging data between systems
- Working with NoSQL databases
- Implementing communication protocols like JSON-RPC

The simplicity of JSON makes it easy to learn, while its flexibility makes it suitable for a wide range of applications.

---

**Next Steps**:

- Read the [JSON-RPC 2.0 Tutorial](JSONRPC2.md) to learn how JSON is used in remote procedure calls
- Explore the examples in the `tests/` directory
- Run the tutorial programs to see JSON in action
