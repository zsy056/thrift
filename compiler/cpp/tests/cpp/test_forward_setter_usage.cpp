// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements. See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership. The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the License for the
// specific language governing permissions and limitations
// under the License.

#include "test_forward_setter_types.h"
#include <string>
#include <vector>
#include <map>
#include <utility>

// This file tests that the generated forward_setter code compiles and works correctly
// It exercises the template setters with various argument types

int main() {
    test::forward_setter::TestForwardSetter obj;
    
    // Test with lvalue references
    std::string str1 = "hello";
    obj.__set_complex_string(str1);
    
    // Test with rvalue references (move semantics)
    obj.__set_complex_string(std::string("world"));
    obj.__set_complex_string("literal string");
    
    // Test with complex struct - lvalue
    test::forward_setter::InnerStruct inner1;
    inner1.__set_name("inner");
    inner1.__set_value(42);
    obj.__set_complex_struct(inner1);
    
    // Test with complex struct - rvalue (move)
    test::forward_setter::InnerStruct inner2;
    inner2.__set_name("moved");
    obj.__set_complex_struct(std::move(inner2));
    
    // Test with list - lvalue
    std::vector<std::string> list1;
    list1.push_back("item1");
    list1.push_back("item2");
    obj.__set_complex_list(list1);
    
    // Test with list - rvalue (move)
    std::vector<std::string> list2;
    list2.push_back("item3");
    obj.__set_complex_list(std::move(list2));
    
    // Test with map - lvalue
    std::map<std::string, int32_t> map1;
    map1["key1"] = 1;
    map1["key2"] = 2;
    obj.__set_complex_map(map1);
    
    // Test with map - rvalue (move)
    std::map<std::string, int32_t> map2;
    map2["key3"] = 3;
    obj.__set_complex_map(std::move(map2));
    
    // Test primitive setters (should work as before)
    obj.__set_primitive_field(123);
    obj.__set_primitive_bool(true);
    
    // Verify the __isset is set correctly for optional fields
    if (!obj.__isset.complex_string || !obj.__isset.complex_struct ||
        !obj.__isset.complex_list || !obj.__isset.complex_map) {
        return 1; // Error: optional fields not marked as set
    }
    
    return 0; // Success
}
