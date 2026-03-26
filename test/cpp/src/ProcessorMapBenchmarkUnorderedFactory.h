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

#ifndef PROCESSOR_MAP_BENCHMARK_UNORDERED_FACTORY_H
#define PROCESSOR_MAP_BENCHMARK_UNORDERED_FACTORY_H

#include <memory>
#include <thrift/TProcessor.h>

/**
 * Creates a StressServiceProcessor backed by an unordered_processor_map.
 *
 * This factory is compiled into a separate translation unit with the
 * preprocessor define -Dstresssvc=stresssvc_unordered so the generated
 * symbols live in a different namespace from the ordered version, allowing
 * both to be linked into the same benchmark binary.
 */
std::shared_ptr<apache::thrift::TProcessor> createUnorderedStressProcessor();

#endif // PROCESSOR_MAP_BENCHMARK_UNORDERED_FACTORY_H
