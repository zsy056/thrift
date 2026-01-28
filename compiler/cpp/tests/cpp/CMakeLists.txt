#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements. See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership. The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the License for the
# specific language governing permissions and limitations
# under the License.
#

# C++ specific compile-check targets
# This ensures generated C++ code compiles correctly

if(NOT TARGET thrift-compiler)
    message(STATUS "Skipping C++ compile-checks (no thrift-compiler target)")
    return()
endif()

# Try to find Boost for the compile checks
find_package(Boost QUIET)
set(_compile_check_boost_include_dirs "")
if(Boost_FOUND)
    set(_compile_check_boost_include_dirs ${Boost_INCLUDE_DIRS})
elseif(DEFINED BOOST_ROOT)
    if(EXISTS "${BOOST_ROOT}/include/boost")
        set(_compile_check_boost_include_dirs "${BOOST_ROOT}/include")
    elseif(EXISTS "${BOOST_ROOT}/boost")
        set(_compile_check_boost_include_dirs "${BOOST_ROOT}")
    endif()
endif()

if(_compile_check_boost_include_dirs STREQUAL "")
    message(STATUS "Skipping C++ compile-checks (Boost headers not found)")
    return()
endif()

# Helper macro to create a compile-check target
macro(add_cpp_compile_check target_name thrift_file gen_options)
    set(_thrift_file "${CMAKE_CURRENT_SOURCE_DIR}/${thrift_file}")
    set(_gen_out_dir "${CMAKE_CURRENT_BINARY_DIR}/generated-${target_name}")
    set(_gen_cpp_dir "${_gen_out_dir}/gen-cpp")
    
    # Extract base name from thrift file
    get_filename_component(_thrift_basename ${thrift_file} NAME_WE)
    set(_types_cpp "${_gen_cpp_dir}/${_thrift_basename}_types.cpp")

    add_custom_command(
        OUTPUT "${_types_cpp}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_gen_out_dir}"
        COMMAND ${CMAKE_COMMAND} -E chdir "${_gen_out_dir}"
            $<TARGET_FILE:thrift-compiler>
            --gen cpp:${gen_options}
            -o "${_gen_out_dir}"
            "${_thrift_file}"
        DEPENDS thrift-compiler "${_thrift_file}"
        VERBATIM
    )

    set_source_files_properties(
        "${_types_cpp}"
        PROPERTIES GENERATED TRUE
    )

    add_library(thrift_compiler_generated_${target_name} STATIC
        "${_types_cpp}"
    )

    target_include_directories(thrift_compiler_generated_${target_name} PRIVATE
        "${_gen_cpp_dir}"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/cpp/src"
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_BINARY_DIR}"
        ${_compile_check_boost_include_dirs}
    )

    # Build the compile-check as part of the standard test build
    add_dependencies(thrift_compiler_tests thrift_compiler_generated_${target_name})
endmacro()

# Existing compile-check targets
add_cpp_compile_check(private_optional "test_private_optional.thrift" "private_optional")
add_cpp_compile_check(enum_class "test_enum_class.thrift" "pure_enums=enum_class")

# New compile-check targets for forward_setter
add_cpp_compile_check(moveable "test_forward_setter.thrift" "moveable_types")
add_cpp_compile_check(forward_setter "test_forward_setter.thrift" "moveable_types=forward_setter")

# Create a test source file to verify the forward_setter template instantiation
set(_forward_setter_test_cpp "${CMAKE_CURRENT_SOURCE_DIR}/test_forward_setter_usage.cpp")
if(EXISTS "${_forward_setter_test_cpp}")
    set(_forward_setter_gen_out_dir "${CMAKE_CURRENT_BINARY_DIR}/generated-forward_setter")
    set(_forward_setter_gen_cpp_dir "${_forward_setter_gen_out_dir}/gen-cpp")
    
    add_executable(test_forward_setter_usage
        "${_forward_setter_test_cpp}"
    )
    
    target_include_directories(test_forward_setter_usage PRIVATE
        "${_forward_setter_gen_cpp_dir}"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/cpp/src"
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_BINARY_DIR}"
        ${_compile_check_boost_include_dirs}
    )
    
    # Ensure the generated code is available before compiling the test
    add_dependencies(test_forward_setter_usage thrift_compiler_generated_forward_setter)
    
    # Link against the generated library
    target_link_libraries(test_forward_setter_usage 
        thrift_compiler_generated_forward_setter
    )
    
    # Add as a dependency of the test build
    add_dependencies(thrift_compiler_tests test_forward_setter_usage)
endif()
