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

#include "OptionalService.h"
#include "StdOptionalLegacyTest_types.h"
#include "StdOptionalTest_constants.h"
#include "StdOptionalTest_types.h"

#include <thrift/TUuid.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>

#include <cassert>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace current = thrift::test::std_optional;
namespace legacy = thrift::test::std_optional_legacy;

template <typename T, typename = void>
struct has_i32_value_isset : std::false_type {};

template <typename T>
struct has_i32_value_isset<T, std::void_t<decltype(std::declval<T>().__isset.i32_value)>>
  : std::true_type {};

template <typename Writer, typename Reader>
void write_to_read(const Writer& writer, Reader& reader) {
  auto transport = std::make_shared<apache::thrift::transport::TMemoryBuffer>();
  apache::thrift::protocol::TBinaryProtocol protocol(transport);
  writer.write(&protocol);
  reader.read(&protocol);
}

void verify_types() {
  using Fields = current::OptionalFields;
  static_assert(std::is_same<decltype(Fields::bool_value), std::optional<bool>>::value, "");
  static_assert(std::is_same<decltype(Fields::i8_value), std::optional<int8_t>>::value, "");
  static_assert(std::is_same<decltype(Fields::i16_value), std::optional<int16_t>>::value, "");
  static_assert(std::is_same<decltype(Fields::i32_value), std::optional<int32_t>>::value, "");
  static_assert(std::is_same<decltype(Fields::i64_value), std::optional<int64_t>>::value, "");
  static_assert(std::is_same<decltype(Fields::double_value), std::optional<double>>::value, "");
  static_assert(std::is_same<decltype(Fields::string_value), std::optional<std::string>>::value,
                "");
  static_assert(std::is_same<decltype(Fields::binary_value), std::optional<std::string>>::value,
                "");
  static_assert(
      std::is_same<decltype(Fields::uuid_value), std::optional<apache::thrift::TUuid>>::value, "");
  static_assert(
      std::is_same<decltype(Fields::enum_value), std::optional<current::Shade::type>>::value, "");
  static_assert(
      std::is_same<decltype(Fields::list_value), std::optional<std::vector<int32_t>>>::value, "");
  static_assert(
      std::is_same<decltype(Fields::set_value), std::optional<std::set<std::string>>>::value, "");
  static_assert(std::is_same<decltype(Fields::map_value),
                             std::optional<std::map<std::string, int32_t>>>::value,
                "");
  static_assert(std::is_same<decltype(Fields::struct_value), std::optional<current::Child>>::value,
                "");
  static_assert(std::is_same<decltype(Fields::ref_value),
                             std::optional<std::shared_ptr<current::ReferencedChild>>>::value,
                "");
  static_assert(std::is_same<decltype(Fields::typedef_value), std::optional<current::Count>>::value,
                "");
  static_assert(std::is_same<decltype(Fields::default_field), int32_t>::value, "");
  static_assert(std::is_same<decltype(Fields::required_field), int32_t>::value, "");
  static_assert(!has_i32_value_isset<Fields>::value, "explicit optionals must not use __isset");
}

void verify_default_construction_and_setters() {
  current::OptionalFields fields;
  assert(!fields.bool_value);
  assert(!fields.i32_value);
  assert(!fields.string_value);
  assert(!fields.list_value);
  assert(!fields.struct_value);
  assert(!fields.ref_value);
  assert(!fields.typedef_value);

  assert(fields.default_i32 == 42);
  assert(fields.default_string == "default");
  assert(fields.default_list == std::vector<int32_t>({1, 2, 3}));
  assert(fields.default_struct);
  assert(fields.default_struct->id == 7);
  assert(fields.default_struct->note == "child");

  fields.__set_i32_value(91);
  fields.__set_string_value("value");
  fields.__set_list_value({5, 6});
  assert(fields.i32_value == 91);
  assert(fields.string_value == "value");
  assert(fields.list_value == std::vector<int32_t>({5, 6}));

  fields.i32_value.reset();
  assert(!fields.i32_value);
}

current::OptionalFields populated_fields() {
  current::OptionalFields fields;
  fields.required_field = 100;
  fields.default_field = 101;
  fields.bool_value = true;
  fields.i8_value = 8;
  fields.i16_value = 16;
  fields.i32_value = 32;
  fields.i64_value = 64;
  fields.double_value = 1.25;
  fields.string_value = "text";
  fields.binary_value = std::string("a\0b", 3);
  fields.uuid_value = apache::thrift::TUuid{"5e2ab188-1726-4e75-a04f-1ed9a6a89c4c"};
  fields.enum_value = current::Shade::BLUE;
  fields.list_value = std::vector<int32_t>{1, 2};
  fields.set_value = std::set<std::string>{"a", "b"};
  fields.map_value = std::map<std::string, int32_t>{{"one", 1}, {"two", 2}};
  fields.struct_value.emplace();
  fields.struct_value->id = 77;
  fields.struct_value->note = "nested";
  fields.ref_value = std::make_shared<current::ReferencedChild>();
  (*fields.ref_value)->note = "referenced";
  fields.typedef_value = 79;
  return fields;
}

void verify_value_operations() {
  current::OptionalFields original = populated_fields();
  current::OptionalFields copied(original);
  assert(copied == original);

  copied.string_value.reset();
  assert(copied != original);

  swap(copied, original);
  assert(!original.string_value);
  assert(copied.string_value == "text");

  std::ostringstream stream;
  stream << original;
  assert(stream.str().find("string_value=<null>") != std::string::npos);
  stream.str("");
  stream.clear();
  stream << copied;
  assert(stream.str().find("string_value=text") != std::string::npos);
}

void verify_round_trip() {
  current::OptionalFields source = populated_fields();
  current::OptionalFields target;
  write_to_read(source, target);
  assert(target.ref_value);
  assert(*target.ref_value);
  assert(**target.ref_value == **source.ref_value);
  source.ref_value.reset();
  target.ref_value.reset();
  assert(target == source);

  current::OptionalFields absent;
  absent.required_field = 1;
  current::OptionalFields absent_target;
  write_to_read(absent, absent_target);
  assert(!absent_target.i32_value);
  assert(!absent_target.string_value);
  assert(absent_target.default_i32 == 42);
}

void verify_legacy_wire_compatibility() {
  current::OptionalFields current_source = populated_fields();
  legacy::OptionalFields legacy_target;
  write_to_read(current_source, legacy_target);
  assert(legacy_target.__isset.i32_value);
  assert(legacy_target.i32_value == 32);
  assert(legacy_target.__isset.struct_value);
  assert(legacy_target.struct_value.id == 77);
  assert(legacy_target.struct_value.__isset.note);
  assert(legacy_target.struct_value.note == "nested");
  assert(legacy_target.__isset.ref_value);
  assert(legacy_target.ref_value);
  assert(legacy_target.ref_value->note == "referenced");
  assert(legacy_target.__isset.typedef_value);
  assert(legacy_target.typedef_value == 79);

  legacy::OptionalFields legacy_source;
  legacy_source.required_field = 200;
  legacy_source.i32_value = 201;
  legacy_source.__isset.i32_value = true;
  legacy_source.string_value = "legacy";
  legacy_source.__isset.string_value = true;
  legacy_source.struct_value.id = 202;
  legacy_source.struct_value.note = "legacy child";
  legacy_source.struct_value.__isset.note = true;
  legacy_source.__isset.struct_value = true;
  legacy_source.ref_value = std::make_shared<legacy::ReferencedChild>();
  legacy_source.ref_value->note = "legacy referenced child";
  legacy_source.ref_value->__isset.note = true;
  legacy_source.__isset.ref_value = true;
  legacy_source.typedef_value = 204;
  legacy_source.__isset.typedef_value = true;

  current::OptionalFields current_target;
  write_to_read(legacy_source, current_target);
  assert(current_target.i32_value == 201);
  assert(current_target.string_value == "legacy");
  assert(current_target.struct_value);
  assert(current_target.struct_value->id == 202);
  assert(current_target.struct_value->note == "legacy child");
  assert(current_target.ref_value);
  assert(*current_target.ref_value);
  assert((*current_target.ref_value)->note == "legacy referenced child");
  assert(current_target.typedef_value == 204);
}

void verify_constants_and_exceptions() {
  const auto& constant = current::g_StdOptionalTest_constants.OPTIONAL_CONSTANT;
  assert(constant.i32_value == 17);
  assert(constant.string_value == "constant");
  assert(constant.list_value == std::vector<int32_t>({4, 5}));
  assert(constant.struct_value);
  assert(constant.struct_value->id == 9);
  assert(constant.struct_value->note == "constant child");

  current::Failure failure;
  assert(!failure.reason);
  failure.__set_reason("failure");
  assert(failure.reason == "failure");
  current::Failure round_tripped;
  write_to_read(failure, round_tripped);
  assert(round_tripped == failure);
}

int main() {
  verify_types();
  verify_default_construction_and_setters();
  verify_value_operations();
  verify_round_trip();
  verify_legacy_wire_compatibility();
  verify_constants_and_exceptions();
  return 0;
}
