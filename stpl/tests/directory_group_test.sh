#!/bin/bash
set -e

echo "Running $0"

cd dist/build
rm -rf temp
mkdir temp
cd temp

# GIVEN
mkdir somedir

# and
cat >somedir/A.st <<'END'
This is template A containing template B: $B()$
END

# and
cat >somedir/B.st <<'END'
This is template B: $foo$
END

# and
cat >model.json <<'END'
{
    "foo":"hi"
}
END

# WHEN
../stpl/stpl -g somedir -n A model.json >output

# THEN
diff - output <<'END'
This is template A containing template B: This is template B: hi
END

cd ..
rm -rf temp
