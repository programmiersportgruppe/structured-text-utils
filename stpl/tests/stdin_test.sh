#!/bin/bash
set -e

echo "Running $0"

cd dist/build
rm -rf temp
mkdir temp
cd temp

# GIVEN
cat >template.st <<'END'
Hello $name$!
END

# WHEN
echo '{"name": "World"}' | ../stpl/stpl -t template.st >output

# THEN
diff - output <<'END'
Hello World!
END

cd ..
rm -rf temp
