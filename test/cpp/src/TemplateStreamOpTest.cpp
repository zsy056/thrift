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
 * Test file to verify that template_streamop generated code compiles and works correctly.
 * This tests the templated operator<< and printTo with various stream types.
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <chrono>

// Include generated thrift types with template_streamop option
#include "ThriftTest_types.h"

using namespace thrift::test;

// Custom minimal stream implementation for testing and performance comparison
class MinimalStream {
private:
    std::string buffer_;
public:
    MinimalStream& operator<<(const std::string& s) {
        buffer_ += s;
        return *this;
    }
    
    MinimalStream& operator<<(const char* s) {
        buffer_ += s;
        return *this;
    }
    
    MinimalStream& operator<<(char c) {
        buffer_ += c;
        return *this;
    }
    
    MinimalStream& operator<<(int32_t i) {
        buffer_ += std::to_string(i);
        return *this;
    }
    
    MinimalStream& operator<<(int64_t i) {
        buffer_ += std::to_string(i);
        return *this;
    }
    
    MinimalStream& operator<<(uint32_t i) {
        buffer_ += std::to_string(i);
        return *this;
    }
    
    MinimalStream& operator<<(uint64_t i) {
        buffer_ += std::to_string(i);
        return *this;
    }
    
    MinimalStream& operator<<(double d) {
        buffer_ += std::to_string(d);
        return *this;
    }
    
    MinimalStream& operator<<(bool b) {
        buffer_ += (b ? "true" : "false");
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
    std::cout << "Testing template_streamop with ThriftTest types..." << std::endl;
    
    // Test 1: Test with std::ostringstream
    {
        Xtruct x;
        x.__set_string_thing("test string");
        x.__set_byte_thing(42);
        x.__set_i32_thing(12345);
        x.__set_i64_thing(9876543210LL);
        
        std::ostringstream oss;
        oss << x;
        std::string result = oss.str();
        
        std::cout << "  Generated output: " << result << std::endl;
        
        assert(!result.empty());
        assert(result.find("test string") != std::string::npos);
        assert(result.find("42") != std::string::npos);
        assert(result.find("12345") != std::string::npos);
        std::cout << "  ✓ std::ostringstream works: " << result << std::endl;
    }
    
    // Test 2: Test with custom MinimalStream
    {
        Xtruct x;
        x.__set_string_thing("custom stream");
        x.__set_byte_thing(7);
        x.__set_i32_thing(999);
        x.__set_i64_thing(1234567890LL);
        
        MinimalStream ms;
        ms << x;
        std::string result = ms.str();
        
        assert(!result.empty());
        assert(result.find("custom stream") != std::string::npos);
        assert(result.find("7") != std::string::npos);
        assert(result.find("999") != std::string::npos);
        std::cout << "  ✓ MinimalStream works: " << result << std::endl;
    }
    
    // Test 3: Test nested structures
    {
        Xtruct x;
        x.__set_string_thing("inner");
        x.__set_i32_thing(100);
        
        Xtruct2 x2;
        x2.__set_byte_thing(5);
        x2.__set_struct_thing(x);
        x2.__set_i32_thing(200);
        
        std::ostringstream oss;
        oss << x2;
        std::string result = oss.str();
        
        assert(!result.empty());
        assert(result.find("inner") != std::string::npos);
        assert(result.find("100") != std::string::npos);
        assert(result.find("200") != std::string::npos);
        std::cout << "  ✓ Nested structures work" << std::endl;
    }
    
    // Test 4: Test optional fields
    {
        Bonk bonk;
        bonk.__set_message("test message");
        bonk.__set_type(42);
        
        std::ostringstream oss;
        oss << bonk;
        std::string result = oss.str();
        
        assert(!result.empty());
        assert(result.find("test message") != std::string::npos);
        assert(result.find("42") != std::string::npos);
        std::cout << "  ✓ Optional fields work" << std::endl;
    }
    
    // Performance Test: Compare std::ostringstream vs MinimalStream
    {
        const int iterations = 10000;
        Xtruct x;
        x.__set_string_thing("performance test string");
        x.__set_byte_thing(123);
        x.__set_i32_thing(456789);
        x.__set_i64_thing(9876543210LL);
        
        // Test std::ostringstream performance
        auto start_oss = std::chrono::high_resolution_clock::now();
        std::string accumulated_result;  // Prevent optimization by accumulating results
        for (int i = 0; i < iterations; ++i) {
            std::ostringstream oss;
            oss << x;
            accumulated_result += oss.str();  // Use result to prevent optimization
        }
        auto end_oss = std::chrono::high_resolution_clock::now();
        auto duration_oss = std::chrono::duration_cast<std::chrono::microseconds>(end_oss - start_oss).count();
        
        // Test MinimalStream performance
        auto start_ms = std::chrono::high_resolution_clock::now();
        accumulated_result.clear();  // Reuse for MinimalStream test
        for (int i = 0; i < iterations; ++i) {
            MinimalStream ms;
            ms << x;
            accumulated_result += ms.str();  // Use result to prevent optimization
        }
        auto end_ms = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::microseconds>(end_ms - start_ms).count();
        
        std::cout << "\n  Performance comparison (" << iterations << " iterations):" << std::endl;
        std::cout << "    std::ostringstream: " << duration_oss << " μs" << std::endl;
        std::cout << "    MinimalStream:      " << duration_ms << " μs" << std::endl;
        
        if (duration_ms < duration_oss) {
            double improvement = ((double)(duration_oss - duration_ms) / duration_oss) * 100.0;
            std::cout << "    MinimalStream is " << improvement << "% faster" << std::endl;
        } else {
            double difference = ((double)(duration_ms - duration_oss) / duration_oss) * 100.0;
            std::cout << "    std::ostringstream is " << difference << "% faster" << std::endl;
        }
        
        std::cout << "  ✓ Performance test completed" << std::endl;
    }
    
    std::cout << "\n✅ All template_streamop tests passed!" << std::endl;
    return 0;
}
