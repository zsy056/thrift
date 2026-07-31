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

namespace cpp thrift.test.std_optional_legacy

enum Shade {
  RED = 1,
  BLUE = 2,
}

typedef i32 Count

struct Child {
  1: required i32 id;
  2: optional string note;
}

struct ReferencedChild {
  1: optional string note;
}

struct OptionalFields {
  1: optional bool bool_value;
  2: optional i8 i8_value;
  3: optional i16 i16_value;
  4: optional i32 i32_value;
  5: optional i64 i64_value;
  6: optional double double_value;
  7: optional string string_value;
  8: optional binary binary_value;
  9: optional uuid uuid_value;
  10: optional Shade enum_value;
  11: optional list<i32> list_value;
  12: optional set<string> set_value;
  13: optional map<string, i32> map_value;
  14: optional Child struct_value;
  15: optional i32 default_i32 = 42;
  16: optional string default_string = "default";
  17: optional list<i32> default_list = [1, 2, 3];
  18: optional Child default_struct = {"id": 7, "note": "child"};
  19: i32 default_field;
  20: required i32 required_field;
  21: optional ReferencedChild & ref_value;
  22: optional Count typedef_value;
}
