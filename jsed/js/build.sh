#cleanup

rm -f js.o

STATIC_PREFIX=../deps/js-1.8.5/js/src/dist
PREFIX=/usr/local

CPP=${CPP:-g++}
export CPP


if [ "$*" = "--static" ]
then
    if [ ! -d "$STATIC_PREFIX/include" ]; then
        echo "Static version of spidermonkey missing. Try running fetch-dependencies.sh" >&2
        exit 1;
    fi
    $CPP -c js.cpp -I$STATIC_PREFIX/include
else
    $CPP -c js.cpp -I$PREFIX/include/js
fi

