#!/bin/bash

PREFIX=/usr/local

g++ jsed.cpp -o jsed -L$PREFIX/lib -lmozjs185 -I$PREFIX/include/js
