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

namespace cpp stresssvc

/**
 * Service with 35 methods used to benchmark processor map dispatch.
 * The same .thrift file is compiled twice: once without options (generating
 * a std::map-based ProcessMap in namespace stresssvc) and once with the
 * unordered_processor_map option (generating a std::unordered_map-based
 * ProcessMap).  The second compilation uses the preprocessor define
 * -Dstresssvc=stresssvc_unordered as a namespace trick so both can coexist
 * in the same benchmark binary.
 */
service StressService {
  void method01(),
  void method02(),
  void method03(),
  void method04(),
  void method05(),
  void method06(),
  void method07(),
  void method08(),
  void method09(),
  void method10(),
  void method11(),
  void method12(),
  void method13(),
  void method14(),
  void method15(),
  void method16(),
  void method17(),
  void method18(),
  void method19(),
  void method20(),
  void method21(),
  void method22(),
  void method23(),
  void method24(),
  void method25(),
  void method26(),
  void method27(),
  void method28(),
  void method29(),
  void method30(),
  void method31(),
  void method32(),
  void method33(),
  void method34(),
  void method35(),
}
