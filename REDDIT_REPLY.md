# Reddit Reply to u/jonathanhiggs

## For r/cpp Post

---

**Great suggestion! I've implemented it! 🎉**

You're absolutely right - working with C++ types directly is much more ergonomic. I've added full serialization/deserialization support to the library.

## What's New

The library now supports **typed handlers** that work directly with C++ types:

### Before (Raw JSON):
```cpp
disp.add("add", [](const json &params) -> json {
    int a = params["a"].get<int>();
    int b = params["b"].get<int>();
    return a + b;
});
```

### After (Typed):
```cpp
struct AddParams {
    int a, b;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AddParams, a, b)

disp.add_typed<AddParams, int>("add", [](AddParams params) {
    return params.a + params.b;
});
```

## Features

✅ **Type Safety** - Compile-time type checking  
✅ **Custom Types** - Works with structs, classes, STL containers  
✅ **Automatic Errors** - Invalid params detected automatically  
✅ **Zero Boilerplate** - No manual JSON conversion  
✅ **100% Compatible** - All existing code still works

## Examples

**Basic Types:**
```cpp
disp.add_typed<int, int>("double", [](int x) { return x * 2; });
```

**Custom Structs:**
```cpp
struct Point { double x, y; };
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Point, x, y)

disp.add_typed<Point, Point>("move", [](Point p) {
    p.x += 10;
    p.y += 20;
    return p;
});
```

**Vectors:**
```cpp
disp.add_typed<std::vector<int>, std::vector<int>>(
    "double_all", [](std::vector<int> nums) {
        for (auto &n : nums) n *= 2;
        return nums;
    }
);
```

## Client Side Too!

```cpp
endpoint ep(sender);

ep.send_request_typed<std::string, std::string>(
    "greet", 
    std::string("Alice"),
    [](std::string result) {
        std::cout << "Got: " << result << "\n";
    }
);
```

## Documentation

Full guide with examples: [`docs/SERIALIZATION.md`](https://github.com/pooriayousefi/jsonrpc2/blob/main/docs/SERIALIZATION.md)

## Tests

Added 12 comprehensive tests covering:
- Basic types (int, string)
- Custom structs
- STL containers
- Error handling
- Void returns
- Mixed typed/raw JSON handlers

All tests passing! ✅

## Implementation

- Uses C++23 templates (zero runtime overhead)
- JSON-RPC 2.0 compliant
- Works with any type nlohmann::json can serialize
- Backward compatible (no breaking changes)

Thanks for the suggestion! This makes the API much more pleasant to use. 🚀

---

**Commit:** [`19e7ad3`](https://github.com/pooriayousefi/jsonrpc2/commit/19e7ad3)  
**Repository:** https://github.com/pooriayousefi/jsonrpc2

---

## Shorter Version (if needed):

**Implemented! 🎉**

Added typed handler support as you suggested:

```cpp
// Now you can do this:
disp.add_typed<int, int>("double", [](int x) { return x * 2; });

// Instead of this:
disp.add("double", [](const json &params) -> json {
    return params.get<int>() * 2;
});
```

Works with custom types, structs, vectors, etc. Full docs in [`docs/SERIALIZATION.md`](https://github.com/pooriayousefi/jsonrpc2/blob/main/docs/SERIALIZATION.md).

12 new tests, 100% backward compatible. Thanks for the great suggestion!
