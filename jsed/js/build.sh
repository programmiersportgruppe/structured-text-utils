#cleanup

rm -f js.o

STATIC_PREFIX=../deps/js-1.8.5/js/src/dist
PREFIX=/usr/local

if [ "$*" = "--static" ]
then
    g++ -c js.cpp -I$STATIC_PREFIX/include
else
    g++ -I$STATIC_PREFIX/include -c js.cpp -I$PREFIX/include/js
fi

