#cleanup

rm -f js.o

STATIC_PREFIX=../deps/js-1.8.5/js/src/dist
PREFIX=/usr/local



if [ "$*" = "--static" ]
then
    if [ ! -d "$STATIC_PREFIX/include" ]; then
        echo "Static version of spidermonkey missing. Try running fetch-dependencies.sh" >&2
        exit 1;
    fi
    g++ -c js.cpp -I$STATIC_PREFIX/include
else
    g++ -c js.cpp -I$PREFIX/include/js
fi

