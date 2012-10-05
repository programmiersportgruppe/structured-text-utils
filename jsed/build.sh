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

#Test should be able to transform json document that comes on a single line.
INPUT='{"x":"value"}'
OUTPUT=`echo $INPUT | ./jsed 'function(x) x.x'`
if [ "$OUTPUT" = '"value"' ]; then
    echo "TEST PASSED"
else
    echo "TEST FAILED: Expected \"value\" but got $OUTPUT"
fi

#Test should be able to transform a "pretty" json document.

INPUT=$(cat <<'EOF'
{
    "x":"value"
}
EOF
)

OUTPUT=`echo $INPUT | ./jsed 'function(x) x.x'`
if [ "$OUTPUT" = '"value"' ]; then
    echo "TEST PASSED"
else
    echo "TEST FAILED: Expected \"value\" but got $OUTPUT"
fi

#Test should deal with newline separated json documents in multi document mode

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
if [ "$OUTPUT" = "$EXPECTED" ]; then
    echo "TEST PASSED"
else
    echo "TEST FAILED: Expected \"value\" but got $OUTPUT"
fi




