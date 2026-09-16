#!/bin/bash

# This file is licensed under the MIT License. See LICENSE for details.
#
# Usage:
#   ./verify_each_test_case.sh                          # walk every test in every category
#   ./verify_each_test_case.sh --through 1              # cumulative through Assignment 1 (arithmetic-vars)
#   ./verify_each_test_case.sh --through 2              # cumulative through Assignment 2 (+ control-flow)
#   ./verify_each_test_case.sh --through 3              # cumulative through Assignment 3 (+ pass-int)
#   ./verify_each_test_case.sh --through 4              # cumulative through Assignment 4 (+ call-int)
#   ./verify_each_test_case.sh --through 5              # cumulative through Assignment 5 (liveness; same as 4)
#   ./verify_each_test_case.sh --through 6              # cumulative through Assignment 6 (optimizations; same as 4)
#   ./verify_each_test_case.sh --through 7              # cumulative through Assignment 7 (+ oo-int)
#   ./verify_each_test_case.sh arithmetic-vars          # substring filter
#   ./verify_each_test_case.sh --through 2 dowhile      # combined
#
# Each test pauses for a keypress so you can inspect the output.
#
# For every test the Makefile builds BOTH backends and checks that each
# produces the correct answer: the x86-64 binary AND the llc-compiled LLVM
# binary. Any failure — a build error, an llc/LLVM syntax error, or either
# backend giving the wrong answer — stops this script immediately.

set -e
trap 'echo; echo " ***** FAILURE - stopping (see output above) ***** "; echo' ERR

THROUGH=""
FILTER=""
while [ $# -gt 0 ]; do
    case "$1" in
        --through)   THROUGH="$2"; shift 2 ;;
        --through=*) THROUGH="${1#*=}"; shift ;;
        *)           FILTER="$1"; shift ;;
    esac
done

case "$THROUGH" in
    "")      TYPES="arithmetic-vars control-flow pass-int call-int oo-int" ;;
    1)       TYPES="arithmetic-vars" ;;
    2)       TYPES="arithmetic-vars control-flow" ;;
    3)       TYPES="arithmetic-vars control-flow pass-int" ;;
    4|5|6)   TYPES="arithmetic-vars control-flow pass-int call-int" ;;
    7)       TYPES="arithmetic-vars control-flow pass-int call-int oo-int" ;;
    *)       echo "Invalid --through value: $THROUGH (expected 1..7)"; exit 1 ;;
esac

tests_performed=0
for TYPE in $TYPES
do
    if [ ! -d "./test-cases/$TYPE" ]; then continue; fi
    for FILE in ./test-cases/$TYPE/[0-9]*-*.c
    do
        [ -f "$FILE" ] || continue
        if [ -n "$FILTER" ] && [[ "$FILE" != *"$FILTER"* ]]; then continue; fi
        make prog=$FILE type=$TYPE
        read -n 1 -s -r -p "Press any key to continue"
        make clean
        tests_performed=$((tests_performed+1))
    done
done

echo
echo
if [ -n "$THROUGH" ]; then echo " Scope: cumulative through Assignment $THROUGH"; fi
if [ -n "$FILTER" ];  then echo " Filter: $FILTER"; fi
echo " Tests performed: $tests_performed "
echo " ***** ALL TESTS PASSED!! ***** "
echo
echo
