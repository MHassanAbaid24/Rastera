# expect-runtime-error: array index out of range
# expect-stdout: 7
# The bounds check works on a store through a parameter, including one reached via a nested call.
@put(a[], i) [
    a[i] := 7;
    ^ 0;
]
@relay(a[], i) [
    ^ put(a, i);
]
@main() [
    v := arr(2);
    relay(v, 1);
    print(v[1]);
    relay(v, 0 - 1);
    print(5);
    ^ 0;
]
