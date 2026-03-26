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
 * Benchmark: std::map vs std::unordered_map for Thrift processor dispatch.
 *
 * Measures the per-call overhead of processor map lookup using real generated
 * service code (StressService, 35 methods).  Two static libraries are linked:
 *
 *   - libstresssvcgencpp    : generated with the default std::map ProcessMap
 *                             (namespace stresssvc)
 *   - libstresssvcgencpp_unordered: generated with --gen cpp:unordered_processor_map
 *                             compiled with -Dstresssvc=stresssvc_unordered so
 *                             it lives in a distinct namespace
 *
 * Messages are encoded once into raw binary protocol buffers and then
 * fed to each processor through TMemoryBuffer, keeping the network stack out
 * of the picture.
 */

#include "StressService.h"
#include "ProcessorMapBenchmarkUnorderedFactory.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using apache::thrift::protocol::TBinaryProtocol;
using apache::thrift::transport::TMemoryBuffer;

static const int NUM_METHODS = 35;
static const int NUM_LOOKUPS = 1000000;

// Encode a single void method call into a raw binary protocol byte buffer.
static std::vector<uint8_t> encodeCall(const std::string& methodName) {
  auto mem = std::make_shared<TMemoryBuffer>();
  auto proto = std::make_shared<TBinaryProtocol>(mem);
  proto->writeMessageBegin(methodName, apache::thrift::protocol::T_CALL, 1);
  proto->writeStructBegin("");
  proto->writeFieldStop();
  proto->writeStructEnd();
  proto->writeMessageEnd();
  uint8_t* buf;
  uint32_t sz;
  mem->getBuffer(&buf, &sz);
  return std::vector<uint8_t>(buf, buf + sz);
}

// Build encoded call buffers for all NUM_METHODS methods.
static std::vector<std::vector<uint8_t>> buildEncodedCalls() {
  static const char* kMethodNames[NUM_METHODS] = {
    "ctgdctopacgpossklhwtekhftcjjig",
    "snzjovqwpsb",
    "uzwwjlvejrsrbqngm",
    "zqorvufaigfywirkxl",
    "dtotlpjzdmtwderpkp",
    "nzwasrngqcllywgnexwhqpdt",
    "yxxpmcjzuggoctvzndryenpzytr",
    "krjxlgbaopasbbainhm",
    "dyomuuecllmsrzckiwgelkhgylw",
    "cqoactgkmwxeakyuhssurmefbtb",
    "bpnweyceprgdxgpqkpny",
    "rigdqlyyaqocnndj",
    "wjevspzravhr",
    "gxtehfirwngxbq",
    "bidzelsocyrdajolbj",
    "xfgcuwkqeyw",
    "watiyuwvgucwwfv",
    "jvwfrkhbujnrviawgpzapiul",
    "dxeblnurytflyxqrdxnrisoerj",
    "qhoahlprmlfmxue",
    "hrqcygiudbzp",
    "lxosjkdrxwxu",
    "eynbgwlurzpyxpsnv",
    "lewltvhumglsgixcdjdfkimbvku",
    "wtjajtwykt",
    "uxmhmkrusvxznxprclwegiliwhy",
    "gcaozjquqtyfaddrofstept",
    "ueipyoxjyfpvg",
    "wpdzujljniwvcyqvslt",
    "hvnmltgwvcwgmpjawefuiwshaycsmt",
    "qmucfrhpmnltishgdfdntefms",
    "pzvhjhsdbbhyqtcahmizgwcbmp",
    "dvycgokpquj",
    "hplobyctqgcgoekrmahhwbgg",
    "pufcfdgdbupvxlmhkholpxtpafrwpa",
  };
  std::vector<std::vector<uint8_t>> calls;
  calls.reserve(NUM_METHODS);
  for (int i = 0; i < NUM_METHODS; ++i) {
    calls.push_back(encodeCall(kMethodNames[i]));
  }
  return calls;
}

// Run the processor against NUM_LOOKUPS requests cycling over all methods.
// Returns elapsed time in nanoseconds.
static long long runBenchmark(const std::shared_ptr<apache::thrift::TProcessor>& processor,
                               const std::vector<std::vector<uint8_t>>& encodedCalls) {
  // Pre-create transport/protocol stacks once, outside the hot loop.
  auto outMem = std::make_shared<TMemoryBuffer>();
  auto outProto = std::make_shared<TBinaryProtocol>(outMem);

  std::vector<std::shared_ptr<TMemoryBuffer>> inMems;
  std::vector<std::shared_ptr<TBinaryProtocol>> inProtos;
  inMems.reserve(NUM_METHODS);
  inProtos.reserve(NUM_METHODS);
  for (int i = 0; i < NUM_METHODS; ++i) {
    auto mem = std::make_shared<TMemoryBuffer>(
        const_cast<uint8_t*>(encodedCalls[i].data()),
        static_cast<uint32_t>(encodedCalls[i].size()));
    inMems.push_back(mem);
    inProtos.push_back(std::make_shared<TBinaryProtocol>(mem));
  }

  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < NUM_LOOKUPS; ++i) {
    int idx = i % NUM_METHODS;
    const auto& data = encodedCalls[idx];
    // Reset input buffer to the beginning of the pre-encoded call (no allocation).
    inMems[idx]->resetBuffer(const_cast<uint8_t*>(data.data()),
                              static_cast<uint32_t>(data.size()),
                              TMemoryBuffer::OBSERVE);
    // Reset output buffer so it doesn't grow unboundedly.
    outMem->resetBuffer();
    processor->process(inProtos[idx], outProto, nullptr);
  }
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

int main() {
  auto encodedCalls = buildEncodedCalls();

  // Ordered processor (std::map ProcessMap, namespace stresssvc)
  auto orderedHandler = std::make_shared<stresssvc::StressServiceNull>();
  auto orderedProc = std::make_shared<stresssvc::StressServiceProcessor>(orderedHandler);

  // Unordered processor (std::unordered_map ProcessMap, namespace stresssvc_unordered)
  auto unorderedProc = createUnorderedStressProcessor();

  std::cout << "Processor map dispatch benchmark (StressService, " << NUM_METHODS
            << " methods)\n";
  std::cout << "Lookups per run: " << NUM_LOOKUPS << "\n\n";

  // Warm up
  runBenchmark(orderedProc, encodedCalls);
  runBenchmark(unorderedProc, encodedCalls);

  long long orderedNs = runBenchmark(orderedProc, encodedCalls);
  long long unorderedNs = runBenchmark(unorderedProc, encodedCalls);

  double orderedPerLookup = static_cast<double>(orderedNs) / NUM_LOOKUPS;
  double unorderedPerLookup = static_cast<double>(unorderedNs) / NUM_LOOKUPS;
  double speedup = static_cast<double>(orderedNs) / unorderedNs;

  std::cout << "std::map           total=" << orderedNs
            << "ns  per_lookup=" << orderedPerLookup << "ns\n";
  std::cout << "std::unordered_map total=" << unorderedNs
            << "ns  per_lookup=" << unorderedPerLookup << "ns\n";
  std::cout << "speedup (unordered/ordered): " << speedup << "x\n";

  return 0;
}
