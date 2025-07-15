#!/bin/bash

# Basic integration test for mustache generator
# This test verifies that the mustache generator can be invoked and produces output

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
THRIFT_BIN="$BUILD_DIR/compiler/cpp/bin/thrift"

# Check if thrift binary exists
if [ ! -f "$THRIFT_BIN" ]; then
    echo "ERROR: Thrift binary not found at $THRIFT_BIN"
    exit 1
fi

# Create temporary test directory
TEST_DIR=$(mktemp -d)
cd "$TEST_DIR"

# Create a simple test thrift file
cat > test.thrift << 'EOF'
enum Color {
  RED = 1,
  GREEN = 2,
  BLUE = 3
}

struct Point {
  1: i32 x,
  2: i32 y
}

service Calculator {
  i32 add(1: i32 a, 2: i32 b),
  Point getOrigin()
}
EOF

# Create basic mustache templates
mkdir -p templates

cat > templates/enum.mustache << 'EOF'
// Generated enum {{name}}
enum {{name}} {
{{#values}}
  {{name}} = {{value}},
{{/values}}
};
EOF

cat > templates/struct.mustache << 'EOF'
// Generated struct {{name}}
struct {{name}} {
{{#fields}}
  {{#type}}{{name}}{{/type}} {{name}};
{{/fields}}
};
EOF

cat > templates/service.mustache << 'EOF'
// Generated service {{name}}
class {{name}} {
public:
{{#functions}}
  {{#return_type}}{{name}}{{/return_type}} {{name}}();
{{/functions}}
};
EOF

# Test 1: Check that mustache generator is available
echo "Testing mustache generator availability..."
if ! "$THRIFT_BIN" --help 2>&1 | grep -q "mustache (Mustache Template Generator)"; then
    echo "ERROR: Mustache generator not found in thrift help"
    exit 1
fi

# Test 2: Generate files using mustache generator
echo "Testing mustache generator code generation..."
"$THRIFT_BIN" --gen mustache:template_dir=templates test.thrift

# Test 3: Check that output files were created
if [ ! -d "gen-mustache-cpp" ]; then
    echo "ERROR: Output directory gen-mustache-cpp was not created"
    exit 1
fi

if [ ! -f "gen-mustache-cpp/Color.h" ]; then
    echo "ERROR: Color.h was not generated"
    exit 1
fi

if [ ! -f "gen-mustache-cpp/Point.h" ]; then
    echo "ERROR: Point.h was not generated"
    exit 1
fi

if [ ! -f "gen-mustache-cpp/Calculator.h" ]; then
    echo "ERROR: Calculator.h was not generated"
    exit 1
fi

# Test 4: Check content of generated files
echo "Testing generated file content..."

if ! grep -q "enum Color" gen-mustache-cpp/Color.h; then
    echo "ERROR: Color.h does not contain expected enum declaration"
    exit 1
fi

if ! grep -q "RED = 1" gen-mustache-cpp/Color.h; then
    echo "ERROR: Color.h does not contain expected enum value"
    exit 1
fi

if ! grep -q "struct Point" gen-mustache-cpp/Point.h; then
    echo "ERROR: Point.h does not contain expected struct declaration"
    exit 1
fi

if ! grep -q "class Calculator" gen-mustache-cpp/Calculator.h; then
    echo "ERROR: Calculator.h does not contain expected service class"
    exit 1
fi

# Clean up
cd /
rm -rf "$TEST_DIR"

echo "SUCCESS: All mustache generator tests passed!"