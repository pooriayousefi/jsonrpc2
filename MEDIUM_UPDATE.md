# Medium Article Update: Serialization Feature

## Option 1: Add as New Section (Recommended)

Add this section after "Real-World Example: Calculator Service" or before "Lessons Learned":

---

## Update: Community-Driven Feature - Type-Safe Handlers

**Published: October 16, 2025**

After publishing this article, I received valuable feedback from the C++ community on Reddit. User u/jonathanhiggs suggested:

> "It would be nice to make use of json serialisation / deserialization to have methods take and return data types rather than json objects"

**I completely agreed.** Working with C++ types directly is much more ergonomic than manual JSON conversion. So I implemented it!

### The New API

The library now supports **typed handlers** that work directly with C++ types:

#### Before (Original):
```cpp
dispatcher disp;
disp.add("add", [](const json &params) -> json {
    int a = params["a"].get<int>();
    int b = params["b"].get<int>();
    return a + b;
});
```

#### After (With Types):
```cpp
struct AddParams {
    int a, b;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AddParams, a, b)

dispatcher disp;
disp.add_typed<AddParams, int>("add", [](AddParams params) {
    return params.a + params.b;  // So much cleaner!
});
```

### What's Supported

**✅ Basic Types**
```cpp
disp.add_typed<int, int>("double", [](int x) { return x * 2; });
disp.add_typed<std::string, std::string>("greet", 
    [](std::string name) { return "Hello, " + name + "!"; }
);
```

**✅ Custom Structs**
```cpp
struct Point { double x, y; };
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Point, x, y)

disp.add_typed<Point, Point>("move", [](Point p) {
    p.x += 10;
    p.y += 20;
    return p;
});
```

**✅ STL Containers**
```cpp
disp.add_typed<std::vector<int>, std::vector<int>>(
    "double_all", [](std::vector<int> nums) {
        for (auto &n : nums) n *= 2;
        return nums;
    }
);
```

**✅ Void Returns**
```cpp
int counter = 0;
disp.add_typed<int, void>("increment", [&](int x) {
    counter += x;
});
```

**✅ No Parameters**
```cpp
disp.add_no_params<int>("get_random", []() {
    return rand() % 100;
});
```

### Client-Side Too!

The endpoint class also gained typed methods:

```cpp
endpoint ep(sender);

ep.send_request_typed<std::string, std::string>(
    "greet",
    std::string("Alice"),
    [](std::string result) {
        std::cout << "Server said: " << result << "\n";
    }
);
```

### Benefits

1. **Type Safety**: Compile-time checking catches errors early
2. **Less Boilerplate**: No more manual `.get<T>()` calls
3. **Better Errors**: Invalid params detected automatically with `-32602` error
4. **Flexibility**: Works with any type nlohmann::json can serialize
5. **Backward Compatible**: All existing raw JSON handlers still work

### Implementation Details

The implementation uses C++23 templates that resolve at compile time—**zero runtime overhead**. The library automatically:

- Wraps single parameters in JSON arrays (for JSON-RPC 2.0 compliance)
- Deserializes incoming JSON to your C++ types
- Serializes return values back to JSON
- Detects type conversion errors and returns proper error responses

### Testing

I added 12 comprehensive tests covering:
- Basic types (int, string)
- Custom structs with complex fields
- STL containers (vector, map)
- Error handling and validation
- Void return types
- Mixed typed/raw JSON handlers

**All tests passing!** ✅

### The Power of Community Feedback

This is a perfect example of why open source and community engagement matters. A simple suggestion led to a feature that makes the entire library more pleasant to use.

The implementation took about 2 hours, added ~750 lines of code (including tests and documentation), and maintains 100% backward compatibility.

**Full Documentation**: [docs/SERIALIZATION.md](https://github.com/pooriayousefi/jsonrpc2/blob/main/docs/SERIALIZATION.md)

---

## Option 2: Short Update Block

Add this at the top of your article or as a highlighted section:

---

### 🆕 UPDATE (October 16, 2025): Type-Safe Handlers

Following community feedback, the library now supports **typed handlers**! Work with C++ types directly instead of JSON:

```cpp
// New: Type-safe
disp.add_typed<int, int>("double", [](int x) { return x * 2; });

// Old: Still works!
disp.add("double", [](const json &p) -> json { return p.get<int>() * 2; });
```

**Benefits**: Type safety, less boilerplate, automatic error handling. Works with custom types, structs, vectors, etc.

📖 [Full Serialization Guide](https://github.com/pooriayousefi/jsonrpc2/blob/main/docs/SERIALIZATION.md)

---

## Option 3: Separate Follow-Up Article

Create a new article titled: **"Building on Feedback: Adding Type-Safe Handlers to a JSON-RPC Library"**

### Outline:
1. **Introduction**: Community feedback as a driver for improvement
2. **The Request**: What u/jonathanhiggs suggested
3. **The Problem**: Manual JSON conversion is tedious
4. **The Solution**: Template-based type system
5. **Implementation**: How it works under the hood
6. **Examples**: Before/after comparisons
7. **Testing**: Comprehensive test suite
8. **Results**: Improved developer experience
9. **Lessons**: Rapid iteration, backward compatibility
10. **Conclusion**: The value of community engagement

This would be a great follow-up article focusing on:
- **Iterative development** based on user feedback
- **Template metaprogramming** in C++23
- **API design** for usability
- **Community-driven open source**

---

## Recommended Approach

**I recommend Option 1** (add as new section) because:

1. ✅ Keeps readers on the same article
2. ✅ Shows iterative improvement
3. ✅ Demonstrates responsive development
4. ✅ Adds value without requiring a separate article
5. ✅ Medium allows article updates

**Where to insert**: After the "Real-World Example: Calculator Service" section and before "Lessons Learned". This way readers see:
1. How the library works (original content)
2. How it improved based on feedback (new section)
3. Lessons learned (includes this new iteration)

---

## Quick Stats for Article

- **Implementation Time**: ~2 hours
- **Lines Added**: ~750 (code + tests + docs)
- **Tests Added**: 12 (100% passing)
- **Breaking Changes**: 0 (100% backward compatible)
- **GitHub Stars**: Check current count
- **Reddit Engagement**: 5K+ views on r/cpp

---

## Social Media Update Posts

### Twitter/X:
```
🎉 Big update to my JSON-RPC 2.0 C++ library!

Added type-safe handlers based on community feedback. Now you can work with C++ types directly instead of JSON.

Before: json → manual .get<>() → use
After: Direct C++ types! ✨

📖 https://github.com/pooriayousefi/jsonrpc2

#cpp #cpp23 #opensource
```

### LinkedIn:
```
Excited to share a major update to my JSON-RPC 2.0 C++ library!

Following feedback from the C++ community on Reddit, I've added type-safe handler support. Developers can now register methods that work directly with C++ types instead of manually converting to/from JSON.

This is a great example of how community feedback drives better software design. A simple suggestion turned into a feature that makes the entire library more pleasant to use.

Key improvements:
✅ Compile-time type safety
✅ Zero boilerplate for type conversion
✅ Automatic error handling
✅ Works with custom types, structs, and STL containers
✅ 100% backward compatible

Implementation: 2 hours, 750 lines, 12 tests, zero breaking changes.

Read more: https://github.com/pooriayousefi/jsonrpc2

#CPlusPlus #SoftwareDevelopment #OpenSource #CommunityDriven
```

---

Would you like me to help with any of these options?
