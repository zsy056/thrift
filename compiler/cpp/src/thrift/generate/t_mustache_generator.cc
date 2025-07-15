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
      {"cpp_type", &t_type_context::cpp_type},
      {"cpp_arg_type", &t_type_context::cpp_arg_type},
      {"thrift_type", &t_type_context::thrift_type},
      {"read_method", &t_type_context::read_method},
      {"write_method", &t_type_context::write_method},
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
      {"is_void", &t_type_context::is_void},
      {"is_complex_type", &t_type_context::is_complex_type}
    });
  }

private:
  const t_type* type_;

  mstch::node name() {
    return type_->get_name();
  }

  mstch::node cpp_type() {
    return get_cpp_type_name(type_, false, false);
  }

  mstch::node cpp_arg_type() {
    return get_cpp_type_name(type_, false, true);
  }

  mstch::node thrift_type() {
    return get_thrift_protocol_type(type_);
  }

  mstch::node read_method() {
    return get_read_method(type_);
  }

  mstch::node write_method() {
    return get_write_method(type_);
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

  mstch::node is_complex_type() {
    return type_->is_container() || type_->is_struct() || type_->is_xception() || 
           (type_->is_base_type() && (type_->is_string() || type_->is_binary()));
  }

  std::string get_cpp_type_name(const t_type* ttype, bool in_typedef, bool arg) {
    if (ttype->is_base_type()) {
      std::string bname = get_base_type_name(((t_base_type*)ttype)->get_base());
      
      if (!arg) {
        return bname;
      }

      if (ttype->is_string() || ttype->is_binary()) {
        return "const " + bname + "&";
      } else {
        return "const " + bname;
      }
    }

    if (ttype->is_container()) {
      std::string cname;
      if (ttype->is_map()) {
        t_map* tmap = (t_map*)ttype;
        cname = "std::map<" + get_cpp_type_name(tmap->get_key_type(), in_typedef, false) + ", "
                + get_cpp_type_name(tmap->get_val_type(), in_typedef, false) + "> ";
      } else if (ttype->is_set()) {
        t_set* tset = (t_set*)ttype;
        cname = "std::set<" + get_cpp_type_name(tset->get_elem_type(), in_typedef, false) + "> ";
      } else if (ttype->is_list()) {
        t_list* tlist = (t_list*)ttype;
        cname = "std::vector<" + get_cpp_type_name(tlist->get_elem_type(), in_typedef, false) + "> ";
      }

      if (arg) {
        return "const " + cname + "&";
      } else {
        return cname;
      }
    }

    std::string pname = ttype->get_name();
    if (ttype->is_enum()) {
      pname += "::type";
    }

    if (arg) {
      if (is_complex_type_impl(ttype)) {
        return "const " + pname + "&";
      } else {
        return "const " + pname;
      }
    } else {
      return pname;
    }
  }

  bool is_complex_type_impl(const t_type* ttype) {
    return ttype->is_container() || ttype->is_struct() || ttype->is_xception() || 
           (ttype->is_base_type() && (ttype->is_string() || ttype->is_binary()));
  }

  std::string get_base_type_name(t_base_type::t_base tbase) {
    switch (tbase) {
      case t_base_type::TYPE_VOID:
        return "void";
      case t_base_type::TYPE_STRING:
        return "std::string";
      case t_base_type::TYPE_BOOL:
        return "bool";
      case t_base_type::TYPE_I8:
        return "int8_t";
      case t_base_type::TYPE_I16:
        return "int16_t";
      case t_base_type::TYPE_I32:
        return "int32_t";
      case t_base_type::TYPE_I64:
        return "int64_t";
      case t_base_type::TYPE_DOUBLE:
        return "double";
      case t_base_type::TYPE_UUID:
        return "std::string";
      default:
        throw "compiler error: no C++ name for base type " + std::to_string(tbase);
    }
  }

  std::string get_thrift_protocol_type(const t_type* ttype) {
    if (ttype->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)ttype)->get_base();
      switch (tbase) {
        case t_base_type::TYPE_VOID:
          return "T_VOID";
        case t_base_type::TYPE_STRING:
        case t_base_type::TYPE_UUID:
          return "T_STRING";
        case t_base_type::TYPE_BOOL:
          return "T_BOOL";
        case t_base_type::TYPE_I8:
          return "T_BYTE";
        case t_base_type::TYPE_I16:
          return "T_I16";
        case t_base_type::TYPE_I32:
          return "T_I32";
        case t_base_type::TYPE_I64:
          return "T_I64";
        case t_base_type::TYPE_DOUBLE:
          return "T_DOUBLE";
        default:
          return "T_STRING";
      }
    } else if (ttype->is_enum()) {
      return "T_I32";
    } else if (ttype->is_struct() || ttype->is_xception()) {
      return "T_STRUCT";
    } else if (ttype->is_map()) {
      return "T_MAP";
    } else if (ttype->is_set()) {
      return "T_SET";
    } else if (ttype->is_list()) {
      return "T_LIST";
    }
    return "T_VOID";
  }

  std::string get_read_method(const t_type* ttype) {
    if (ttype->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)ttype)->get_base();
      switch (tbase) {
        case t_base_type::TYPE_VOID:
          return "";
        case t_base_type::TYPE_STRING:
        case t_base_type::TYPE_UUID:
          return "String";
        case t_base_type::TYPE_BOOL:
          return "Bool";
        case t_base_type::TYPE_I8:
          return "Byte";
        case t_base_type::TYPE_I16:
          return "I16";
        case t_base_type::TYPE_I32:
          return "I32";
        case t_base_type::TYPE_I64:
          return "I64";
        case t_base_type::TYPE_DOUBLE:
          return "Double";
        default:
          return "String";
      }
    } else if (ttype->is_enum()) {
      return "I32";
    }
    return "";
  }

  std::string get_write_method(const t_type* ttype) {
    return get_read_method(ttype); // Same as read method for basic types
  }
};

/**
 * Context class for field types
 */
class t_field_context : public mstch::object {
public:
  t_field_context(const t_field* field, const std::string& struct_name = "") : field_(field), struct_name_(struct_name) {
    register_methods(this, std::map<std::string, mstch::node(t_field_context::*)()>{
      {"name", &t_field_context::name},
      {"struct_name", &t_field_context::struct_name},
      {"type", &t_field_context::type},
      {"cpp_type", &t_field_context::cpp_type},
      {"thrift_type", &t_field_context::thrift_type},
      {"read_method", &t_field_context::read_method},
      {"write_method", &t_field_context::write_method},
      {"key", &t_field_context::key},
      {"optional", &t_field_context::optional},
      {"required", &t_field_context::required},
      {"is_complex_type", &t_field_context::is_complex_type},
      {"has_next", &t_field_context::has_next}
    });
  }

private:
  const t_field* field_;
  std::string struct_name_;

  mstch::node name() {
    return field_->get_name();
  }

  mstch::node struct_name() {
    return struct_name_;
  }

  mstch::node type() {
    return std::shared_ptr<mstch::object>(new t_type_context(field_->get_type()));
  }

  mstch::node cpp_type() {
    return get_cpp_type_name(field_->get_type(), false, false);
  }

  mstch::node thrift_type() {
    return get_thrift_protocol_type(field_->get_type());
  }

  mstch::node read_method() {
    return get_read_method(field_->get_type());
  }

  mstch::node write_method() {
    return get_write_method(field_->get_type());
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

  mstch::node is_complex_type() {
    const t_type* ttype = field_->get_type();
    return ttype->is_container() || ttype->is_struct() || ttype->is_xception() || 
           (ttype->is_base_type() && (ttype->is_string() || ttype->is_binary()));
  }

  mstch::node has_next() {
    // This would need to be set by the parent context
    return false; // Will be overridden
  }

  std::string get_cpp_type_name(const t_type* ttype, bool in_typedef, bool arg) {
    if (ttype->is_base_type()) {
      std::string bname = get_base_type_name(((t_base_type*)ttype)->get_base());
      
      if (!arg) {
        return bname;
      }

      if (ttype->is_string() || ttype->is_binary()) {
        return "const " + bname + "&";
      } else {
        return "const " + bname;
      }
    }

    if (ttype->is_container()) {
      std::string cname;
      if (ttype->is_map()) {
        t_map* tmap = (t_map*)ttype;
        cname = "std::map<" + get_cpp_type_name(tmap->get_key_type(), in_typedef, false) + ", "
                + get_cpp_type_name(tmap->get_val_type(), in_typedef, false) + "> ";
      } else if (ttype->is_set()) {
        t_set* tset = (t_set*)ttype;
        cname = "std::set<" + get_cpp_type_name(tset->get_elem_type(), in_typedef, false) + "> ";
      } else if (ttype->is_list()) {
        t_list* tlist = (t_list*)ttype;
        cname = "std::vector<" + get_cpp_type_name(tlist->get_elem_type(), in_typedef, false) + "> ";
      }

      if (arg) {
        return "const " + cname + "&";
      } else {
        return cname;
      }
    }

    std::string pname = ttype->get_name();
    if (ttype->is_enum()) {
      pname += "::type";
    }

    if (arg) {
      if (is_complex_type_impl(ttype)) {
        return "const " + pname + "&";
      } else {
        return "const " + pname;
      }
    } else {
      return pname;
    }
  }

  bool is_complex_type_impl(const t_type* ttype) {
    return ttype->is_container() || ttype->is_struct() || ttype->is_xception() || 
           (ttype->is_base_type() && (ttype->is_string() || ttype->is_binary()));
  }

  std::string get_base_type_name(t_base_type::t_base tbase) {
    switch (tbase) {
      case t_base_type::TYPE_VOID:
        return "void";
      case t_base_type::TYPE_STRING:
        return "std::string";
      case t_base_type::TYPE_BOOL:
        return "bool";
      case t_base_type::TYPE_I8:
        return "int8_t";
      case t_base_type::TYPE_I16:
        return "int16_t";
      case t_base_type::TYPE_I32:
        return "int32_t";
      case t_base_type::TYPE_I64:
        return "int64_t";
      case t_base_type::TYPE_DOUBLE:
        return "double";
      case t_base_type::TYPE_UUID:
        return "std::string";
      default:
        throw "compiler error: no C++ name for base type " + std::to_string(tbase);
    }
  }

  std::string get_thrift_protocol_type(const t_type* ttype) {
    if (ttype->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)ttype)->get_base();
      switch (tbase) {
        case t_base_type::TYPE_VOID:
          return "T_VOID";
        case t_base_type::TYPE_STRING:
        case t_base_type::TYPE_UUID:
          return "T_STRING";
        case t_base_type::TYPE_BOOL:
          return "T_BOOL";
        case t_base_type::TYPE_I8:
          return "T_BYTE";
        case t_base_type::TYPE_I16:
          return "T_I16";
        case t_base_type::TYPE_I32:
          return "T_I32";
        case t_base_type::TYPE_I64:
          return "T_I64";
        case t_base_type::TYPE_DOUBLE:
          return "T_DOUBLE";
        default:
          return "T_STRING";
      }
    } else if (ttype->is_enum()) {
      return "T_I32";
    } else if (ttype->is_struct() || ttype->is_xception()) {
      return "T_STRUCT";
    } else if (ttype->is_map()) {
      return "T_MAP";
    } else if (ttype->is_set()) {
      return "T_SET";
    } else if (ttype->is_list()) {
      return "T_LIST";
    }
    return "T_VOID";
  }

  std::string get_read_method(const t_type* ttype) {
    if (ttype->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)ttype)->get_base();
      switch (tbase) {
        case t_base_type::TYPE_VOID:
          return "";
        case t_base_type::TYPE_STRING:
        case t_base_type::TYPE_UUID:
          return "String";
        case t_base_type::TYPE_BOOL:
          return "Bool";
        case t_base_type::TYPE_I8:
          return "Byte";
        case t_base_type::TYPE_I16:
          return "I16";
        case t_base_type::TYPE_I32:
          return "I32";
        case t_base_type::TYPE_I64:
          return "I64";
        case t_base_type::TYPE_DOUBLE:
          return "Double";
        default:
          return "String";
      }
    } else if (ttype->is_enum()) {
      return "I32";
    }
    return "";
  }

  std::string get_write_method(const t_type* ttype) {
    return get_read_method(ttype); // Same as read method for basic types
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
      {"values", &t_enum_context::values},
      {"values_count", &t_enum_context::values_count},
      {"values_list", &t_enum_context::values_list},
      {"values_array", &t_enum_context::values_array},
      {"names_array", &t_enum_context::names_array}
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

  mstch::node values_count() {
    return static_cast<int>(enum_->get_constants().size());
  }

  mstch::node values_list() {
    std::string result;
    const vector<t_enum_value*>& values = enum_->get_constants();
    for (size_t i = 0; i < values.size(); ++i) {
      result += "    " + values[i]->get_name() + " = " + std::to_string(values[i]->get_value());
      if (i < values.size() - 1) {
        result += ",\n";
      } else {
        result += "\n";
      }
    }
    return result;
  }

  mstch::node values_array() {
    std::string result;
    const vector<t_enum_value*>& values = enum_->get_constants();
    for (size_t i = 0; i < values.size(); ++i) {
      result += "  " + enum_->get_name() + "::" + values[i]->get_name();
      if (i < values.size() - 1) {
        result += ",\n";
      } else {
        result += "\n";
      }
    }
    return result;
  }

  mstch::node names_array() {
    std::string result;
    const vector<t_enum_value*>& values = enum_->get_constants();
    for (size_t i = 0; i < values.size(); ++i) {
      result += "  \"" + values[i]->get_name() + "\"";
      if (i < values.size() - 1) {
        result += ",\n";
      } else {
        result += "\n";
      }
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
      {"fields_list", &t_struct_context::fields_list},
      {"init_list", &t_struct_context::init_list},
      {"equality_checks", &t_struct_context::equality_checks},
      {"print_fields", &t_struct_context::print_fields},
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
    for (size_t i = 0; i < fields.size(); ++i) {
      auto field_ctx = std::shared_ptr<t_field_context>(new t_field_context(fields[i], struct_->get_name()));
      result.push_back(field_ctx);
    }
    return result;
  }

  mstch::node fields_list() {
    // Return comma-separated list of field initializers
    std::string result;
    const vector<t_field*>& fields = struct_->get_members();
    for (size_t i = 0; i < fields.size(); ++i) {
      result += fields[i]->get_name() + "(false)";
      if (i < fields.size() - 1) {
        result += ", ";
      }
    }
    return result;
  }

  mstch::node init_list() {
    // Return member initializer list for default constructor
    std::string result;
    const vector<t_field*>& fields = struct_->get_members();
    for (size_t i = 0; i < fields.size(); ++i) {
      result += "     " + fields[i]->get_name() + "(";
      const t_type* type = fields[i]->get_type();
      if (type->is_base_type()) {
        t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
        if (tbase == t_base_type::TYPE_STRING || tbase == t_base_type::TYPE_BOOL) {
          // String and bool get default constructed
          result += ")";
        } else {
          // Numeric types get 0
          result += "0)";
        }
      } else {
        // Other types get default constructed
        result += ")";
      }
      if (i < fields.size() - 1) {
        result += ",\n";
      } else {
        result += " {\n";
      }
    }
    return result;
  }

  mstch::node equality_checks() {
    // Return equality comparison for operator==
    std::string result;
    const vector<t_field*>& fields = struct_->get_members();
    for (size_t i = 0; i < fields.size(); ++i) {
      result += "  if (!(" + fields[i]->get_name() + " == rhs." + fields[i]->get_name() + "))\n";
      result += "    return false;\n";
    }
    return result;
  }

  mstch::node print_fields() {
    // Return comma-separated list for printTo method
    std::string result;
    const vector<t_field*>& fields = struct_->get_members();
    for (size_t i = 0; i < fields.size(); ++i) {
      result += "  out << \"" + fields[i]->get_name() + "=\" << to_string(" + fields[i]->get_name() + ");";
      if (i < fields.size() - 1) {
        result += "\n  out << \", \";\n";
      } else {
        result += "\n";
      }
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
      {"program_name", &t_program_context::program_name},
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

  mstch::node program_name() {
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
  // Generate the main types header and implementation files like cpp generator
  auto context = std::shared_ptr<mstch::object>(new t_program_context(get_program()));
  
  // Generate types header file
  write_template_output("types.h.mustache", get_program()->get_name() + "_types.h", context);
  
  // Generate types implementation file
  write_template_output("types.cpp.mustache", get_program()->get_name() + "_types.cpp", context);
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
  (void)tenum;
  // Enums will be handled in the main types file
}

void t_mustache_generator::generate_struct(t_struct* tstruct) {
  (void)tstruct;
  // Structs will be handled in the main types file
}

void t_mustache_generator::generate_service(t_service* tservice) {
  (void)tservice;
  // Services will be handled separately - for now skip
}

THRIFT_REGISTER_GENERATOR(
    mustache,
    "Mustache Template Generator",
    "    template_dir:    Directory containing mustache templates\n")