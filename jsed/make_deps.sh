#!/bin/sh
set -e
set -u

mkdir -p deps
cd deps

install_autoconf_213() {
    if which -s autoconf-2.13; then
        autoconf_213="autoconf-2.13"
    elif which -s autoconf && autoconf --version | grep --quiet 'version 2.13'; then
        autoconf_213="autoconf"
    else
        if ! [ -x "autoconf-2.13/autoconf" ]; then
            echo "Getting you some Autoconf version 2.13..."
            if ! [ -d "autoconf-2.13" ]; then
                test -f autoconf-2.13.tar.gz || wget http://ftp.gnu.org/gnu/autoconf/autoconf-2.13.tar.gz
                tar xzf autoconf-2.13.tar.gz
            fi
            cd autoconf-2.13
            ./configure
            make
            cd ..
        fi
        autoconf_213="../../../autoconf-2.13/autoconf --macrodir=../../../autoconf-2.13"
    fi
    echo "Using $autoconf_213 (relative to deps/js-1.8.5/js/src)"
}

lib_file=js-1.8.5/js/src/dist/lib/libjs_static.a
install_libjs() {
    if [ -f "$lib_file" ]; then
        echo "libjs available at deps/$lib_file"
        return
    fi

    install_autoconf_213

    echo "Getting you some libjs (SpiderMonkey) version 1.8.5..."
    if ! [ -d "js-1.8.5" ]; then
        test -f js185-1.0.0.tar.gz || wget http://ftp.mozilla.org/pub/mozilla.org/js/js185-1.0.0.tar.gz
        tar xzf js185-1.0.0.tar.gz
    fi
    cd js-1.8.5/js/src
    $autoconf_213

    # https://developer.mozilla.org/en-US/docs/SpiderMonkey/Build_Documentation#Building_SpiderMonkey_as_a_static_library
    ./configure --disable-shared-js

    make

    if [ -f "$lib_file" ]; then
        echo "Static lib successfully available"
    else
        echo "Error building static lib!"
        exit 1
    fi

    cd ../../..
}

install_libjs
