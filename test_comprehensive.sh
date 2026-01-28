#!/bin/bash
# Comprehensive test suite for print_ostream option

set -e

echo "========================================="
echo "Print OStream Comprehensive Test Suite"
echo "========================================="
echo ""

# Test 1: Generate with print_ostream
echo "Test 1: Generate code with templates=print_ostream"
./compiler/cpp/thrift --gen cpp:templates=print_ostream test/PrintOstreamTest.thrift
if [ $? -eq 0 ]; then
  echo "✓ Code generation successful"
else
  echo "✗ Code generation failed"
  exit 1
fi

# Test 2: Verify templated declarations
echo ""
echo "Test 2: Verify templated declarations in header"
if grep -q "template<typename OStream>" gen-cpp/PrintOstreamTest_types.h && \
   grep -q "void printTo(OStream& out) const" gen-cpp/PrintOstreamTest_types.h && \
   grep -q "OStream& operator<<(OStream& out, const" gen-cpp/PrintOstreamTest_types.h; then
  echo "✓ Templated declarations found"
else
  echo "✗ Templated declarations not found"
  exit 1
fi

# Test 3: Verify implementations in .tcc
echo ""
echo "Test 3: Verify implementations in .tcc file"
if [ -f "gen-cpp/PrintOstreamTest_types.tcc" ] && \
   grep -q "template<typename OStream>" gen-cpp/PrintOstreamTest_types.tcc && \
   grep -q "void SimpleStruct::printTo(OStream& out)" gen-cpp/PrintOstreamTest_types.tcc; then
  echo "✓ Implementations in .tcc file"
else
  echo "✗ Implementations not found in .tcc"
  exit 1
fi

# Test 4: Verify .tcc is included
echo ""
echo "Test 4: Verify .tcc is included in header"
if grep -q '#include "PrintOstreamTest_types.tcc"' gen-cpp/PrintOstreamTest_types.h; then
  echo "✓ .tcc included in header"
else
  echo "✗ .tcc not included in header"
  exit 1
fi

# Test 5: Verify no 'virtual' keyword with print_ostream
echo ""
echo "Test 5: Verify no 'virtual' keyword before template"
if ! grep -q "virtual.*template<typename OStream>" gen-cpp/PrintOstreamTest_types.h; then
  echo "✓ No 'virtual' before template (correct)"
else
  echo "✗ Found 'virtual' before template (incorrect)"
  exit 1
fi

# Test 6: Normal generation still works
echo ""
echo "Test 6: Verify normal generation (backward compatibility)"
mkdir -p /tmp/test_backward
./compiler/cpp/thrift --gen cpp -o /tmp/test_backward test/PrintOstreamTest.thrift
if grep -q "virtual void printTo(std::ostream& out)" /tmp/test_backward/gen-cpp/PrintOstreamTest_types.h && \
   ! grep -q "template<typename OStream>" /tmp/test_backward/gen-cpp/PrintOstreamTest_types.h && \
   ! [ -f "/tmp/test_backward/gen-cpp/PrintOstreamTest_types.tcc" ]; then
  echo "✓ Normal generation works (no templates, has virtual, no .tcc)"
else
  echo "✗ Normal generation changed"
  exit 1
fi

# Test 7: templates=only still works
echo ""
echo "Test 7: Verify templates=only mode (backward compatibility)"
mkdir -p /tmp/test_templates_only
./compiler/cpp/thrift --gen cpp:templates=only -o /tmp/test_templates_only test/PrintOstreamTest.thrift
if grep -q "void printTo(std::ostream& out)" /tmp/test_templates_only/gen-cpp/PrintOstreamTest_types.h && \
   ! grep -q "template<typename OStream>" /tmp/test_templates_only/gen-cpp/PrintOstreamTest_types.h && \
   [ -f "/tmp/test_templates_only/gen-cpp/PrintOstreamTest_types.tcc" ]; then
  echo "✓ templates=only works (std::ostream, has .tcc for read/write)"
else
  echo "✗ templates=only changed"
  exit 1
fi

# Test 8: Check nested structs
echo ""
echo "Test 8: Verify nested structs also templated"
if grep -q "void NestedStruct::printTo(OStream& out)" gen-cpp/PrintOstreamTest_types.tcc; then
  echo "✓ Nested struct printTo is templated"
else
  echo "✗ Nested struct not templated"
  exit 1
fi

echo ""
echo "========================================="
echo "✓ All tests passed!"
echo "========================================="
echo ""
echo "Summary of generated code:"
echo "--------------------------"
echo "Generated files:"
ls -lh gen-cpp/PrintOstreamTest_types.*
echo ""
echo "Header declares (sample):"
grep -A 1 "template<typename OStream>" gen-cpp/PrintOstreamTest_types.h | head -6
echo ""
echo "TCC implements (sample):"
grep -A 6 "void SimpleStruct::printTo" gen-cpp/PrintOstreamTest_types.tcc | head -7
