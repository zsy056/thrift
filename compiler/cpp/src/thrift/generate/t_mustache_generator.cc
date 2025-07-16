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

// Helper functions for C++ type generation
static std::string get_base_type_name(t_base_type::t_base tbase) {
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

static bool is_complex_type_impl(const t_type* ttype) {
  return ttype->is_container() || ttype->is_struct() || ttype->is_xception() || 
         (ttype->is_base_type() && (ttype->is_string() || ttype->is_binary()));
}

static std::string get_cpp_type_name(const t_type* ttype, bool in_typedef, bool arg) {
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

static std::string get_thrift_protocol_type(const t_type* ttype) {
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

static std::string get_read_method(const t_type* ttype, const std::string& field_name, bool is_pointer = false) {
  std::string field_ref = is_pointer ? "(*" + field_name + ")" : field_name;
  
  if (ttype->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)ttype)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_VOID:
        return "";
      case t_base_type::TYPE_STRING:
      case t_base_type::TYPE_UUID:
        return "iprot->readString(" + field_ref + ")";
      case t_base_type::TYPE_BOOL:
        return "iprot->readBool(" + field_ref + ")";
      case t_base_type::TYPE_I8:
        return "iprot->readByte(" + field_ref + ")";
      case t_base_type::TYPE_I16:
        return "iprot->readI16(" + field_ref + ")";
      case t_base_type::TYPE_I32:
        return "iprot->readI32(" + field_ref + ")";
      case t_base_type::TYPE_I64:
        return "iprot->readI64(" + field_ref + ")";
      case t_base_type::TYPE_DOUBLE:
        return "iprot->readDouble(" + field_ref + ")";
      default:
        return "iprot->readString(" + field_ref + ")";
    }
  } else if (ttype->is_enum()) {
    return "iprot->readI32(" + field_ref + ")";
  } else if (ttype->is_struct() || ttype->is_xception()) {
    return field_ref + ".read(iprot)";
  } else if (ttype->is_container()) {
    // For containers, we need to generate more complex read logic
    return field_ref + ".read(iprot)";
  }
  return "";
}

static std::string get_write_method(const t_type* ttype, const std::string& field_name, bool is_pointer = false) {
  std::string field_ref = is_pointer ? "(*" + field_name + ")" : field_name;
  
  if (ttype->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)ttype)->get_base();
    switch (tbase) {
      case t_base_type::TYPE_VOID:
        return "";
      case t_base_type::TYPE_STRING:
      case t_base_type::TYPE_UUID:
        return "oprot->writeString(" + field_ref + ")";
      case t_base_type::TYPE_BOOL:
        return "oprot->writeBool(" + field_ref + ")";
      case t_base_type::TYPE_I8:
        return "oprot->writeByte(" + field_ref + ")";
      case t_base_type::TYPE_I16:
        return "oprot->writeI16(" + field_ref + ")";
      case t_base_type::TYPE_I32:
        return "oprot->writeI32(" + field_ref + ")";
      case t_base_type::TYPE_I64:
        return "oprot->writeI64(" + field_ref + ")";
      case t_base_type::TYPE_DOUBLE:
        return "oprot->writeDouble(" + field_ref + ")";
      default:
        return "oprot->writeString(" + field_ref + ")";
    }
  } else if (ttype->is_enum()) {
    return "oprot->writeI32(" + field_ref + ")";
  } else if (ttype->is_struct() || ttype->is_xception()) {
    return field_ref + ".write(oprot)";
  } else if (ttype->is_container()) {
    // For containers, we need to generate more complex write logic
    return field_ref + ".write(oprot)";
  }
  return "";
}

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
    return is_complex_type_impl(type_);
  }
};

/**
 * Context class for field types
 */
class t_field_context : public mstch::object {
public:
  t_field_context(const t_field* field, const std::string& struct_name = "", bool is_pointer = false) 
    : field_(field), struct_name_(struct_name), synthetic_name_(""), synthetic_type_(nullptr), synthetic_key_(0), is_last_(false), is_pointer_(is_pointer) {
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
      {"has_next", &t_field_context::has_next},
      {"last", &t_field_context::last},
      {"is_pointer", &t_field_context::is_pointer}
    });
  }

  // Constructor for synthetic fields (like return value "success" field)
  t_field_context(const t_field* field, const std::string& struct_name, const std::string& synthetic_name, 
                  const t_type* synthetic_type, int32_t synthetic_key, bool is_pointer = false)
    : field_(field), struct_name_(struct_name), synthetic_name_(synthetic_name), 
      synthetic_type_(synthetic_type), synthetic_key_(synthetic_key), is_last_(false), is_pointer_(is_pointer) {
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
      {"has_next", &t_field_context::has_next},
      {"last", &t_field_context::last},
      {"is_pointer", &t_field_context::is_pointer}
    });
  }

private:
  const t_field* field_;
  const std::string struct_name_;
  // For synthetic fields
  const std::string synthetic_name_;
  const t_type* synthetic_type_;
  int32_t synthetic_key_;
  bool is_last_;
  bool is_pointer_;

public:
  void set_last(bool last) { is_last_ = last; }

private:

  mstch::node name() {
    if (!synthetic_name_.empty()) {
      return synthetic_name_;
    }
    return field_->get_name();
  }

  mstch::node struct_name() {
    return struct_name_;
  }

  mstch::node type() {
    const t_type* type = synthetic_type_ ? synthetic_type_ : field_->get_type();
    return std::shared_ptr<mstch::object>(new t_type_context(type));
  }

  mstch::node cpp_type() {
    const t_type* type = synthetic_type_ ? synthetic_type_ : field_->get_type();
    if (is_pointer_) {
      return get_cpp_type_name(type, false, false) + "*";
    }
    return get_cpp_type_name(type, false, false);
  }

  mstch::node is_pointer() {
    return is_pointer_;
  }

  mstch::node thrift_type() {
    const t_type* type = synthetic_type_ ? synthetic_type_ : field_->get_type();
    return get_thrift_protocol_type(type);
  }

  mstch::node read_method() {
    const t_type* type = synthetic_type_ ? synthetic_type_ : field_->get_type();
    std::string field_name = synthetic_name_.empty() ? field_->get_name() : synthetic_name_;
    return get_read_method(type, field_name, is_pointer_);
  }

  mstch::node write_method() {
    const t_type* type = synthetic_type_ ? synthetic_type_ : field_->get_type();
    std::string field_name = synthetic_name_.empty() ? field_->get_name() : synthetic_name_;
    return get_write_method(type, field_name, is_pointer_);
  }

  mstch::node key() {
    if (synthetic_key_ != 0) {
      return synthetic_key_;
    }
    return static_cast<int>(field_->get_key());
  }

  mstch::node optional() {
    if (synthetic_type_) {
      return false; // synthetic fields like "success" are not optional
    }
    return field_->get_req() == t_field::T_OPTIONAL;
  }

  mstch::node required() {
    if (synthetic_type_) {
      return false; // synthetic fields are neither required nor optional
    }
    return field_->get_req() == t_field::T_REQUIRED;
  }

  mstch::node is_complex_type() {
    const t_type* ttype = synthetic_type_ ? synthetic_type_ : field_->get_type();
    return ttype->is_container() || ttype->is_struct() || ttype->is_xception() || 
           (ttype->is_base_type() && (ttype->is_string() || ttype->is_binary()));
  }

  mstch::node has_next() {
    // This would need to be set by the parent context
    return false; // Will be overridden
  }

  mstch::node last() {
    return is_last_;
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
  t_function_context(const t_function* function, const std::string& service_name = "") 
    : function_(function), service_name_(service_name) {
    register_methods(this, std::map<std::string, mstch::node(t_function_context::*)()>{
      {"name", &t_function_context::name},
      {"return_type", &t_function_context::return_type},
      {"arguments", &t_function_context::arguments},
      {"exceptions", &t_function_context::exceptions},
      {"is_oneway", &t_function_context::is_oneway},
      {"return_cpp_type", &t_function_context::return_cpp_type},
      {"returns_void", &t_function_context::returns_void},
      {"is_complex_return", &t_function_context::is_complex_return},
      {"cpp_return_signature", &t_function_context::cpp_return_signature},
      {"arguments_signature", &t_function_context::arguments_signature},
      {"cpp_arguments_signature", &t_function_context::cpp_arguments_signature},
      {"arguments_list", &t_function_context::arguments_list},
      {"has_exceptions", &t_function_context::has_exceptions},
      {"args_struct_name", &t_function_context::args_struct_name},
      {"result_struct_name", &t_function_context::result_struct_name},
      {"pargs_struct_name", &t_function_context::pargs_struct_name}
    });
  }

private:
  const t_function* function_;
  const std::string service_name_;

  bool is_complex_return_impl() {
    const t_type* ret_type = function_->get_returntype();
    return is_complex_type_impl(ret_type) && !ret_type->is_void();
  }

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

  mstch::node return_cpp_type() {
    const t_type* ret_type = function_->get_returntype();
    return get_cpp_type_name(ret_type, false, false);
  }

  mstch::node returns_void() {
    return function_->get_returntype()->is_void();
  }

  mstch::node is_complex_return() {
    return is_complex_return_impl();
  }

  mstch::node cpp_return_signature() {
    const t_type* ret_type = function_->get_returntype();
    if (ret_type->is_void()) {
      return std::string("void");
    } else if (is_complex_return_impl()) {
      return std::string("void");
    } else {
      return get_cpp_type_name(ret_type, false, false);
    }
  }

  mstch::node cpp_arguments_signature() {
    std::string result;
    const vector<t_field*>& args = function_->get_arglist()->get_members();
    
    // Add regular arguments
    for (size_t i = 0; i < args.size(); ++i) {
      result += get_cpp_type_name(args[i]->get_type(), false, true) + " " + args[i]->get_name();
      if (i < args.size() - 1 || is_complex_return_impl()) {
        result += ", ";
      }
    }
    
    // Add return reference parameter for complex return types
    const t_type* ret_type = function_->get_returntype();
    if (is_complex_return_impl()) {
      result += get_cpp_type_name(ret_type, false, false) + "& _return";
    }
    
    return std::string(result);
  }

  mstch::node arguments_signature() {
    std::string result;
    const vector<t_field*>& args = function_->get_arglist()->get_members();
    for (size_t i = 0; i < args.size(); ++i) {
      result += get_cpp_type_name(args[i]->get_type(), false, true) + " " + args[i]->get_name();
      if (i < args.size() - 1) {
        result += ", ";
      }
    }
    return result;
  }

  mstch::node arguments_list() {
    std::string result;
    const vector<t_field*>& args = function_->get_arglist()->get_members();
    for (size_t i = 0; i < args.size(); ++i) {
      result += args[i]->get_name();
      if (i < args.size() - 1) {
        result += ", ";
      }
    }
    return result;
  }

  mstch::node has_exceptions() {
    return !function_->get_xceptions()->get_members().empty();
  }

  mstch::node args_struct_name() {
    return service_name_ + "_" + function_->get_name() + "_args";
  }

  mstch::node result_struct_name() {
    return service_name_ + "_" + function_->get_name() + "_result";
  }

  mstch::node pargs_struct_name() {
    return service_name_ + "_" + function_->get_name() + "_pargs";
  }
};

/**
 * Context class for argument/result helper structs
 */
class t_function_helper_context : public mstch::object {
public:
  t_function_helper_context(const t_function* function, const std::string& service_name, const std::string& helper_type)
    : function_(function), service_name_(service_name), helper_type_(helper_type) {
    register_methods(this, std::map<std::string, mstch::node(t_function_helper_context::*)()>{
      {"name", &t_function_helper_context::name},
      {"fields", &t_function_helper_context::fields},
      {"has_fields", &t_function_helper_context::has_fields},
      {"equality_comparison", &t_function_helper_context::equality_comparison}
    });
  }

private:
  const t_function* function_;
  const std::string service_name_;
  const std::string helper_type_;

  mstch::node name() {
    return service_name_ + "_" + function_->get_name() + "_" + helper_type_;
  }

  mstch::node fields() {
    mstch::array result;
    
    if (helper_type_ == "args") {
      // For args struct, include function arguments
      const t_struct* args = function_->get_arglist();
      const vector<t_field*>& fields = args->get_members();
      for (size_t i = 0; i < fields.size(); ++i) {
        std::string struct_name = service_name_ + "_" + function_->get_name() + "_" + helper_type_;
        auto field_ctx = std::shared_ptr<t_field_context>(new t_field_context(fields[i], struct_name));
        // Set last flag for template usage
        field_ctx->set_last(i == fields.size() - 1);
        result.push_back(field_ctx);
      }
    } else if (helper_type_ == "result") {
      // For result struct, include return value and exceptions
      std::vector<std::shared_ptr<t_field_context>> field_contexts;
      
      if (!function_->get_returntype()->is_void()) {
        // Create synthetic field for return value - we need to be careful with memory management
        std::string struct_name = service_name_ + "_" + function_->get_name() + "_" + helper_type_;
        field_contexts.push_back(std::shared_ptr<t_field_context>(new t_field_context(nullptr, struct_name, "success", function_->get_returntype(), 0)));
      }
      
      // Add exception fields
      const t_struct* exceptions = function_->get_xceptions();
      const vector<t_field*>& exc_fields = exceptions->get_members();
      for (const auto& field : exc_fields) {
        std::string struct_name = service_name_ + "_" + function_->get_name() + "_" + helper_type_;
        field_contexts.push_back(std::shared_ptr<t_field_context>(new t_field_context(field, struct_name)));
      }
      
      // Set last flag
      for (size_t i = 0; i < field_contexts.size(); ++i) {
        field_contexts[i]->set_last(i == field_contexts.size() - 1);
        result.push_back(field_contexts[i]);
      }
    } else if (helper_type_ == "pargs") {
      // For pargs struct, include function arguments as pointers
      const t_struct* args = function_->get_arglist();
      const vector<t_field*>& fields = args->get_members();
      for (size_t i = 0; i < fields.size(); ++i) {
        std::string struct_name = service_name_ + "_" + function_->get_name() + "_" + helper_type_;
        auto field_ctx = std::shared_ptr<t_field_context>(new t_field_context(fields[i], struct_name, true)); // true = is_pointer
        // Set last flag for template usage
        field_ctx->set_last(i == fields.size() - 1);
        result.push_back(field_ctx);
      }
    } else if (helper_type_ == "presult") {
      // For presult struct, include return value and exceptions as pointers
      std::vector<std::shared_ptr<t_field_context>> field_contexts;
      
      if (!function_->get_returntype()->is_void()) {
        // Create synthetic field for return value - we need to be careful with memory management
        std::string struct_name = service_name_ + "_" + function_->get_name() + "_" + helper_type_;
        field_contexts.push_back(std::shared_ptr<t_field_context>(new t_field_context(nullptr, struct_name, "success", function_->get_returntype(), 0, true))); // true = is_pointer
      }
      
      // Add exception fields
      const t_struct* exceptions = function_->get_xceptions();
      const vector<t_field*>& exc_fields = exceptions->get_members();
      for (const auto& field : exc_fields) {
        std::string struct_name = service_name_ + "_" + function_->get_name() + "_" + helper_type_;
        field_contexts.push_back(std::shared_ptr<t_field_context>(new t_field_context(field, struct_name, true))); // true = is_pointer
      }
      
      // Set last flag
      for (size_t i = 0; i < field_contexts.size(); ++i) {
        field_contexts[i]->set_last(i == field_contexts.size() - 1);
        result.push_back(field_contexts[i]);
      }
    }
    
    return result;
  }

  mstch::node equality_comparison() {
    std::string result;
    
    if (helper_type_ == "args" || helper_type_ == "pargs") {
      const t_struct* args = function_->get_arglist();
      const vector<t_field*>& fields = args->get_members();
      for (size_t i = 0; i < fields.size(); ++i) {
        result += "this->" + fields[i]->get_name() + " == rhs." + fields[i]->get_name();
        if (i < fields.size() - 1) {
          result += " &&\n          ";
        }
      }
    } else if (helper_type_ == "result" || helper_type_ == "presult") {
      std::vector<std::string> field_names;
      
      if (!function_->get_returntype()->is_void()) {
        field_names.push_back("success");
      }
      
      const t_struct* exceptions = function_->get_xceptions();
      const vector<t_field*>& exc_fields = exceptions->get_members();
      for (const auto& field : exc_fields) {
        field_names.push_back(field->get_name());
      }
      
      for (size_t i = 0; i < field_names.size(); ++i) {
        result += "this->" + field_names[i] + " == rhs." + field_names[i];
        if (i < field_names.size() - 1) {
          result += " &&\n          ";
        }
      }
    }
    
    return result;
  }

  mstch::node has_fields() {
    if (helper_type_ == "args" || helper_type_ == "pargs") {
      return !function_->get_arglist()->get_members().empty();
    } else if (helper_type_ == "result" || helper_type_ == "presult") {
      return !function_->get_returntype()->is_void() || !function_->get_xceptions()->get_members().empty();
    }
    return false;
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
      {"extends", &t_service_context::extends},
      {"has_extends", &t_service_context::has_extends},
      {"extends_name", &t_service_context::extends_name},
      {"has_functions", &t_service_context::has_functions},
      {"function_helpers", &t_service_context::function_helpers}
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
      result.push_back(std::shared_ptr<mstch::object>(new t_function_context(func, service_->get_name())));
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

  mstch::node has_extends() {
    return service_->get_extends() != nullptr;
  }

  mstch::node extends_name() {
    const t_service* extends = service_->get_extends();
    if (extends) {
      return extends->get_name();
    }
    return "";
  }

  mstch::node has_functions() {
    return !service_->get_functions().empty();
  }

  mstch::node function_helpers() {
    mstch::array result;
    const vector<t_function*>& functions = service_->get_functions();
    
    for (const auto& func : functions) {
      // Add args struct
      result.push_back(std::shared_ptr<mstch::object>(
        new t_function_helper_context(func, service_->get_name(), "args")));
      
      // Add result struct
      result.push_back(std::shared_ptr<mstch::object>(
        new t_function_helper_context(func, service_->get_name(), "result")));
      
      // Add pargs struct (pointer arguments for serialization)
      result.push_back(std::shared_ptr<mstch::object>(
        new t_function_helper_context(func, service_->get_name(), "pargs")));
      
      // Add presult struct (pointer results for deserialization) 
      result.push_back(std::shared_ptr<mstch::object>(
        new t_function_helper_context(func, service_->get_name(), "presult")));
    }
    
    return result;
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
  bool file_exists(const std::string& filename);
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
  if (template_dir_.empty()) {
    return;
  }

  string service_name = tservice->get_name();
  
  // Create service header file using template
  string header_template_path = template_dir_ + "/service.h.mustache";
  string header_output_path = get_out_dir() + service_name + ".h";
  
  if (file_exists(header_template_path)) {
    std::ifstream template_file(header_template_path);
    if (template_file.is_open()) {
      std::string template_content((std::istreambuf_iterator<char>(template_file)),
                                   std::istreambuf_iterator<char>());
      template_file.close();

      // Create context for the service
      t_service_context service_context(tservice);
      mstch::map context_map;
      context_map["service"] = std::shared_ptr<mstch::object>(&service_context, [](mstch::object*) {});
      context_map["program"] = std::shared_ptr<mstch::object>(new t_program_context(program_));
      
      // Render template
      std::string rendered = mstch::render(template_content, context_map);
      
      // Write to output file
      std::ofstream output_file(header_output_path);
      if (output_file.is_open()) {
        output_file << rendered;
        output_file.close();
      }
    }
  }

  // Create service implementation file using template
  string impl_template_path = template_dir_ + "/service.cpp.mustache";
  string impl_output_path = get_out_dir() + service_name + ".cpp";
  
  if (file_exists(impl_template_path)) {
    std::ifstream template_file(impl_template_path);
    if (template_file.is_open()) {
      std::string template_content((std::istreambuf_iterator<char>(template_file)),
                                   std::istreambuf_iterator<char>());
      template_file.close();

      // Create context for the service
      t_service_context service_context(tservice);
      mstch::map context_map;
      context_map["service"] = std::shared_ptr<mstch::object>(&service_context, [](mstch::object*) {});
      context_map["program"] = std::shared_ptr<mstch::object>(new t_program_context(program_));
      
      // Render template
      std::string rendered = mstch::render(template_content, context_map);
      
      // Write to output file
      std::ofstream output_file(impl_output_path);
      if (output_file.is_open()) {
        output_file << rendered;
        output_file.close();
      }
    }
  }

  // Create service skeleton file using template
  string skeleton_template_path = template_dir_ + "/service_server.skeleton.cpp.mustache";
  string skeleton_output_path = get_out_dir() + service_name + "_server.skeleton.cpp";
  
  if (file_exists(skeleton_template_path)) {
    std::ifstream template_file(skeleton_template_path);
    if (template_file.is_open()) {
      std::string template_content((std::istreambuf_iterator<char>(template_file)),
                                   std::istreambuf_iterator<char>());
      template_file.close();

      // Create context for the service
      t_service_context service_context(tservice);
      mstch::map context_map;
      context_map["service"] = std::shared_ptr<mstch::object>(&service_context, [](mstch::object*) {});
      context_map["program"] = std::shared_ptr<mstch::object>(new t_program_context(program_));
      
      // Render template
      std::string rendered = mstch::render(template_content, context_map);
      
      // Write to output file
      std::ofstream output_file(skeleton_output_path);
      if (output_file.is_open()) {
        output_file << rendered;
        output_file.close();
      }
    }
  }
}

bool t_mustache_generator::file_exists(const std::string& filename) {
  std::ifstream file(filename);
  return file.good();
}

THRIFT_REGISTER_GENERATOR(
    mustache,
    "Mustache Template Generator",
    "    template_dir:    Directory containing mustache templates\n")