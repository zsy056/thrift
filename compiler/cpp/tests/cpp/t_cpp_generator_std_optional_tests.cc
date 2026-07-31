// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements. See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership. The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the License for the
// specific language governing permissions and limitations
// under the License.

#include "t_cpp_generator_test_utils.h"

using cpp_generator_test_utils::join_path;
using cpp_generator_test_utils::parse_thrift_for_test;
using cpp_generator_test_utils::read_file;
using cpp_generator_test_utils::source_dir;
using std::map;
using std::string;

extern string extract_class_definition(const string& content, const string& class_name);

namespace {
void generate_std_optional(const map<string, string>& extra_options = {}) {
  string path = join_path(source_dir(), "test_std_optional.thrift");
  std::unique_ptr<t_program> program(new t_program(path, "test_std_optional"));
  parse_thrift_for_test(program.get());

  map<string, string> options = extra_options;
  options["std_optional"] = "";
  std::unique_ptr<t_generator> generator(
      t_generator_registry::get_generator(program.get(), "cpp", options, ""));
  REQUIRE(generator != nullptr);
  REQUIRE_NOTHROW(generator->generate_program());
}
} // namespace

TEST_CASE("t_cpp_generator keeps legacy optional storage by default", "[functional]") {
  string path = join_path(source_dir(), "test_std_optional.thrift");
  std::unique_ptr<t_program> program(new t_program(path, "test_std_optional"));
  parse_thrift_for_test(program.get());

  std::unique_ptr<t_generator> generator(
      t_generator_registry::get_generator(program.get(), "cpp", {}, ""));
  REQUIRE(generator != nullptr);
  REQUIRE_NOTHROW(generator->generate_program());

  const string header = read_file("gen-cpp/test_std_optional_types.h");
  const string class_definition = extract_class_definition(header, "OptionalFields");
  REQUIRE(header.find("#include <optional>") == string::npos);
  REQUIRE(class_definition.find("int32_t i32_value;") != string::npos);
  REQUIRE(header.find("bool i32_value :1;") != string::npos);
}

TEST_CASE("t_cpp_generator std_optional changes only explicit optional fields", "[functional]") {
  generate_std_optional();

  const string header = read_file("gen-cpp/test_std_optional_types.h");
  const string class_definition = extract_class_definition(header, "OptionalFields");
  REQUIRE(header.find("#include <optional>") != string::npos);
  REQUIRE(class_definition.find("std::optional<bool> bool_value;") != string::npos);
  REQUIRE(class_definition.find("std::optional<int32_t> i32_value;") != string::npos);
  REQUIRE(class_definition.find("std::optional<std::string> string_value;") != string::npos);
  REQUIRE(class_definition.find("std::optional<Shade::type> enum_value;") != string::npos);
  REQUIRE(class_definition.find("std::optional<std::vector<int32_t> > list_value;")
          != string::npos);
  REQUIRE(class_definition.find("std::optional<std::set<std::string> > set_value;")
          != string::npos);
  REQUIRE(class_definition.find("std::optional<std::map<std::string, int32_t> > map_value;")
          != string::npos);
  REQUIRE(class_definition.find("std::optional<Child> struct_value;") != string::npos);
  REQUIRE(class_definition.find("std::optional<::std::shared_ptr<ReferencedChild>> ref_value;")
          != string::npos);
  REQUIRE(class_definition.find("std::optional<Count> typedef_value;") != string::npos);

  REQUIRE(class_definition.find("int32_t default_field;") != string::npos);
  REQUIRE(class_definition.find("int32_t required_field;") != string::npos);
  REQUIRE(header.find("bool default_field :1;") != string::npos);
  REQUIRE(header.find("bool i32_value :1;") == string::npos);
}

TEST_CASE("t_cpp_generator std_optional emits presence-aware operations", "[functional]") {
  generate_std_optional();

  const string implementation = read_file("gen-cpp/test_std_optional_types.cpp");
  REQUIRE(implementation.find("this->i32_value.emplace();") != string::npos);
  REQUIRE(implementation.find("readI32((*(this->i32_value)))") != string::npos);
  REQUIRE(implementation.find("if (this->i32_value.has_value())") != string::npos);
  REQUIRE(implementation.find("writeI32((*(this->i32_value)))") != string::npos);
  REQUIRE(implementation.find("__isset.i32_value") == string::npos);
  REQUIRE(implementation.find("__isset.default_field") != string::npos);
  REQUIRE(implementation.find("i32_value == rhs.i32_value") != string::npos);
  REQUIRE(implementation.find("i32_value.has_value()") != string::npos);
  REQUIRE(implementation.find("to_string(*i32_value)") != string::npos);
}

TEST_CASE("t_cpp_generator std_optional preserves explicit defaults and constants",
          "[functional]") {
  generate_std_optional();

  const string implementation = read_file("gen-cpp/test_std_optional_types.cpp");
  const string constants = read_file("gen-cpp/test_std_optional_constants.cpp");
  REQUIRE(implementation.find("default_i32(42)") != string::npos);
  REQUIRE(implementation.find("default_string(\"default\")") != string::npos);
  REQUIRE(implementation.find("default_list.emplace();") != string::npos);
  REQUIRE(implementation.find("(*default_list).push_back(1);") != string::npos);
  REQUIRE(implementation.find("default_struct.emplace();") != string::npos);
  REQUIRE(implementation.find("(*default_struct).id = 7;") != string::npos);
  REQUIRE(constants.find("OPTIONAL_CONSTANT.i32_value = 17;") != string::npos);
  REQUIRE(constants.find("OPTIONAL_CONSTANT.__isset.i32_value") == string::npos);
}

TEST_CASE("t_cpp_generator std_optional supports option combinations and services",
          "[functional]") {
  generate_std_optional(
      {{"private_optional", ""}, {"moveable_types", "forward_setter"}, {"template_streamop", ""}});

  const string header = read_file("gen-cpp/test_std_optional_types.h");
  const string class_definition = extract_class_definition(header, "OptionalFields");
  const string tcc = read_file("gen-cpp/test_std_optional_types.tcc");
  const string implementation = read_file("gen-cpp/test_std_optional_types.cpp");
  const string constants = read_file("gen-cpp/test_std_optional_constants.cpp");
  const string service_header = read_file("gen-cpp/OptionalService.h");
  const string service_implementation = read_file("gen-cpp/OptionalService.cpp");

  REQUIRE(class_definition.find("const std::optional<int32_t>& __get_i32_value() const")
          != string::npos);
  REQUIRE(tcc.find("this->string_value = ::std::forward<T_>(val);") != string::npos);
  REQUIRE(tcc.find("__isset.string_value") == string::npos);
  REQUIRE(implementation.find("(*default_struct).__set_note(\"child\");") != string::npos);
  REQUIRE(implementation.find("->__get_note().has_value()") != string::npos);
  REQUIRE(constants.find("OPTIONAL_CONSTANT.__set_i32_value(17);") != string::npos);
  REQUIRE(service_header.find("const Child& child") != string::npos);
  REQUIRE(service_header.find("const int32_t count") != string::npos);
  REQUIRE(service_implementation.find("result.__isset.success = true;") != string::npos);
  REQUIRE(service_implementation.find("result.__isset.failure = true;") != string::npos);
}
