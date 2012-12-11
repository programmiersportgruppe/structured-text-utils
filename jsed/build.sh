#!/bin/bash
# Compiling and testing jsed
set -e

echo Building jsed

PREFIX=/usr/local
STATIC_PREFIX=deps/js-1.8.5/js/src/dist

rm -f jsed
rm -f jsedjs.h

# Generate jsedjs.h
./convert-js-to-header.rb <jsed.js >jsedjs.h

if [ "$*" = "--static" ]
then
    g++ jsed.cpp -o jsed -I$STATIC_PREFIX/include -Wl,$STATIC_PREFIX/lib/libjs_static.a
else
    g++ jsed.cpp -o jsed -L$PREFIX/lib -lmozjs185 -I$PREFIX/include/js
fi

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
OUTPUT=`echo $INPUT | ./jsed 'function(x) x.x'`

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

OUTPUT=`echo "$INPUT" | ./jsed --pretty 'function(x) x.map(function(e) e.toUpperCase())'`

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


