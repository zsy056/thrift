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

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <mstch/mstch.hpp>

#include "thrift/platform.h"
#include "thrift/generate/t_oop_generator.h"

using std::map;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

/**
 * Context class for enum values
 */
class t_enum_value_context : public mstch::object {
public:
  t_enum_value_context(const t_enum_value* value) : value_(value) {
    register_methods(this, std::map<std::string, mstch::node(t_enum_value_context::*)()>{
      {"name", &t_enum_value_context::name},
      {"value", &t_enum_value_context::value}
    });
  }

private:
  const t_enum_value* value_;

  mstch::node name() {
    return value_->get_name();
  }

  mstch::node value() {
    return value_->get_value();
  }
};

/**
 * Context class for type information
 */
class t_type_context : public mstch::object {
public:
  t_type_context(const t_type* type) : type_(type) {
    register_methods(this, std::map<std::string, mstch::node(t_type_context::*)()>{
      {"name", &t_type_context::name},
      {"is_base_type", &t_type_context::is_base_type},
      {"is_container", &t_type_context::is_container},
      {"is_list", &t_type_context::is_list},
      {"is_set", &t_type_context::is_set},
      {"is_map", &t_type_context::is_map},
      {"is_string", &t_type_context::is_string},
      {"is_binary", &t_type_context::is_binary},
      {"is_bool", &t_type_context::is_bool},
      {"is_struct", &t_type_context::is_struct},
      {"is_enum", &t_type_context::is_enum},
      {"is_void", &t_type_context::is_void}
    });
  }

private:
  const t_type* type_;

  mstch::node name() {
    return type_->get_name();
  }

  mstch::node is_base_type() {
    return type_->is_base_type();
  }

  mstch::node is_container() {
    return type_->is_container();
  }

  mstch::node is_list() {
    return type_->is_list();
  }

  mstch::node is_set() {
    return type_->is_set();
  }

  mstch::node is_map() {
    return type_->is_map();
  }

  mstch::node is_string() {
    return type_->is_string();
  }

  mstch::node is_binary() {
    return type_->is_binary();
  }

  mstch::node is_bool() {
    return type_->is_bool();
  }

  mstch::node is_struct() {
    return type_->is_struct();
  }

  mstch::node is_enum() {
    return type_->is_enum();
  }

  mstch::node is_void() {
    return type_->is_void();
  }
};

/**
 * Context class for field types
 */
class t_field_context : public mstch::object {
public:
  t_field_context(const t_field* field) : field_(field) {
    register_methods(this, std::map<std::string, mstch::node(t_field_context::*)()>{
      {"name", &t_field_context::name},
      {"type", &t_field_context::type},
      {"key", &t_field_context::key},
      {"optional", &t_field_context::optional},
      {"required", &t_field_context::required}
    });
  }

private:
  const t_field* field_;

  mstch::node name() {
    return field_->get_name();
  }

  mstch::node type() {
    return std::shared_ptr<mstch::object>(new t_type_context(field_->get_type()));
  }

  mstch::node key() {
    return static_cast<int>(field_->get_key());
  }

  mstch::node optional() {
    return field_->get_req() == t_field::T_OPTIONAL;
  }

  mstch::node required() {
    return field_->get_req() == t_field::T_REQUIRED;
  }
};

/**
 * Context class for enum types
 */
class t_enum_context : public mstch::object {
public:
  t_enum_context(const t_enum* tenum) : enum_(tenum) {
    register_methods(this, std::map<std::string, mstch::node(t_enum_context::*)()>{
      {"name", &t_enum_context::name},
      {"values", &t_enum_context::values}
    });
  }

private:
  const t_enum* enum_;

  mstch::node name() {
    return enum_->get_name();
  }

  mstch::node values() {
    mstch::array result;
    const vector<t_enum_value*>& values = enum_->get_constants();
    for (const auto& value : values) {
      result.push_back(std::shared_ptr<mstch::object>(new t_enum_value_context(value)));
    }
    return result;
  }
};

/**
 * Context class for struct types
 */
class t_struct_context : public mstch::object {
public:
  t_struct_context(const t_struct* tstruct) : struct_(tstruct) {
    register_methods(this, std::map<std::string, mstch::node(t_struct_context::*)()>{
      {"name", &t_struct_context::name},
      {"fields", &t_struct_context::fields},
      {"is_exception", &t_struct_context::is_exception},
      {"is_union", &t_struct_context::is_union}
    });
  }

private:
  const t_struct* struct_;

  mstch::node name() {
    return struct_->get_name();
  }

  mstch::node fields() {
    mstch::array result;
    const vector<t_field*>& fields = struct_->get_members();
    for (const auto& field : fields) {
      result.push_back(std::shared_ptr<mstch::object>(new t_field_context(field)));
    }
    return result;
  }

  mstch::node is_exception() {
    return struct_->is_xception();
  }

  mstch::node is_union() {
    return struct_->is_union();
  }
};

/**
 * Context class for function types
 */
class t_function_context : public mstch::object {
public:
  t_function_context(const t_function* function) : function_(function) {
    register_methods(this, std::map<std::string, mstch::node(t_function_context::*)()>{
      {"name", &t_function_context::name},
      {"return_type", &t_function_context::return_type},
      {"arguments", &t_function_context::arguments},
      {"exceptions", &t_function_context::exceptions},
      {"is_oneway", &t_function_context::is_oneway}
    });
  }

private:
  const t_function* function_;

  mstch::node name() {
    return function_->get_name();
  }

  mstch::node return_type() {
    return std::shared_ptr<mstch::object>(new t_type_context(function_->get_returntype()));
  }

  mstch::node arguments() {
    mstch::array result;
    const vector<t_field*>& args = function_->get_arglist()->get_members();
    for (const auto& arg : args) {
      result.push_back(std::shared_ptr<mstch::object>(new t_field_context(arg)));
    }
    return result;
  }

  mstch::node exceptions() {
    mstch::array result;
    const vector<t_field*>& exceptions = function_->get_xceptions()->get_members();
    for (const auto& exc : exceptions) {
      result.push_back(std::shared_ptr<mstch::object>(new t_field_context(exc)));
    }
    return result;
  }

  mstch::node is_oneway() {
    return function_->is_oneway();
  }
};

/**
 * Context class for service types
 */
class t_service_context : public mstch::object {
public:
  t_service_context(const t_service* service) : service_(service) {
    register_methods(this, std::map<std::string, mstch::node(t_service_context::*)()>{
      {"name", &t_service_context::name},
      {"functions", &t_service_context::functions},
      {"extends", &t_service_context::extends}
    });
  }

private:
  const t_service* service_;

  mstch::node name() {
    return service_->get_name();
  }

  mstch::node functions() {
    mstch::array result;
    const vector<t_function*>& functions = service_->get_functions();
    for (const auto& func : functions) {
      result.push_back(std::shared_ptr<mstch::object>(new t_function_context(func)));
    }
    return result;
  }

  mstch::node extends() {
    const t_service* extends = service_->get_extends();
    if (extends) {
      return std::shared_ptr<mstch::object>(new t_service_context(extends));
    }
    return mstch::node();
  }
};

/**
 * Context class for typedef types
 */
class t_typedef_context : public mstch::object {
public:
  t_typedef_context(const t_typedef* ttypedef) : typedef_(ttypedef) {
    register_methods(this, std::map<std::string, mstch::node(t_typedef_context::*)()>{
      {"name", &t_typedef_context::name},
      {"type", &t_typedef_context::type}
    });
  }

private:
  const t_typedef* typedef_;

  mstch::node name() {
    return typedef_->get_symbolic();
  }

  mstch::node type() {
    return std::shared_ptr<mstch::object>(new t_type_context(typedef_->get_type()));
  }
};

/**
 * Context class for const types
 */
class t_const_context : public mstch::object {
public:
  t_const_context(const t_const* tconst) : const_(tconst) {
    register_methods(this, std::map<std::string, mstch::node(t_const_context::*)()>{
      {"name", &t_const_context::name},
      {"type", &t_const_context::type},
      {"value", &t_const_context::value}
    });
  }

private:
  const t_const* const_;

  mstch::node name() {
    return const_->get_name();
  }

  mstch::node type() {
    return std::shared_ptr<mstch::object>(new t_type_context(const_->get_type()));
  }

  mstch::node value() {
    // For now, return a string representation of the value
    return const_->get_value()->get_string();
  }
};

/**
 * Adapter class that exposes thrift AST to mstch mustache template engine
 */
class t_program_context : public mstch::object {
public:
  t_program_context(const t_program* program) : program_(program) {
    register_methods(this, std::map<std::string, mstch::node(t_program_context::*)()>{
      {"name", &t_program_context::name},
      {"structs", &t_program_context::structs},
      {"enums", &t_program_context::enums},
      {"services", &t_program_context::services},
      {"typedefs", &t_program_context::typedefs},
      {"consts", &t_program_context::consts},
      {"namespaces", &t_program_context::namespaces},
      {"includes", &t_program_context::includes}
    });
  }

private:
  const t_program* program_;

  mstch::node name() {
    return program_->get_name();
  }

  mstch::node structs() {
    mstch::array result;
    const vector<t_struct*>& structs = program_->get_structs();
    for (const auto& s : structs) {
      result.push_back(std::shared_ptr<mstch::object>(new t_struct_context(s)));
    }
    return result;
  }

  mstch::node enums() {
    mstch::array result;
    const vector<t_enum*>& enums = program_->get_enums();
    for (const auto& e : enums) {
      result.push_back(std::shared_ptr<mstch::object>(new t_enum_context(e)));
    }
    return result;
  }

  mstch::node services() {
    mstch::array result;
    const vector<t_service*>& services = program_->get_services();
    for (const auto& svc : services) {
      result.push_back(std::shared_ptr<mstch::object>(new t_service_context(svc)));
    }
    return result;
  }

  mstch::node typedefs() {
    mstch::array result;
    const vector<t_typedef*>& typedefs = program_->get_typedefs();
    for (const auto& td : typedefs) {
      result.push_back(std::shared_ptr<mstch::object>(new t_typedef_context(td)));
    }
    return result;
  }

  mstch::node consts() {
    mstch::array result;
    const vector<t_const*>& consts = program_->get_consts();
    for (const auto& c : consts) {
      result.push_back(std::shared_ptr<mstch::object>(new t_const_context(c)));
    }
    return result;
  }

  mstch::node namespaces() {
    mstch::map result;
    const std::map<std::string, std::string>& namespaces = program_->get_namespaces();
    for (const auto& ns : namespaces) {
      result[ns.first] = ns.second;
    }
    return result;
  }

  mstch::node includes() {
    mstch::array result;
    const vector<t_program*>& includes = program_->get_includes();
    for (const auto& inc : includes) {
      result.push_back(std::shared_ptr<mstch::object>(new t_program_context(inc)));
    }
    return result;
  }
};

/**
 * Mustache template generator for C++
 */
class t_mustache_generator : public t_oop_generator {
public:
  t_mustache_generator(t_program* program,
                       const std::map<std::string, std::string>& parsed_options,
                       const std::string& option_string)
    : t_oop_generator(program) {
    (void)option_string;
    
    // Parse options
    template_dir_ = "";
    std::map<std::string, std::string>::const_iterator iter;
    for (iter = parsed_options.begin(); iter != parsed_options.end(); ++iter) {
      if (iter->first.compare("template_dir") == 0) {
        template_dir_ = iter->second;
      } else {
        throw "unknown option mustache:" + iter->first;
      }
    }

    out_dir_base_ = "gen-mustache-cpp";
  }

  ~t_mustache_generator() override = default;

  void init_generator() override;
  void close_generator() override;
  std::string display_name() const override;

  void generate_typedef(t_typedef* ttypedef) override;
  void generate_enum(t_enum* tenum) override;
  void generate_struct(t_struct* tstruct) override;
  void generate_service(t_service* tservice) override;

private:
  std::string template_dir_;

  std::string load_template(const std::string& template_name);
  void write_template_output(const std::string& template_name, 
                           const std::string& output_file,
                           std::shared_ptr<mstch::object> context);
};

void t_mustache_generator::init_generator() {
  // Make output directory
  MKDIR(get_out_dir().c_str());
}

void t_mustache_generator::close_generator() {
  // Nothing to do
}

std::string t_mustache_generator::display_name() const {
  return "Mustache Template Generator";
}

std::string t_mustache_generator::load_template(const std::string& template_name) {
  std::string template_path;
  if (!template_dir_.empty()) {
    template_path = template_dir_ + "/" + template_name;
  } else {
    // Default template location
    template_path = "templates/" + template_name;
  }

  std::ifstream template_file(template_path);
  if (!template_file.is_open()) {
    throw std::runtime_error("Could not open template file: " + template_path);
  }

  std::stringstream buffer;
  buffer << template_file.rdbuf();
  return buffer.str();
}

void t_mustache_generator::write_template_output(const std::string& template_name,
                                                const std::string& output_file,
                                                std::shared_ptr<mstch::object> context) {
  std::string template_content = load_template(template_name);
  std::string rendered = mstch::render(template_content, context);
  
  std::string output_path = get_out_dir() + output_file;
  std::ofstream out_file(output_path);
  if (!out_file.is_open()) {
    throw std::runtime_error("Could not open output file: " + output_path);
  }
  
  out_file << rendered;
  out_file.close();
}

void t_mustache_generator::generate_typedef(t_typedef* ttypedef) {
  (void)ttypedef;
  // Typedefs will be handled in the main header template
}

void t_mustache_generator::generate_enum(t_enum* tenum) {
  auto context = std::shared_ptr<mstch::object>(new t_enum_context(tenum));
  write_template_output("enum.mustache", tenum->get_name() + ".h", context);
}

void t_mustache_generator::generate_struct(t_struct* tstruct) {
  auto context = std::shared_ptr<mstch::object>(new t_struct_context(tstruct));
  write_template_output("struct.mustache", tstruct->get_name() + ".h", context);
}

void t_mustache_generator::generate_service(t_service* tservice) {
  auto context = std::shared_ptr<mstch::object>(new t_service_context(tservice));
  write_template_output("service.mustache", tservice->get_name() + ".h", context);
}

THRIFT_REGISTER_GENERATOR(
    mustache,
    "Mustache Template Generator",
    "    template_dir:    Directory containing mustache templates\n")