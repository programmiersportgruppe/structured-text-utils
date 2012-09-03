#!/bin/sh
# Compiling and testing stpl

set -e

cabal clean
cabal configure
cabal build

echo
./test.sh
