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
 * Test file to verify that forward_setter generated code compiles and works correctly.
 * This exercises the template setters with various argument types.
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>

// This test would be compiled with actual generated thrift code
// For now, it's a placeholder to demonstrate the pattern

int main() {
    std::cout << "Forward setter test - placeholder" << std::endl;
    // In a full build, this would:
    // 1. Include generated thrift types with forward_setter option
    // 2. Test setting fields with lvalues
    // 3. Test setting fields with rvalues (using std::move)
    // 4. Test setting fields with temporaries
    // 5. Verify __isset is set correctly for optional fields
    
    return 0;
}
