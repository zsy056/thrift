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
 * Factory for the unordered_processor_map variant of StressServiceProcessor.
 *
 * This file is compiled with -Dstresssvc=stresssvc_unordered so every
 * occurrence of the namespace token 'stresssvc' in both this file and the
 * included generated header becomes 'stresssvc_unordered'.  That places the
 * processor symbols in a distinct namespace, allowing both the ordered and
 * unordered versions to be linked into the same benchmark binary without
 * duplicate-symbol errors.
 */

#include "ProcessorMapBenchmarkUnorderedFactory.h"

// Compiled with -Dstresssvc=stresssvc_unordered: 'stresssvc' in the
// generated header is textually replaced, putting everything into the
// stresssvc_unordered namespace.
#include "StressService.h"

#include <memory>

std::shared_ptr<apache::thrift::TProcessor> createUnorderedStressProcessor() {
  auto handler = std::make_shared<stresssvc::StressServiceNull>();
  return std::make_shared<stresssvc::StressServiceProcessor>(handler);
}
