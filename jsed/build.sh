#!/bin/bash
# Compiling and testing jsed
set -e

echo "Building jsed"
echo "============="
PREFIX=/usr/local
STATIC_PREFIX=deps/js-1.8.5/js/src/dist

# To support c++11 uncomment the following lines
#CPP=${CPP:-g++-mp-4.7}
#CPP_FLAGS=${CPP_FLAGS:--std=c++11}

CPP=${CPP:-g++}
CPP_FLAGS=${CPP_FLAGS:-}

export CPP_FLAGS
export CPP

# cleanup

rm -f jsed
rm -f jsedjs.h
echo ""
echo "Building filter module"
echo "----------------------"

set +e
(cd filter && ./build.sh && cd ..)
status=$?
if [ $status -ne 0 ]; then
  echo "Failed to build filter module" >&2
  exit 1
fi
set -e
echo

echo ""
echo "Building js wrapper module"
echo "--------------------------"

set +e
(cd js && ./build.sh "$*" && cd ..)
status=$?
if [ $status -ne 0 ]; then
  echo "Failed to build js wrapper module" >&2
  exit 1
fi
set -e
echo



echo Compiling jsed
echo "--------------"
# Generate jsedjs.h
./convert-js-to-header.rb <jsed.js >jsedjs.h


if [ "$*" = "--static" ]
then
    $CPP $CPP_FLAGS filter/filter.o js/js.o jsed.cpp  -I$STATIC_PREFIX/include -o jsed -Wl,$STATIC_PREFIX/lib/libjs_static.a
else
    $CPP $CPP_FLAGS filter/filter.o js/js.o jsed.cpp -o jsed -I$PREFIX/include/js -L$PREFIX/lib -lmozjs185
fi

echo Testing

function assertEq() {
    ASSERTION_DESC=$1
    EXPECTED_OUTPUT=$2
    ACTUAL_OUTPUT=$3

    red='\033[31m'
    green='\033[32m'
    reset='\033[0m'

    if [ "$EXPECTED_OUTPUT" = "$ACTUAL_OUTPUT" ]; then
        echo -e "${green}PASSED${reset} $ASSERTION_DESC"
    else
        echo -e "${red}FAILED${reset} $ASSERTION_DESC : Expected $EXPECTED_OUTPUT but got $ACTUAL_OUTPUT"
    fi
}


#Test should be able to transform json document that comes on a single line.
INPUT='{"x":"value"}'
set +e
OUTPUT=`echo $INPUT | ./jsed 'function(x) x.x'`
status=$?
if [ $status -ne 0 ]; then
  echo "Failed test" >&2
  echo $OUTPUT
  exit 2
fi
set -e


assertEq "transform single line json" \
         '"value"' "$OUTPUT"

#Test should be able to transform a "pretty" json document.

INPUT=$(cat <<'EOF'
{
    "x":"value"
}
EOF
)

OUTPUT=`echo $INPUT | ./jsed 'function(x) x.x'`
assertEq "transform 'pretty' json document" \
         '"value"' $OUTPUT


INPUT=$(cat <<'EOF'
{ "x":"value1" }
{ "x":"value2" }
EOF
)

EXPECTED=$(cat <<'EOF'
"value1"
"value2"
EOF
)

OUTPUT=`echo "$INPUT" | ./jsed -m 'function(x) x.x'`

assertEq "transform multiple, newline separated json documents" \
         "$EXPECTED" "$OUTPUT"


INPUT=$(cat <<'EOF'
["Hello","brave","new","World"]
EOF
)

EXPECTED=$(cat <<'EOF'
Hello
brave
new
World
EOF
)

OUTPUT=`echo "$INPUT" | ./jsed --raw  'function(x) x.join("\\n")'`

assertEq "transform using the raw output feature" \
         "$EXPECTED" "$OUTPUT"


# Test the pretty output feature


INPUT=$(cat <<'EOF'
{"k":"v"}
EOF
)

EXPECTED=$(cat <<'EOF'
{
    "k": "v"
}
EOF
)

OUTPUT=`echo "$INPUT" | ./jsed --pretty  'function(x) x'`

assertEq "produce pretty output" \
         "$EXPECTED" "$OUTPUT"



# Test the transformation from file feature

INPUT=$(cat <<'EOF'
{}
EOF
)

EXPECTED=$(cat <<'EOF'
{
    "name": "felix"
}
EOF
)

OUTPUT=`echo "$INPUT" | ./jsed --pretty -f test.js`

assertEq "transformation from file" \
         "$EXPECTED" "$OUTPUT"


# Map object

INPUT=$(cat <<'EOF'
{ "name": "felix", "occupation": "developer"}
EOF
)

EXPECTED=$(cat <<'EOF'
{
    "name": "FELIX",
    "occupation": "DEVELOPER"
}
EOF
)

OUTPUT=`echo "$INPUT" | ./jsed --pretty 'function(x) x.map(function(k, v) v.toUpperCase())'`

assertEq "support mapping of object properties" \
         "$EXPECTED" "$OUTPUT"


# Filter object by key

INPUT=$(cat <<'EOF'
{ "name": "felix", "occupation": "developer"}
EOF
)

EXPECTED=$(cat <<'EOF'
{
    "name": "felix"
}
EOF
)

OUTPUT=`echo "$INPUT" | ./jsed --pretty 'function(x) x.filter(function(k) k === "name")'`

assertEq "support filtering of object properties by key" \
         "$EXPECTED" "$OUTPUT"

# filter object by value

INPUT=$(cat <<'EOF'
{ "name": "felix", "occupation": "developer"}
EOF
)

EXPECTED=$(cat <<'EOF'
{
    "name": "felix"
}
EOF
)

OUTPUT=`echo "$INPUT" | ./jsed --pretty 'function(x) x.filter(function(k, v) v === "felix")'`

assertEq "support filtering of object properties by value" \
         "$EXPECTED" "$OUTPUT"

# Inplace editing

$(cat >tmp.json<<'EOF'
{ "name": "felix", "occupation": "developer"}
EOF
)

EXPECTED=$(cat <<'EOF'
{
    "name": "felix"
}
EOF
)

./jsed --in-place tmp.json --pretty 'function(x) x.filter(function(k, v) v === "felix")'

OUTPUT=`cat tmp.json`

assertEq "support in place editing" \
         "$EXPECTED" "$OUTPUT"


# pipe data through external tool
INPUT=$(cat <<'EOF'
{"numbers":"3\n1\n2"}
EOF
)

EXPECTED=$(cat <<'EOF'
1
2
3
EOF
)

OUTPUT=`echo "$INPUT" | ./jsed --raw 'function(x) "2\\n3\\n1".pipe("sort", ["-n"])'`

assertEq "pipe data through external tool" \
         "$EXPECTED" "$OUTPUT"
