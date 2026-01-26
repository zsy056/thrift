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

#include "../catch/catch.hpp"
#include <thrift/parse/t_program.h>
#include <thrift/generate/t_generator.h>
#include <thrift/generate/t_generator_registry.h>
#include <thrift/globals.h>
#include <thrift/main.h>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>

#include <algorithm>

using std::string;
using std::map;
using std::ifstream;
using std::ofstream;
using std::ostringstream;

// Provided by compiler/cpp/tests/thrift_test_parser_support.cc
extern std::string g_curdir;
extern std::string g_curpath;

// Helper function to read file content
static string read_file(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static string source_dir() {
    string file = __FILE__;
    std::replace(file.begin(), file.end(), '\\', '/');
    size_t slash = file.rfind('/');
    return (slash == string::npos) ? string(".") : file.substr(0, slash);
}

static string join_path(const string& a, const string& b) {
    if (a.empty()) {
        return b;
    }
    if (a.back() == '/' || a.back() == '\\') {
        return a + b;
    }
    return a + "/" + b;
}

static string normalize_for_compare(string s) {
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());

    std::istringstream in(s);
    std::ostringstream out;
    string line;
    bool in_block_comment = false;
    bool first = true;
    while (std::getline(in, line)) {
        while (!line.empty() && (line.back() == ' ' || line.back() == '\t')) {
            line.pop_back();
        }

        const auto first_non_ws = line.find_first_not_of(" \t");
        if (first_non_ws == string::npos) {
            continue;
        }

        const string trimmed = line.substr(first_non_ws);

        if (in_block_comment) {
            if (trimmed.find("*/") != string::npos) {
                in_block_comment = false;
            }
            continue;
        }

        if (trimmed.size() >= 2 && trimmed.compare(0, 2, "//") == 0) {
            continue;
        }

        if (trimmed.size() >= 2 && trimmed.compare(0, 2, "/*") == 0) {
            if (trimmed.find("*/") == string::npos) {
                in_block_comment = true;
            }
            continue;
        }

        if (!first) {
            out << '\n';
        }
        first = false;
        out << line;
    }

    return out.str();
}

// Helper function to extract enum definition from generated header
string extract_enum_definition(const string& content, const string& enum_name) {
    // Look for the enum definition
    size_t enum_start = content.find("enum " + enum_name);
    if (enum_start == string::npos) {
        // Try to find enum class
        enum_start = content.find("enum class " + enum_name);
        if (enum_start == string::npos) {
            return "";
        }
    }
    
    // Find the opening brace
    size_t brace_start = content.find("{", enum_start);
    if (brace_start == string::npos) {
        return "";
    }
    
    // Find the matching closing brace
    size_t brace_end = content.find("};", brace_start);
    if (brace_end == string::npos) {
        return "";
    }
    
    return content.substr(enum_start, brace_end - enum_start + 2);
}

static void parse_thrift_for_test(t_program* program) {
    REQUIRE(program != nullptr);

    // These globals are used by the parser; see thrift/globals.h.
    g_program = program;
    g_scope = program->scope();
    g_parent_scope = nullptr;
    g_parent_prefix = program->get_name() + ".";

    g_curpath = program->get_path();
    g_curdir = directory_name(g_curpath);

    // Pass 1: scan includes (even if none) to match the compiler behavior.
    g_parse_mode = INCLUDES;
    yylineno = 1;
    yyin = std::fopen(g_curpath.c_str(), "r");
    REQUIRE(yyin != nullptr);
    REQUIRE(yyparse() == 0);
    std::fclose(yyin);
    yyin = nullptr;

    // Pass 2: parse program.
    g_parse_mode = PROGRAM;
    yylineno = 1;
    yyin = std::fopen(g_curpath.c_str(), "r");
    REQUIRE(yyin != nullptr);
    REQUIRE(yyparse() == 0);
    std::fclose(yyin);
    yyin = nullptr;
}

TEST_CASE("t_cpp_generator default behavior generates wrapper struct for enums", "[functional]")
{
    string path = join_path(source_dir(), "test_enum_class.thrift");
    string name = "test_enum_class";
    map<string, string> parsed_options = {}; // No options
    string option_string = "";

    std::unique_ptr<t_program> program(new t_program(path, name));
    parse_thrift_for_test(program.get());
    
    std::unique_ptr<t_generator> gen(
        t_generator_registry::get_generator(program.get(), "cpp", parsed_options, option_string));
    REQUIRE(gen != nullptr);
    
    // Generate code
    REQUIRE_NOTHROW(gen->generate_program());

    // Read generated output
    string generated_file = "gen-cpp/test_enum_class_types.h";
    string generated_content = read_file(generated_file);
    REQUIRE(!generated_content.empty());

    // Verify it generates a struct wrapper with an enum inside
    REQUIRE(generated_content.find("struct Color") != string::npos);
    REQUIRE(generated_content.find("enum type") != string::npos);
}

TEST_CASE("t_cpp_generator with pure_enums generates plain enum", "[functional]")
{
    string path = join_path(source_dir(), "test_enum_class.thrift");
    string name = "test_enum_class";
    map<string, string> parsed_options = {{"pure_enums", ""}};
    string option_string = "";

    std::unique_ptr<t_program> program(new t_program(path, name));
    parse_thrift_for_test(program.get());
    
    std::unique_ptr<t_generator> gen(
        t_generator_registry::get_generator(program.get(), "cpp", parsed_options, option_string));
    REQUIRE(gen != nullptr);
    
    // Generate code
    REQUIRE_NOTHROW(gen->generate_program());

    // Read generated output
    string generated_file = "gen-cpp/test_enum_class_types.h";
    string generated_content = read_file(generated_file);
    REQUIRE(!generated_content.empty());

    // Extract enum definition
    string enum_def = extract_enum_definition(generated_content, "Color");
    REQUIRE(!enum_def.empty());

    // Verify it generates a plain enum (not enum class)
    REQUIRE(enum_def.find("enum Color") != string::npos);
    REQUIRE(enum_def.find("enum class") == string::npos);
    
    // Should NOT have struct wrapper
    REQUIRE(generated_content.find("struct Color") == string::npos);
}

TEST_CASE("t_cpp_generator with pure_enums=enum_class generates C++ 11 enum class", "[functional]")
{
    string path = join_path(source_dir(), "test_enum_class.thrift");
    string name = "test_enum_class";
    map<string, string> parsed_options = {{"pure_enums", "enum_class"}};
    string option_string = "";

    std::unique_ptr<t_program> program(new t_program(path, name));
    parse_thrift_for_test(program.get());
    
    std::unique_ptr<t_generator> gen(
        t_generator_registry::get_generator(program.get(), "cpp", parsed_options, option_string));
    REQUIRE(gen != nullptr);
    
    // Generate code
    REQUIRE_NOTHROW(gen->generate_program());

    // Read generated output
    string generated_file = "gen-cpp/test_enum_class_types.h";
    string generated_content = read_file(generated_file);
    REQUIRE(!generated_content.empty());

    // Extract enum definition
    string enum_def = extract_enum_definition(generated_content, "Color");
    REQUIRE(!enum_def.empty());

    // Verify it generates an enum class
    REQUIRE(enum_def.find("enum class Color") != string::npos);
    
    // Should NOT have struct wrapper
    REQUIRE(generated_content.find("struct Color") == string::npos);
}
