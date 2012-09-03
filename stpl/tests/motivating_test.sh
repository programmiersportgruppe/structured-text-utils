#!/bin/bash
set -e

echo "Running $0"

cd dist/build
rm -rf temp
mkdir temp
cd temp

# GIVEN
cat >letter.tpl <<'END'
Dear $name$,

$body$

Kind regards,
Felix
END

# and
cat >letter.json <<'END'
{
   "name":"Ben",
   "body":"I would love to make you do some work for me."
}
END

# WHEN
../stpl/stpl -t letter.tpl letter.json > output

# THEN
diff - output <<'END'
Dear Ben,

I would love to make you do some work for me.

Kind regards,
Felix
END

cd ..
rm -rf temp
