#/usr/bin/env bash

echo Compiling
set -e
g++ -c filter.cpp
g++ filter-test.cpp filter.o -o tester
set +e

echo Testing

function assertEq() {
    ASSERTION_DESC=$1
    EXPECTED_OUTPUT=$2
    ACTUAL_OUTPUT=$3

    red='\033[31m'
    green='\033[32m'
    reset='\033[0m'

    if [ "$EXPECTED_OUTPUT" = "$ACTUAL_OUTPUT" ]; then
        echo -e "${green}PASSED${reset} $ASSERTION_DESC"
    else
        echo -e "${red}FAILED${reset} $ASSERTION_DESC: Expected $EXPECTED_OUTPUT, but got $ACTUAL_OUTPUT"
    fi
}


x=$(echo hello | ./tester grep ello)
exitStatus=$?
assertEq "input is passed through to output"  "hello" "$x"
assertEq "exit status is set to 0 on success" "0" "$exitStatus"

x=$(echo hello | ./tester ls iamnotafile)
exitStatus=$?
assertEq "exit status is set to 1 on failure" "1" "$exitStatus"

