# CMake generated Testfile for 
# Source directory: /home/runner/work/thrift/thrift/test/py
# Build directory: /home/runner/work/thrift/thrift/build/test/py
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(python_test_generate "/usr/local/bin/cmake" "-DTHRIFTCOMPILER=/home/runner/work/thrift/thrift/build/compiler/cpp/bin/thrift" "-DMY_PROJECT_DIR=/home/runner/work/thrift/thrift" "-DMY_CURRENT_SOURCE_DIR=/home/runner/work/thrift/thrift/test/py" "-DMY_CURRENT_BINARY_DIR=/home/runner/work/thrift/thrift/build/test/py" "-P" "/home/runner/work/thrift/thrift/test/py/generate.cmake")
set_tests_properties(python_test_generate PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/thrift/thrift/test/py/CMakeLists.txt;20;add_test;/home/runner/work/thrift/thrift/test/py/CMakeLists.txt;0;")
add_test(python_test "/usr/bin/python" "/home/runner/work/thrift/thrift/test/py/RunClientServer.py" "--gen-base=/home/runner/work/thrift/thrift/build/test/py" "DEPENDS" "python_test_generate")
set_tests_properties(python_test PROPERTIES  WORKING_DIRECTORY "/home/runner/work/thrift/thrift/test/py" _BACKTRACE_TRIPLES "/home/runner/work/thrift/thrift/test/py/CMakeLists.txt;29;add_test;/home/runner/work/thrift/thrift/test/py/CMakeLists.txt;0;")
