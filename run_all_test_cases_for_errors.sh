#!/bin/bash
# Progress Bar code modified from: https://www.baeldung.com/linux/command-line-progress-bar
# Parallel code written by Grok (xAI) — 12/2025 with Grok v4.
# Adjusted by Claude Opus 4.7 (Anthropic) at Dan O'Malley's direction, 2026.
#
# Needs sudo apt install parallel
#
# Each test builds BOTH backends and checks each for answer-correctness: the
# x86-64 binary AND the llc-compiled LLVM binary. Any failure (build error,
# llc/LLVM syntax error, or either backend giving the wrong answer) makes
# `make` exit non-zero, and `parallel --halt now,fail=1` stops the whole run.
#
# Usage:
#   ./run_all_test_cases_for_errors.sh                          # every test in every category
#   ./run_all_test_cases_for_errors.sh --through 1              # cumulative through Assignment 1 (arithmetic-vars)
#   ./run_all_test_cases_for_errors.sh --through 2              # cumulative through Assignment 2 (+ control-flow)
#   ./run_all_test_cases_for_errors.sh --through 3              # cumulative through Assignment 3 (+ pass-int)
#   ./run_all_test_cases_for_errors.sh --through 4              # cumulative through Assignment 4 (+ call-int)
#   ./run_all_test_cases_for_errors.sh --through 5              # cumulative through Assignment 5 (liveness; same set as 4)
#   ./run_all_test_cases_for_errors.sh --through 6              # cumulative through Assignment 6 (optimizations; same set as 4)
#   ./run_all_test_cases_for_errors.sh --through 7              # cumulative through Assignment 7 (+ oo-int)
#   ./run_all_test_cases_for_errors.sh arithmetic-vars          # substring filter on the test-case path
#   ./run_all_test_cases_for_errors.sh dowhile                  # substring filter
#   ./run_all_test_cases_for_errors.sh --through 2 dowhile      # combined: dowhile tests within Assignment-2 scope
#
# Cumulative scope and substring filter compose: --through selects which
# test categories appear; the substring further narrows what runs.

function run_test {
    local file="$1" type="$2"
    local basename
    basename=$(basename "${file%.c}")
    local tmpdir
    tmpdir=$(mktemp -d /tmp/compiler-test.XXXXXX)
    trap "rm -rf '$tmpdir'" EXIT

    cp Makefile lex.l parser.y ast.h ast.c visitor.h \
       symboltable.h symboltable.c semantic.h semantic.c \
       typecheck.h typecheck.c \
       constfold.h constfold.c \
       regalloc.h regalloc.c \
       liveness.h liveness.c \
       emit_llvm.c emit_x64.c main.c "$tmpdir"
    mkdir -p "$tmpdir/test-cases/$type"
    cp "test-cases/$type/driver.cpp" "test-cases/$type/evaluator.cpp" "$tmpdir/test-cases/$type/"
    cp "$file" "$tmpdir/prog.c"

    cd "$tmpdir" || exit 1
    make prog=prog.c type=$type &> test.log
    local exitcode=$?

    if [ $exitcode -eq 0 ]; then
        make clean &> /dev/null
    else
        cd - || exit 1
        mkdir -p logs
        cp "$tmpdir/test.log" "logs/${basename}_x64.log"
        echo
        echo " **** FAILED: $file **** "
        echo " See logs/${basename}_x64.log "
        echo " ---- last 30 lines: ----"
        tail -30 "logs/${basename}_x64.log" | sed 's/^/   /'
        echo " ------------------------"
        echo
        return 1
    fi

    cd - || exit 1
    return 0
}

set -e

# Parse args: --through N selects cumulative scope; any other positional arg
# is a substring filter on test-case paths.
THROUGH=""
FILTER=""
while [ $# -gt 0 ]; do
    case "$1" in
        --through)
            THROUGH="$2"
            shift 2
            ;;
        --through=*)
            THROUGH="${1#*=}"
            shift
            ;;
        -h|--help)
            sed -n '2,22p' "$0"
            exit 0
            ;;
        *)
            FILTER="$1"
            shift
            ;;
    esac
done

# Decide which test categories are in scope.
include_arithmetic_vars=0
include_control_flow=0
include_pass_int=0
include_call_int=0
include_oo_int=0
case "$THROUGH" in
    "")      include_arithmetic_vars=1; include_control_flow=1; include_pass_int=1; include_call_int=1; include_oo_int=1 ;;
    1)       include_arithmetic_vars=1 ;;
    2)       include_arithmetic_vars=1; include_control_flow=1 ;;
    3)       include_arithmetic_vars=1; include_control_flow=1; include_pass_int=1 ;;
    4|5|6)   include_arithmetic_vars=1; include_control_flow=1; include_pass_int=1; include_call_int=1 ;;
    7)       include_arithmetic_vars=1; include_control_flow=1; include_pass_int=1; include_call_int=1; include_oo_int=1 ;;
    *)       echo "Invalid --through value: $THROUGH (expected 1..7)"; exit 1 ;;
esac

[ "$include_arithmetic_vars" = 1 ] && mapfile -t arithmetic_vars_files < <(find "./test-cases/arithmetic-vars/" -type f -name "[0-9]*-*.c" 2>/dev/null | sort) || arithmetic_vars_files=()
[ "$include_control_flow"    = 1 ] && mapfile -t control_flow_files    < <(find "./test-cases/control-flow/"    -type f -name "[0-9]*-*.c" 2>/dev/null | sort) || control_flow_files=()
[ "$include_pass_int"        = 1 ] && mapfile -t pass_int_files        < <(find "./test-cases/pass-int/"        -type f -name "[0-9]*-*.c" 2>/dev/null | sort) || pass_int_files=()
[ "$include_call_int"        = 1 ] && mapfile -t call_int_files        < <(find "./test-cases/call-int/"        -type f -name "[0-9]*-*.c" 2>/dev/null | sort) || call_int_files=()
[ "$include_oo_int"          = 1 ] && mapfile -t oo_int_files          < <(find "./test-cases/oo-int/"          -type f -name "[0-9]*-*.c" 2>/dev/null | sort) || oo_int_files=()

if [ -n "$FILTER" ]; then
    mapfile -t arithmetic_vars_files < <(printf "%s\n" "${arithmetic_vars_files[@]}" | grep -F "$FILTER" || true)
    mapfile -t control_flow_files    < <(printf "%s\n" "${control_flow_files[@]}"    | grep -F "$FILTER" || true)
    mapfile -t pass_int_files        < <(printf "%s\n" "${pass_int_files[@]}"        | grep -F "$FILTER" || true)
    mapfile -t call_int_files        < <(printf "%s\n" "${call_int_files[@]}"        | grep -F "$FILTER" || true)
    mapfile -t oo_int_files          < <(printf "%s\n" "${oo_int_files[@]}"          | grep -F "$FILTER" || true)
fi

if [ -n "$THROUGH" ]; then echo "Scope: cumulative through Assignment $THROUGH"; fi
if [ -n "$FILTER" ];  then echo "Filter: $FILTER"; fi

total=$(( ${#arithmetic_vars_files[@]} + ${#control_flow_files[@]} + ${#pass_int_files[@]} + ${#call_int_files[@]} + ${#oo_int_files[@]} ))

all_tests=()
for f in "${arithmetic_vars_files[@]}"; do all_tests+=("$f arithmetic-vars"); done
for f in "${control_flow_files[@]}";    do all_tests+=("$f control-flow");    done
for f in "${pass_int_files[@]}";        do all_tests+=("$f pass-int");        done
for f in "${call_int_files[@]}";        do all_tests+=("$f call-int");        done
for f in "${oo_int_files[@]}";          do all_tests+=("$f oo-int");          done

if [ ${#all_tests[@]} -eq 0 ]; then
    echo "No test cases matched."
    exit 0
fi

export -f run_test
printf "%s\n" "${all_tests[@]}" | parallel --colsep ' ' --bar --halt now,fail=1 -j8 run_test {1} {2}

echo
echo
echo " Tests performed: $total "
echo " ***** ALL TESTS PASSED!! ***** "
echo
echo
