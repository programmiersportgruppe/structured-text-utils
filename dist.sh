#!/usr/bin/env bash
#
# builds all projects and creates distribution archive

set -e

VERSION=0.1
PLATFORM="`uname`-`uname -m`"
ARCHIVE_NAME=structured-text-utils-$VERSION-$PLATFORM

cd jsed
./build.sh
cd ..

cd stpl
./build.sh
cd ..

pwd

mkdir -p $ARCHIVE_NAME/bin
cp jsed/jsed $ARCHIVE_NAME/bin
cp stpl/dist/build/stpl/stpl $ARCHIVE_NAME/bin

cp README.md $ARCHIVE_NAME

tar cvfj $ARCHIVE_NAME.tar.bz2 $ARCHIVE_NAME

rm -rf $ARCHIVE_NAME



