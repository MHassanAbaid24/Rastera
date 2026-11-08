# expect-stdout: 7
# expect-stdout: 99
# expect-stdout: 5
# expect-exit: 0
# Scalar parameters beside an array parameter stay local copies; the array is shared.
@mix(a[], x) [
    x := x + 100;
    a[0] := 99;
    ^ x;
]
@main() [
    v := <7, 8>;
    n := 5;
    print(v[0]);
    mix(v, n);
    print(v[0]);
    print(n);
    ^ 0;
]
