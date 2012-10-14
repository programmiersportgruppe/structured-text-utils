#!/bin/bash
set -e

echo "Running $0"

cd dist/build
rm -rf temp
mkdir temp
cd temp

# GIVEN
cat >nested.yaml<<'END'
flat-simple: flat
flat-two-lines: |
                 Line One
                 Line Two
nested:
    a: aValue
    b: bValue
END


# WHEN
../yaml2json/yaml2json < nested.yaml > output

# THEN

diff - output <<'END'
{"flat-simple":"flat","flat-two-lines":"Line One\nLine Two\n","nested":{"a":"aValue","b":"bValue"}}
END

cd ..
rm -rf temp

