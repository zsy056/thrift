// Licensed to the Apache Software Foundation(ASF) under one
// or more contributor license agreements.See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.The ASF licenses this file
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

using std::string;
using std::map;
using cpp_generator_test_utils::read_file;
using cpp_generator_test_utils::source_dir;
using cpp_generator_test_utils::join_path;
using cpp_generator_test_utils::normalize_for_compare;
using cpp_generator_test_utils::parse_thrift_for_test;

// Helper function to extract class definition from generated header
string extract_class_definition(const string& content, const string& class_name) {
    size_t class_start = content.find("class " + class_name + " :");
    if (class_start == string::npos) {
        return "";
    }
    
    size_t class_end = content.find("};", class_start);
    if (class_end == string::npos) {
        return "";
    }
    
    return content.substr(class_start, class_end - class_start + 2);
}

TEST_CASE("t_cpp_generator default behavior generates all public fields", "[functional]")
{
    string path = join_path(source_dir(), "test_private_optional.thrift");
    string name = "test_private_optional";
    map<string, string> parsed_options = {}; // No private_optional flag
    string option_string = "";

    std::unique_ptr<t_program> program(new t_program(path, name));
    parse_thrift_for_test(program.get());
    
    std::unique_ptr<t_generator> gen(
        t_generator_registry::get_generator(program.get(), "cpp", parsed_options, option_string));
    REQUIRE(gen != nullptr);
    
    // Generate code
    REQUIRE_NOTHROW(gen->generate_program());

    // Read generated output
    string generated_file = "gen-cpp/test_private_optional_types.h";
    string generated_content = read_file(generated_file);
    REQUIRE(!generated_content.empty());

    // Compare generated class definition against the expected fixture.
    string class_def = extract_class_definition(generated_content, "TestStruct");
    REQUIRE(!class_def.empty());

    string expected_path = join_path(source_dir(), "expected_TestStruct_default.txt");
    string expected_content = read_file(expected_path);
    REQUIRE(!expected_content.empty());

    REQUIRE(normalize_for_compare(class_def) == normalize_for_compare(expected_content));
    
}

TEST_CASE("t_cpp_generator with private_optional generates private optional fields", "[functional]")
{
    string path = join_path(source_dir(), "test_private_optional.thrift");
    string name = "test_private_optional";
    map<string, string> parsed_options = {{"private_optional", ""}};
    string option_string = "";

    std::unique_ptr<t_program> program(new t_program(path, name));
    parse_thrift_for_test(program.get());
    
    std::unique_ptr<t_generator> gen(
        t_generator_registry::get_generator(program.get(), "cpp", parsed_options, option_string));
    REQUIRE(gen != nullptr);
    
    // Generate code
    REQUIRE_NOTHROW(gen->generate_program());

    // Read generated output
    string generated_file = "gen-cpp/test_private_optional_types.h";
    string generated_content = read_file(generated_file);
    REQUIRE(!generated_content.empty());

    // Extract class definition
    string class_def = extract_class_definition(generated_content, "TestStruct");
    REQUIRE(!class_def.empty());

    // Compare generated class definition against the expected fixture.
    string expected_path = join_path(source_dir(), "expected_TestStruct_private_optional.txt");
    string expected_content = read_file(expected_path);
    REQUIRE(!expected_content.empty());

    REQUIRE(normalize_for_compare(class_def) == normalize_for_compare(expected_content));
    
}

TEST_CASE("t_cpp_generator with moveable_types only generates move semantics", "[functional]")
{
    string path = join_path(source_dir(), "test_forward_setter.thrift");
    string name = "test_forward_setter";
    map<string, string> parsed_options = {{"moveable_types", ""}};
    string option_string = "";

    std::unique_ptr<t_program> program(new t_program(path, name));
    parse_thrift_for_test(program.get());
    
    std::unique_ptr<t_generator> gen(
        t_generator_registry::get_generator(program.get(), "cpp", parsed_options, option_string));
    REQUIRE(gen != nullptr);
    
    // Generate code
    REQUIRE_NOTHROW(gen->generate_program());

    // Read generated output
    string generated_file = "gen-cpp/test_forward_setter_types.h";
    string generated_content = read_file(generated_file);
    REQUIRE(!generated_content.empty());

    // Extract class definition
    string class_def = extract_class_definition(generated_content, "TestForwardSetter");
    REQUIRE(!class_def.empty());

    // Verify move constructor and move assignment are present
    REQUIRE(class_def.find("TestForwardSetter(TestForwardSetter&&)") != string::npos);
    REQUIRE(class_def.find("TestForwardSetter& operator=(TestForwardSetter&&)") != string::npos);
    
    // Verify setters are NOT templated (traditional setters)
    REQUIRE(class_def.find("void __set_complex_string(const std::string& val);") != string::npos);
    // Should not have template setters
    bool hasTemplateSetters = (class_def.find("template <typename T_>") != string::npos);
    REQUIRE(!hasTemplateSetters);
}

TEST_CASE("t_cpp_generator with moveable_types=forward_setter generates forwarding setters", "[functional]")
{
    string path = join_path(source_dir(), "test_forward_setter.thrift");
    string name = "test_forward_setter";
    map<string, string> parsed_options = {{"moveable_types", "forward_setter"}};
    string option_string = "";

    std::unique_ptr<t_program> program(new t_program(path, name));
    parse_thrift_for_test(program.get());
    
    std::unique_ptr<t_generator> gen(
        t_generator_registry::get_generator(program.get(), "cpp", parsed_options, option_string));
    REQUIRE(gen != nullptr);
    
    // Generate code
    REQUIRE_NOTHROW(gen->generate_program());

    // Read generated output
    string generated_file = "gen-cpp/test_forward_setter_types.h";
    string generated_content = read_file(generated_file);
    REQUIRE(!generated_content.empty());

    // Extract class definition
    string class_def = extract_class_definition(generated_content, "TestForwardSetter");
    REQUIRE(!class_def.empty());

    // Verify move constructor and move assignment are present
    REQUIRE(class_def.find("TestForwardSetter(TestForwardSetter&&)") != string::npos);
    REQUIRE(class_def.find("TestForwardSetter& operator=(TestForwardSetter&&)") != string::npos);
    
    // Verify setters for complex types use templates with perfect forwarding
    REQUIRE(class_def.find("template <typename T_>") != string::npos);
    REQUIRE(class_def.find("void __set_complex_string(T_&& val)") != string::npos);
    REQUIRE(class_def.find("::std::forward<T_>(val)") != string::npos);
    
    // Verify primitive setters are NOT templated
    REQUIRE(class_def.find("void __set_primitive_field(const int32_t val);") != string::npos);
    REQUIRE(class_def.find("void __set_primitive_bool(const bool val);") != string::npos);
}

