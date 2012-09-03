#!/bin/bash
set -e

echo "Running $0"

cd dist/build
rm -rf temp
mkdir temp
cd temp

# GIVEN
cat >template.st <<'END'
This is it: $it$
END

# and
cat >model.json <<'END'
42
END

# WHEN
../stpl/stpl -t template.st model.json >output

# THEN
diff - output <<'END'
This is it: 42
END

cd ..
rm -rf temp
