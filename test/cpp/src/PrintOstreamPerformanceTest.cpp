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
#include <chrono>
#include <vector>
#include "gen-cpp/PrintOstreamTest_types.h"

using namespace test::print_ostream;

// Custom ostream implementation with minimal overhead
// This is NOT derived from std::ostream, avoiding virtual function overhead
class FastOStringStream {
private:
  std::string buffer_;

public:
  FastOStringStream() {
    buffer_.reserve(1024);  // Pre-allocate to reduce reallocations
  }

  // Minimal interface for printTo
  FastOStringStream& operator<<(const std::string& s) {
    buffer_ += s;
    return *this;
  }

  FastOStringStream& operator<<(const char* s) {
    buffer_ += s;
    return *this;
  }

  FastOStringStream& operator<<(char c) {
    buffer_ += c;
    return *this;
  }

  FastOStringStream& operator<<(int32_t val) {
    buffer_ += std::to_string(val);
    return *this;
  }

  FastOStringStream& operator<<(int64_t val) {
    buffer_ += std::to_string(val);
    return *this;
  }

  FastOStringStream& operator<<(double val) {
    buffer_ += std::to_string(val);
    return *this;
  }

  std::string str() const {
    return buffer_;
  }

  void clear() {
    buffer_.clear();
  }

  void reset() {
    buffer_.clear();
    buffer_.reserve(1024);
  }
};

// Benchmark helper
template<typename Func>
double benchmark(const std::string& name, Func&& func, int iterations) {
  auto start = std::chrono::high_resolution_clock::now();
  
  for (int i = 0; i < iterations; ++i) {
    func();
  }
  
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  double avg_time = duration.count() / static_cast<double>(iterations);
  
  std::cout << name << ": " << avg_time << " µs per iteration\n";
  return avg_time;
}

int main() {
  const int ITERATIONS = 100000;
  
  std::cout << "Performance Comparison: std::ostringstream vs FastOStringStream\n";
  std::cout << "Iterations: " << ITERATIONS << "\n\n";

  // Create test data
  SimpleStruct simple;
  simple.__set_id(42);
  simple.__set_name("performance_test_name");
  simple.__set_description("This is a performance test description");

  NestedStruct nested;
  nested.__set_simple(simple);
  nested.__set_tags({"tag1", "tag2", "tag3", "tag4", "tag5"});
  nested.__set_scores({
    {"score1", 100}, {"score2", 200}, {"score3", 300},
    {"score4", 400}, {"score5", 500}
  });

  std::cout << "=== SimpleStruct Performance ===\n";
  
  // Benchmark with std::ostringstream
  double std_time = benchmark("std::ostringstream", [&]() {
    std::ostringstream oss;
    oss << simple;
    oss.str();  // Get the string
  }, ITERATIONS);

  // Benchmark with FastOStringStream
  double fast_time = benchmark("FastOStringStream ", [&]() {
    FastOStringStream fos;
    fos << simple;
    fos.str();  // Get the string
  }, ITERATIONS);

  double speedup = std_time / fast_time;
  std::cout << "Speedup: " << speedup << "x\n\n";

  std::cout << "=== NestedStruct Performance ===\n";
  
  // Benchmark with std::ostringstream
  double std_time_nested = benchmark("std::ostringstream", [&]() {
    std::ostringstream oss;
    oss << nested;
    oss.str();
  }, ITERATIONS);

  // Benchmark with FastOStringStream
  double fast_time_nested = benchmark("FastOStringStream ", [&]() {
    FastOStringStream fos;
    fos << nested;
    fos.str();
  }, ITERATIONS);

  double speedup_nested = std_time_nested / fast_time_nested;
  std::cout << "Speedup: " << speedup_nested << "x\n\n";

  // Verify correctness
  std::cout << "=== Correctness Verification ===\n";
  std::ostringstream oss;
  oss << simple;
  std::string std_result = oss.str();

  FastOStringStream fos;
  fos << simple;
  std::string fast_result = fos.str();

  if (std_result == fast_result) {
    std::cout << "✓ Both implementations produce identical output\n";
  } else {
    std::cout << "✗ WARNING: Output mismatch!\n";
    std::cout << "std::ostringstream: " << std_result << "\n";
    std::cout << "FastOStringStream:  " << fast_result << "\n";
    return 1;
  }

  std::cout << "\nSample output:\n" << std_result << "\n";

  return 0;
}
