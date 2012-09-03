#!/bin/sh
# Testing stpl

status="PASSED"
for t in tests/*.sh
do
    $t || status="FAILED"
done

echo "\nTEST $status"
if [ "$status" != "PASSED" ]; then exit 1; fi
