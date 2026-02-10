# CMake generated Testfile for 
# Source directory: /home/runner/work/thrift/thrift/test/cpp
# Build directory: /home/runner/work/thrift/thrift/build_full/test/cpp
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(StressTest "/home/runner/work/thrift/thrift/build_full/bin/StressTest")
set_tests_properties(StressTest PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;79;add_test;/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;0;")
add_test(StressTestConcurrent "/home/runner/work/thrift/thrift/build_full/bin/StressTest" "--client-type=concurrent")
set_tests_properties(StressTestConcurrent PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;80;add_test;/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;0;")
add_test(StressTestNonBlocking "/home/runner/work/thrift/thrift/build_full/bin/StressTestNonBlocking")
set_tests_properties(StressTestNonBlocking PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;89;add_test;/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;0;")
add_test(SpecificNameTest "/home/runner/work/thrift/thrift/build_full/bin/SpecificNameTest")
set_tests_properties(SpecificNameTest PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;96;add_test;/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;0;")
add_test(ForwardSetterTest "/home/runner/work/thrift/thrift/build_full/bin/ForwardSetterTest")
set_tests_properties(ForwardSetterTest PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;119;add_test;/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;0;")
add_test(PrivateOptionalTest "/home/runner/work/thrift/thrift/build_full/bin/PrivateOptionalTest")
set_tests_properties(PrivateOptionalTest PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;142;add_test;/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;0;")
add_test(EnumClassTest "/home/runner/work/thrift/thrift/build_full/bin/EnumClassTest")
set_tests_properties(EnumClassTest PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;165;add_test;/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;0;")
add_test(TemplateStreamOpTest "/home/runner/work/thrift/thrift/build_full/bin/TemplateStreamOpTest")
set_tests_properties(TemplateStreamOpTest PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;188;add_test;/home/runner/work/thrift/thrift/test/cpp/CMakeLists.txt;0;")
