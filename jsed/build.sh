#!/bin/bash
# Compiling and testing jsed

set -e

PREFIX=/usr/local


rm jsed
rm jsedjs.h

# Generate jsedjs.h
./convert-js-to-header.rb <jsed.js >jsedjs.h

g++ jsed.cpp -o jsed -L$PREFIX/lib -lmozjs185 -I$PREFIX/include/js

INPUT='{"x":"value"}'
OUTPUT=`echo $INPUT | ./jsed 'function(x) x.x'`
if [ $OUTPUT == "\"value\"" ]; then
    echo TEST PASSED
else
    echo TEST FAILED
fi

