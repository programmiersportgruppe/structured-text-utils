#!/bin/bash
set -e

echo "Running $0"

cd dist/build
rm -rf temp
mkdir temp
cd temp

# GIVEN
cat >arrays.yaml<<'END'
string-array:
    - one
    - two
    - three
END


# WHEN
../yaml2json/yaml2json < arrays.yaml > output

# THEN

diff - output <<'END'
{"string-array":["one","two","three"]}
END

cd ..
rm -rf temp
