#!/bin/bash
# Simple test to verify generated code with print_ostream option compiles

set -e

echo "Testing print_ostream code generation..."

# Generate code
./compiler/cpp/thrift --gen cpp:templates=print_ostream test/PrintOstreamTest.thrift

echo "✓ Code generation successful"

# Check that .tcc file was created
if [ ! -f "gen-cpp/PrintOstreamTest_types.tcc" ]; then
  echo "✗ ERROR: .tcc file was not generated"
  exit 1
fi

echo "✓ .tcc file generated"

# Check that printTo is templated in the header
if grep -q "template<typename OStream>" gen-cpp/PrintOstreamTest_types.h; then
  echo "✓ Templated printTo declaration found in header"
else
  echo "✗ ERROR: Templated printTo not found in header"
  exit 1
fi

# Check that printTo implementation is in .tcc
if grep -q "template<typename OStream>" gen-cpp/PrintOstreamTest_types.tcc && \
   grep -q "void SimpleStruct::printTo" gen-cpp/PrintOstreamTest_types.tcc; then
  echo "✓ Templated printTo implementation found in .tcc"
else
  echo "✗ ERROR: Templated printTo implementation not found in .tcc"
  exit 1
fi

# Check that operator<< is templated
if grep -q "template<typename OStream>" gen-cpp/PrintOstreamTest_types.tcc && \
   grep -q "OStream& operator<<" gen-cpp/PrintOstreamTest_types.tcc; then
  echo "✓ Templated operator<< found in .tcc"
else
  echo "✗ ERROR: Templated operator<< not found in .tcc"
  exit 1
fi

# Check that .tcc is included in the header
if grep -q '#include "PrintOstreamTest_types.tcc"' gen-cpp/PrintOstreamTest_types.h; then
  echo "✓ .tcc file is included in header"
else
  echo "✗ ERROR: .tcc file not included in header"
  exit 1
fi

echo ""
echo "All validation checks passed!"
echo ""
echo "Sample generated code:"
echo "======================"
echo ""
echo "From PrintOstreamTest_types.h:"
grep -A 2 "template<typename OStream>" gen-cpp/PrintOstreamTest_types.h | head -6
echo ""
echo "From PrintOstreamTest_types.tcc:"
grep -A 8 "void SimpleStruct::printTo" gen-cpp/PrintOstreamTest_types.tcc | head -9
