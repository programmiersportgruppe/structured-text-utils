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

# NB: The expectation here is to see no output when boolean values are displayed.
#     This is consistent with the behaviour of hStringTemplate, but seems contrary
#     to the StringTemplate specification.
#     Should we try to fix this to display JSON-style "true" or "false"?
diff - output <<'END'
{"flat-simple":"flat","flat-two-lines":"Line One\nLine Two","nested":{"a":"aValue","b":"bValue"}}
END

cd ..
rm -rf temp

