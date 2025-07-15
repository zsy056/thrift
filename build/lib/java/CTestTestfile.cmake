# CMake generated Testfile for 
# Source directory: /home/runner/work/thrift/thrift/lib/java
# Build directory: /home/runner/work/thrift/thrift/build/lib/java
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(JavaTest "/usr/bin/gradle" "test" "--console=plain" "--no-daemon" "-Prelease=true" "-Pthrift.version=0.23.0" "-Pbuild.dir=/home/runner/work/thrift/thrift/build/lib/java/build" "-Pthrift.compiler=/home/runner/work/thrift/thrift/build/compiler/cpp/bin/thrift")
set_tests_properties(JavaTest PROPERTIES  WORKING_DIRECTORY "/home/runner/work/thrift/thrift/lib/java" _BACKTRACE_TRIPLES "/home/runner/work/thrift/thrift/lib/java/CMakeLists.txt;91;add_test;/home/runner/work/thrift/thrift/lib/java/CMakeLists.txt;0;")
