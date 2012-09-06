#!/bin/bash
set -e

echo "Running $0"

cd dist/build
rm -rf temp
mkdir temp
cd temp

# GIVEN
cat >data.json <<'END'
{"name":"Mom"}
END

# WHEN
../stpl/stpl -t 'Look $name$, no template file!' data.json > output

# THEN
diff - output <<'END'
Look Mom, no template file!
END

cd ..
rm -rf temp
