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

TEST_CASE("t_cpp_generator default behavior uses std::map for processor map", "[functional]")
{
    string path = join_path(source_dir(), "test_unordered_map.thrift");
    string name = "test_unordered_map";
    map<string, string> parsed_options = {}; // No options
    string option_string = "";

    std::unique_ptr<t_program> program(new t_program(path, name));
    parse_thrift_for_test(program.get());

    std::unique_ptr<t_generator> gen(
        t_generator_registry::get_generator(program.get(), "cpp", parsed_options, option_string));
    REQUIRE(gen != nullptr);

    REQUIRE_NOTHROW(gen->generate_program());

    string generated_file = "gen-cpp/TestService.h";
    string generated_content = read_file(generated_file);
    REQUIRE(!generated_content.empty());

    // Without the option, std::map should be used
    REQUIRE(generated_content.find("typedef std::map<std::string, ProcessFunction> ProcessMap;") != string::npos);
    // std::unordered_map should not be used
    REQUIRE(generated_content.find("std::unordered_map") == string::npos);
    // The unordered_map header should not be included
    REQUIRE(generated_content.find("#include <unordered_map>") == string::npos);
}

TEST_CASE("t_cpp_generator with unordered_processor_map uses std::unordered_map for processor map", "[functional]")
{
    string path = join_path(source_dir(), "test_unordered_map.thrift");
    string name = "test_unordered_map";
    map<string, string> parsed_options = {{"unordered_processor_map", ""}};
    string option_string = "";

    std::unique_ptr<t_program> program(new t_program(path, name));
    parse_thrift_for_test(program.get());

    std::unique_ptr<t_generator> gen(
        t_generator_registry::get_generator(program.get(), "cpp", parsed_options, option_string));
    REQUIRE(gen != nullptr);

    REQUIRE_NOTHROW(gen->generate_program());

    string generated_file = "gen-cpp/TestService.h";
    string generated_content = read_file(generated_file);
    REQUIRE(!generated_content.empty());

    // With the option, std::unordered_map should be used
    REQUIRE(generated_content.find("typedef std::unordered_map<std::string, ProcessFunction> ProcessMap;") != string::npos);
    // std::map should not be used for ProcessMap
    REQUIRE(generated_content.find("typedef std::map<std::string, ProcessFunction> ProcessMap;") == string::npos);
    // The unordered_map header should be included
    REQUIRE(generated_content.find("#include <unordered_map>") != string::npos);
}

TEST_CASE("t_cpp_generator with unordered_processor_map and templates uses std::unordered_map for processor map", "[functional]")
{
    string path = join_path(source_dir(), "test_unordered_map.thrift");
    string name = "test_unordered_map";
    map<string, string> parsed_options = {{"unordered_processor_map", ""}, {"templates", ""}};
    string option_string = "";

    std::unique_ptr<t_program> program(new t_program(path, name));
    parse_thrift_for_test(program.get());

    std::unique_ptr<t_generator> gen(
        t_generator_registry::get_generator(program.get(), "cpp", parsed_options, option_string));
    REQUIRE(gen != nullptr);

    REQUIRE_NOTHROW(gen->generate_program());

    string generated_file = "gen-cpp/TestService.h";
    string generated_content = read_file(generated_file);
    REQUIRE(!generated_content.empty());

    // With the option and templates, std::unordered_map should be used for ProcessFunctions map
    REQUIRE(generated_content.find("typedef std::unordered_map<std::string, ProcessFunctions> ProcessMap;") != string::npos);
    // std::map should not be used for ProcessMap
    REQUIRE(generated_content.find("typedef std::map<std::string, ProcessFunctions> ProcessMap;") == string::npos);
    // The unordered_map header should be included
    REQUIRE(generated_content.find("#include <unordered_map>") != string::npos);
}
