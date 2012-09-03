#!/bin/bash
set -e

echo "Running $0"

cd dist/build
rm -rf temp
mkdir temp
cd temp

# GIVEN
cat >letter.tpl <<'END'
Nada:$nada$
Name:$name$
Answer:$answer$
Pi:$pi$
Truth:$truth$
Falsity:$false$
Object Attribute:$object.attribute$
Nested Attribute:$object.nested.inner$
Numbers:$numbers$
Separated Numbers:$numbers;separator=", "$

$if(truth)$Yay!$else$Boo truth$endif$
$if(falsity)$Boo falsity$else$Yay!$endif$
END

# and
cat >letter.json <<'END'
{
    "nada":null,
    "name":"Felix",
    "answer":42,
    "pi":3.14159265359,
    "truth": true,
    "falsity": false,
    "object": {
        "attribute":"value",
        "nested":{
            "inner":"nice"
        }
    },
    "numbers": [1, 2, 3]
}
END

# WHEN
../stpl/stpl -t letter.tpl letter.json > output

# THEN

# NB: The expectation here is to see no output when boolean values are displayed.
#     This is consistent with the behaviour of hStringTemplate, but seems contrary
#     to the StringTemplate specification.
#     Should we try to fix this to display JSON-style "true" or "false"?
diff - output <<'END'
Nada:
Name:Felix
Answer:42
Pi:3.14159265359
Truth:
Falsity:
Object Attribute:value
Nested Attribute:nice
Numbers:123
Separated Numbers:1, 2, 3

Yay!
Yay!
END

cd ..
rm -rf temp
