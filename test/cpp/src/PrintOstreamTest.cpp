/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <iostream>
#include <sstream>
#include <cassert>
#include "gen-cpp/PrintOstreamTest_types.h"

using namespace test::print_ostream;

// Custom ostream-like class that is NOT derived from std::ostream
// This demonstrates the templated printTo can work with custom stream types
class CustomOStringStream {
private:
  std::string buffer_;

public:
  CustomOStringStream() = default;

  // Minimal interface needed for printTo to work
  CustomOStringStream& operator<<(const std::string& s) {
    buffer_ += s;
    return *this;
  }

  CustomOStringStream& operator<<(const char* s) {
    buffer_ += s;
    return *this;
  }

  CustomOStringStream& operator<<(char c) {
    buffer_ += c;
    return *this;
  }

  CustomOStringStream& operator<<(int32_t val) {
    buffer_ += std::to_string(val);
    return *this;
  }

  CustomOStringStream& operator<<(int64_t val) {
    buffer_ += std::to_string(val);
    return *this;
  }

  CustomOStringStream& operator<<(double val) {
    buffer_ += std::to_string(val);
    return *this;
  }

  std::string str() const {
    return buffer_;
  }

  void clear() {
    buffer_.clear();
  }
};

int main() {
  std::cout << "Testing templated printTo and operator<< with print_ostream option\n\n";

  // Test 1: Basic struct with std::ostream
  {
    std::cout << "Test 1: SimpleStruct with std::ostream\n";
    SimpleStruct s;
    s.__set_id(42);
    s.__set_name("test_name");
    s.__set_description("test description");

    std::ostringstream oss;
    oss << s;
    std::string result = oss.str();
    std::cout << "Output: " << result << "\n";
    assert(result.find("SimpleStruct") != std::string::npos);
    assert(result.find("42") != std::string::npos);
    assert(result.find("test_name") != std::string::npos);
    assert(result.find("test description") != std::string::npos);
    std::cout << "PASSED\n\n";
  }

  // Test 2: SimpleStruct with custom ostream
  {
    std::cout << "Test 2: SimpleStruct with CustomOStringStream\n";
    SimpleStruct s;
    s.__set_id(123);
    s.__set_name("custom");

    CustomOStringStream cos;
    cos << s;
    std::string result = cos.str();
    std::cout << "Output: " << result << "\n";
    assert(result.find("SimpleStruct") != std::string::npos);
    assert(result.find("123") != std::string::npos);
    assert(result.find("custom") != std::string::npos);
    std::cout << "PASSED\n\n";
  }

  // Test 3: Optional field handling
  {
    std::cout << "Test 3: Optional field (not set) with std::ostream\n";
    SimpleStruct s;
    s.__set_id(99);
    s.__set_name("no_description");
    // description is not set

    std::ostringstream oss;
    oss << s;
    std::string result = oss.str();
    std::cout << "Output: " << result << "\n";
    assert(result.find("<null>") != std::string::npos);
    std::cout << "PASSED\n\n";
  }

  // Test 4: Nested struct with std::ostream
  {
    std::cout << "Test 4: NestedStruct with std::ostream\n";
    SimpleStruct simple;
    simple.__set_id(1);
    simple.__set_name("nested");

    NestedStruct nested;
    nested.__set_simple(simple);
    nested.__set_tags(std::vector<std::string>{"tag1", "tag2", "tag3"});
    nested.__set_scores({{"score1", 100}, {"score2", 200}});

    std::ostringstream oss;
    oss << nested;
    std::string result = oss.str();
    std::cout << "Output: " << result << "\n";
    assert(result.find("NestedStruct") != std::string::npos);
    assert(result.find("SimpleStruct") != std::string::npos);
    std::cout << "PASSED\n\n";
  }

  // Test 5: Nested struct with custom ostream
  {
    std::cout << "Test 5: NestedStruct with CustomOStringStream\n";
    SimpleStruct simple;
    simple.__set_id(2);
    simple.__set_name("custom_nested");

    NestedStruct nested;
    nested.__set_simple(simple);
    nested.__set_tags(std::vector<std::string>{"a", "b"});

    CustomOStringStream cos;
    cos << nested;
    std::string result = cos.str();
    std::cout << "Output: " << result << "\n";
    assert(result.find("NestedStruct") != std::string::npos);
    assert(result.find("SimpleStruct") != std::string::npos);
    assert(result.find("custom_nested") != std::string::npos);
    std::cout << "PASSED\n\n";
  }

  std::cout << "All tests passed!\n";
  return 0;
}
