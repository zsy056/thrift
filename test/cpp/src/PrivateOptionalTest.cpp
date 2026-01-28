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

/**
 * Test file to verify that private_optional generated code compiles and works correctly.
 * This exercises the private_optional option using ThriftTest types.
 */

#include <iostream>
#include <string>
#include <cassert>

// Include generated thrift types with private_optional option
#include "ThriftTest_types.h"

using namespace thrift::test;

int main() {
    std::cout << "Testing private_optional with ThriftTest types..." << std::endl;
    
    // Test 1: Verify getters work for accessing private optional fields
    {
        Xtruct x;
        x.__set_string_thing("test");
        const std::string& str = x.__get_string_thing();
        assert(str == "test");
        std::cout << "  ✓ Getter for string field works" << std::endl;
    }
    
    // Test 2: Verify setters work
    {
        Xtruct x;
        x.__set_i32_thing(42);
        x.__set_i64_thing(1234567890);
        assert(x.__get_i32_thing() == 42);
        assert(x.__get_i64_thing() == 1234567890);
        std::cout << "  ✓ Setters for primitive fields work" << std::endl;
    }
    
    // Test 3: Verify getters/setters for complex types
    {
        Xtruct2 x2;
        Xtruct x;
        x.__set_string_thing("nested");
        x.__set_i32_thing(99);
        x2.__set_struct_thing(x);
        assert(x2.__get_struct_thing().string_thing == "nested");
        assert(x2.__get_struct_thing().i32_thing == 99);
        std::cout << "  ✓ Getters/setters for struct fields work" << std::endl;
    }
    
    std::cout << "\n✅ All private_optional tests passed!" << std::endl;
    return 0;
}
