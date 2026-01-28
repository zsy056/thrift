# Print OStream Template Option

## Overview

The `print_ostream` option for the C++ code generator enables generation of templated `printTo` methods and `operator<<` that can work with any output stream type, not just `std::ostream`.

## Usage

```bash
thrift --gen cpp:templates=print_ostream your_file.thrift
```

## What it does

When the `print_ostream` option is enabled:

1. **Templated `printTo` method**: Instead of:
   ```cpp
   void printTo(std::ostream& out) const;
   ```
   
   Generates:
   ```cpp
   template<typename OStream>
   void printTo(OStream& out) const;
   ```

2. **Templated `operator<<`**: Instead of:
   ```cpp
   std::ostream& operator<<(std::ostream& out, const MyStruct& obj);
   ```
   
   Generates:
   ```cpp
   template<typename OStream>
   OStream& operator<<(OStream& out, const MyStruct& obj);
   ```

3. **Implementation in .tcc file**: Both the templated `printTo` and `operator<<` implementations are placed in a `.tcc` file (e.g., `MyFile_types.tcc`) which is automatically included at the end of the header file.

## Benefits

### 1. Custom Stream Types
You can use custom output stream classes that are not derived from `std::ostream`:

```cpp
class FastOStringStream {
  std::string buffer_;
public:
  FastOStringStream& operator<<(const std::string& s) {
    buffer_ += s;
    return *this;
  }
  // ... other operators
  
  std::string str() const { return buffer_; }
};

// Works with generated code!
MyStruct s;
FastOStringStream stream;
stream << s;  // Uses templated operator<<
```

### 2. Performance
Custom stream implementations can avoid the virtual function overhead of `std::ostream`, potentially improving performance for serialization-heavy code.

### 3. Flexibility
Enables integration with custom logging frameworks, formatting libraries, or specialized output systems that don't derive from `std::ostream`.

## Example

Given this Thrift file:
```thrift
struct Person {
  1: i32 id,
  2: string name,
  3: optional string email
}
```

Generate with:
```bash
thrift --gen cpp:templates=print_ostream person.thrift
```

You can now use it with any stream type:
```cpp
#include "gen-cpp/person_types.h"
#include <iostream>
#include <sstream>

Person p;
p.id = 1;
p.name = "Alice";

// Works with std::ostream
std::cout << p << std::endl;

// Works with std::ostringstream
std::ostringstream oss;
oss << p;

// Works with custom streams
CustomStream custom;
custom << p;
```

## Testing

Functional tests are provided in `test/cpp/src/`:
- `PrintOstreamTest.cpp` - Tests basic functionality with both std::ostream and custom streams
- `PrintOstreamPerformanceTest.cpp` - Compares performance between std::ostringstream and custom stream

Validation script:
```bash
./test_print_ostream.sh
```

## Backward Compatibility

This option does not affect existing code generation options:
- Default `--gen cpp` still generates non-templated `printTo(std::ostream&)`
- `--gen cpp:templates` still generates templated read/write methods with non-templated printTo
- `--gen cpp:templates=only` still works as before

The `print_ostream` option only affects the `printTo` and `operator<<` generation.

## Requirements

- The custom stream type must provide `operator<<` overloads for:
  - `const std::string&`
  - `const char*`
  - Basic types (int32_t, int64_t, double, etc.)
  
- The stream type must return a reference to itself from `operator<<` to support chaining

## Implementation Details

- Templated implementations are written to `<name>_types.tcc`
- The `.tcc` file is automatically included at the end of `<name>_types.h`
- No changes to the `.cpp` implementation file
- Works alongside other template options (read/write templates)
