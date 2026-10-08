#!/usr/bin/env bash
# Rastera test runner.  Usage: tests/run_tests.sh <path-to-rasterc>
#
# tests/pass/*.rst  must compile and run. Directives (comment lines):
#     # expect-exit: N        process exit status (default 0)
#     # expect-stdout: text   one line of expected stdout; repeat for more lines, in order
#                             (if absent, stdout must be empty)
# tests/fail/*.rst  must be rejected. Directive:
#     # expect-error: text    compiler stderr must contain this text (required)
#     # expect-runtime-error: text   compiles, but running exits with 2 and stderr contains text
set -uo pipefail

RASTERC="$(realpath "${1:?usage: $0 <path-to-rasterc>}")"
TESTS="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

passed=0
failed=0

fail() {
    echo "FAIL $1: $2"
    failed=$((failed + 1))
}

directive() {  # directive <name> <file>: values of `# <name>: value` lines
    sed -n "s/^# $1: \{0,1\}//p" "$2"
}

for test in "$TESTS"/pass/*.rst; do
    [ -e "$test" ] || continue
    name="pass/$(basename "$test")"
    exe="$WORK/prog"
    if ! "$RASTERC" "$test" -o "$exe" 2>"$WORK/err"; then
        fail "$name" "compile error: $(cat "$WORK/err")"
        continue
    fi
    (cd "$WORK" && "$exe" >"$WORK/out" 2>"$WORK/err")
    status=$?
    want_status="$(directive expect-exit "$test")"
    want_status="${want_status:-0}"
    if [ "$status" != "$want_status" ]; then
        fail "$name" "exit status $status, expected $want_status"
        continue
    fi
    if ! diff <(directive expect-stdout "$test") "$WORK/out" >"$WORK/diff"; then
        fail "$name" "stdout differs (< expected, > actual):"$'\n'"$(cat "$WORK/diff")"
        continue
    fi
    passed=$((passed + 1))
done

for test in "$TESTS"/fail/*.rst; do
    [ -e "$test" ] || continue
    name="fail/$(basename "$test")"
    want_compile="$(directive expect-error "$test")"
    want_runtime="$(directive expect-runtime-error "$test")"
    if [ -z "$want_compile" ] && [ -z "$want_runtime" ]; then
        fail "$name" "missing '# expect-error:' or '# expect-runtime-error:' directive"
        continue
    fi
    exe="$WORK/prog"
    if "$RASTERC" "$test" -o "$exe" 2>"$WORK/err"; then
        if [ -n "$want_compile" ]; then
            fail "$name" "compiled, but expected error containing: $want_compile"
            continue
        fi
        (cd "$WORK" && "$exe" >/dev/null 2>"$WORK/err")
        status=$?
        if [ "$status" != 2 ] || ! grep -qF -- "$want_runtime" "$WORK/err"; then
            fail "$name" "expected runtime error (exit 2) containing '$want_runtime', got exit $status: $(cat "$WORK/err")"
            continue
        fi
    else
        if [ -z "$want_compile" ]; then
            fail "$name" "compile error, expected runtime error: $(cat "$WORK/err")"
            continue
        fi
        if ! grep -qF -- "$want_compile" "$WORK/err"; then
            fail "$name" "error message does not contain '$want_compile': $(cat "$WORK/err")"
            continue
        fi
    fi
    passed=$((passed + 1))
done

echo "$passed passed, $failed failed"
[ "$failed" -eq 0 ]
