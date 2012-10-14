#!/bin/sh
# Compiling and testing stpl

set -e

cabal clean
cabal configure
cabal build

# Hlint if available
if which -s hlint
then
    echo "Linting"
    hlint . --ignore="Use string literal"
    hlint . --hint=Dollar
fi

echo


./test.sh
