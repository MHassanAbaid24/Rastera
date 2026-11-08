#!/usr/bin/env bash
# Rastera test runner. Usage: tests/run_tests.sh [--interp] <path-to-rasterc-or-rasteri>
#
# Without --interp, pass cases compile with rasterc and run the resulting program.
# With --interp, every case runs directly through rasteri. Directives:
#     # expect-exit: N        process exit status (default 0)
#     # expect-stdout: text   one expected stdout line; repeat in output order
#     # expect-error: text    static-error stderr must contain this text and exit 1
#     # expect-runtime-error: text   stderr must contain this text and exit 2
#                                expected stdout is also checked before a runtime error
# Image directives (pass cases only). Every run starts without an out.ppm in its working
# directory; a program writes one with export(), and these check the file it left:
#     # expect-ppm-size: W H         header is exactly "P6\nW H\n255\n" and the file holds W*H RGB bytes
#     # expect-ppm-pixel: X Y RRGGBB  pixel (X, Y), counted from the top-left, has this hex colour; repeatable
#     # expect-ppm-sha256: HASH      sha256 of the whole file (header and pixels)
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

ppm_check() {  # ppm_check <test>: check out.ppm against the expect-ppm-* directives; print why on failure
    local test="$1" ppm="$WORK/out.ppm" size pixels hash
    size="$(directive expect-ppm-size "$test")"
    pixels="$(directive expect-ppm-pixel "$test")"
    hash="$(directive expect-ppm-sha256 "$test")"
    [ -z "$size$pixels$hash" ] && return 0
    if [ ! -f "$ppm" ]; then
        echo "out.ppm was not written"
        return 1
    fi
    local width height header_len want_width want_height want_bytes
    read -r width height < <(sed -n 2p "$ppm")
    header_len="$(head -n 3 "$ppm" | wc -c)"
    if [ -n "$size" ]; then
        read -r want_width want_height <<<"$size"
        if ! printf 'P6\n%s %s\n255\n' "$want_width" "$want_height" | cmp -s - <(head -c "$header_len" "$ppm"); then
            echo "ppm header is not 'P6 $want_width $want_height 255'"
            return 1
        fi
        want_bytes=$((header_len + want_width * want_height * 3))
        if [ "$(stat -c %s "$ppm")" != "$want_bytes" ]; then
            echo "ppm is $(stat -c %s "$ppm") bytes, expected $want_bytes"
            return 1
        fi
    fi
    local x y want got
    while read -r x y want; do
        [ -z "$x" ] && continue
        got="$(od -An -tx1 -v -j $((header_len + (y * width + x) * 3)) -N 3 "$ppm" | tr -d ' \n')"
        if [ "$got" != "${want,,}" ]; then
            echo "pixel ($x, $y) is $got, expected ${want,,}"
            return 1
        fi
    done <<<"$pixels"
    if [ -n "$hash" ] && [ "$(sha256sum "$ppm" | cut -d' ' -f1)" != "$hash" ]; then
        echo "ppm sha256 is $(sha256sum "$ppm" | cut -d' ' -f1), expected $hash"
        return 1
    fi
    return 0
}

run_pass() {
    local test="$1"
    local exe="$WORK/prog"
    compile_failed=0
    rm -f "$WORK/out.ppm"
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
    if ! reason="$(ppm_check "$test")"; then
        fail "$name" "$reason"
        continue
    fi
    passed=$((passed + 1))
done

for test in "$TESTS"/fail/*.rst; do
    [ -e "$test" ] || continue
    rm -f "$WORK/out.ppm"
    name="fail/$(basename "$test")"
    want_compile="$(directive expect-error "$test")"
    want_runtime="$(directive expect-runtime-error "$test")"
    if [ -z "$want_compile" ] && [ -z "$want_runtime" ]; then
        fail "$name" "missing '# expect-error:' or '# expect-runtime-error:' directive"
        continue
    fi
    if [ "$mode" = compiler ]; then
        exe="$WORK/prog"
        : >"$WORK/out"
        "$RASTERA" "$test" -o "$exe" 2>"$WORK/err"
        status=$?
        if [ "$status" = 0 ] && [ -z "$want_compile" ]; then
            (cd "$WORK" && "$exe" >"$WORK/out" 2>"$WORK/err")
            status=$?
        fi
        if ! diff <(directive expect-stdout "$test") "$WORK/out" >"$WORK/diff"; then
            fail "$name" "stdout differs (< expected, > actual):"$'\n'"$(cat "$WORK/diff")"
            continue
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
