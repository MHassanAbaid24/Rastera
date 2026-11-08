#!/usr/bin/env bash
# Rastera test runner. Usage: tests/run_tests.sh [--interp] <path-to-rasterc-or-rasteri>
#
# Without --interp, pass cases compile with rasterc and run the resulting program.
# With --interp, every case runs directly through rasteri. Directives:
#     # expect-exit: N        process exit status (default 0)
#     # expect-stdout: text   one expected stdout line; repeat in output order
#     # expect-error: text    static-error stderr must contain this text
#     # expect-runtime-error: text   stderr must contain this text and exit 2
set -uo pipefail

mode=compiler
if [ "${1:-}" = "--interp" ]; then
    mode=interpreter
    shift
fi
RASTERA="$(realpath "${1:?usage: $0 [--interp] <path-to-rasterc-or-rasteri>}")"
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

run_pass() {
    local test="$1"
    local exe="$WORK/prog"
    compile_failed=0
    if [ "$mode" = compiler ]; then
        if ! "$RASTERA" "$test" -o "$exe" 2>"$WORK/err"; then
            compile_failed=1
            return
        fi
        (cd "$WORK" && "$exe" >"$WORK/out" 2>"$WORK/err")
    else
        (cd "$WORK" && "$RASTERA" "$test" >"$WORK/out" 2>"$WORK/err")
    fi
}

for test in "$TESTS"/pass/*.rst; do
    [ -e "$test" ] || continue
    name="pass/$(basename "$test")"
    run_pass "$test"
    status=$?
    if [ "$compile_failed" = 1 ]; then
        fail "$name" "compile error: $(cat "$WORK/err")"
        continue
    fi
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
    if [ "$mode" = compiler ]; then
        exe="$WORK/prog"
        "$RASTERA" "$test" -o "$exe" 2>"$WORK/err"
        status=$?
        if [ "$status" = 0 ] && [ -z "$want_compile" ]; then
            (cd "$WORK" && "$exe" >"$WORK/out" 2>"$WORK/err")
            status=$?
        fi
    else
        (cd "$WORK" && "$RASTERA" "$test" >"$WORK/out" 2>"$WORK/err")
        status=$?
    fi
    if [ -n "$want_compile" ]; then
        if [ "$status" != 1 ] || ! grep -qF -- "$want_compile" "$WORK/err"; then
            fail "$name" "expected static error (exit 1) containing '$want_compile', got exit $status: $(cat "$WORK/err")"
            continue
        fi
        passed=$((passed + 1))
        continue
    fi
    if [ "$status" != 2 ] || ! grep -qF -- "$want_runtime" "$WORK/err"; then
        fail "$name" "expected runtime error (exit 2) containing '$want_runtime', got exit $status: $(cat "$WORK/err")"
        continue
    fi
    if ! diff <(directive expect-stdout "$test") "$WORK/out" >"$WORK/diff"; then
        fail "$name" "stdout differs (< expected, > actual):"$'\n'"$(cat "$WORK/diff")"
        continue
    fi
    passed=$((passed + 1))
done

echo "$passed passed, $failed failed"
[ "$failed" -eq 0 ]
