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
 * Benchmark: std::map vs std::unordered_map for Thrift processor dispatch
 *
 * This benchmark measures the lookup performance of std::map versus
 * std::unordered_map when used as the processor map in generated Thrift
 * service processors. The processor map maps method name strings to
 * function pointers, and is looked up on every incoming RPC call.
 *
 * Build:
 *   g++ -O2 -std=c++11 -o processor_map_benchmark processor_map_benchmark.cc
 *
 * Run:
 *   ./processor_map_benchmark
 */

#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

// Simulate a function pointer type used in the processor map
typedef void (*ProcessFunction)();

static void dummy_function() {}

// Populate a map with N method names of the form "method_0", "method_1", ...
template <typename MapType>
static MapType build_processor_map(int num_methods) {
  MapType m;
  for (int i = 0; i < num_methods; ++i) {
    m["method_" + std::to_string(i)] = &dummy_function;
  }
  return m;
}

// Return method names in a shuffled-ish order to avoid branch-prediction bias
static std::vector<std::string> build_lookup_keys(int num_methods, int num_lookups) {
  std::vector<std::string> keys;
  keys.reserve(num_lookups);
  for (int i = 0; i < num_lookups; ++i) {
    keys.push_back("method_" + std::to_string(i % num_methods));
  }
  return keys;
}

template <typename MapType>
static long long run_benchmark(const MapType& m,
                               const std::vector<std::string>& keys) {
  volatile int found = 0; // prevent dead-code elimination
  auto start = std::chrono::high_resolution_clock::now();

  for (const auto& key : keys) {
    auto it = m.find(key);
    if (it != m.end()) {
      ++found;
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  (void)found;
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

static void print_result(const std::string& label, int num_methods,
                         long long ns, int num_lookups) {
  double ns_per_lookup = static_cast<double>(ns) / num_lookups;
  std::cout << label
            << "  methods=" << num_methods
            << "  total=" << ns << "ns"
            << "  per_lookup=" << ns_per_lookup << "ns"
            << "\n";
}

int main() {
  const int NUM_LOOKUPS = 1000000;
  const std::vector<int> METHOD_COUNTS = {5, 10, 20, 50, 100};

  std::cout << "Processor map dispatch benchmark\n";
  std::cout << "Lookups per run: " << NUM_LOOKUPS << "\n\n";

  for (int num_methods : METHOD_COUNTS) {
    auto ordered_map = build_processor_map<std::map<std::string, ProcessFunction>>(num_methods);
    auto unordered_map = build_processor_map<std::unordered_map<std::string, ProcessFunction>>(num_methods);
    auto keys = build_lookup_keys(num_methods, NUM_LOOKUPS);

    // Warm up
    run_benchmark(ordered_map, keys);
    run_benchmark(unordered_map, keys);

    long long ordered_ns   = run_benchmark(ordered_map, keys);
    long long unordered_ns = run_benchmark(unordered_map, keys);

    print_result("std::map          ", num_methods, ordered_ns,   NUM_LOOKUPS);
    print_result("std::unordered_map", num_methods, unordered_ns, NUM_LOOKUPS);

    double speedup = static_cast<double>(ordered_ns) / unordered_ns;
    std::cout << "  speedup (unordered/ordered): " << speedup << "x\n\n";
  }

  return 0;
}
