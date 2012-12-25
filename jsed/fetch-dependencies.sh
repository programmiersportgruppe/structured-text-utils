#!/bin/bash
# Script to fetch, make and install spidermonkey

set -x

mkdir deps
cd deps
wget http://ftp.mozilla.org/pub/mozilla.org/js/js185-1.0.0.tar.gz
status=$?
if [ $status -ne 0 ]; then
    echo "Failed to download spidermonkey. wget returned $status." >&2
    exit 1
fi

set -e
tar xzf js185-1.0.0.tar.gz

cd js-1.8.5/js/src

./configure
make all
sudo make install

cd ../../..
cd ..

