#!/bin/bash

PREFIX=/usr/local

# Generate jsedjs.h
./convert-js-to-header.rb <jsed.js >jsedjs.h

g++ jsed.cpp -o jsed -L$PREFIX/lib -lmozjs185 -I$PREFIX/include/js
