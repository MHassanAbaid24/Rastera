# expect-stdout: 0
# expect-stdout: 10
# expect-stdout: 20
# expect-stdout: 30
# expect-stdout: 3
# expect-exit: 0
# An array parameter is passed by reference: stores through it are visible to the caller.
@fill(a[], n, step) [
    * i :: 0..(n - 1) [
        a[i] := (i + 1) * step;
    ]
    ^ n;
]
@main() [
    v := arr(3);
    print(v[0]);
    count := fill(v, 3, 10);
    print(v[0]);
    print(v[1]);
    print(v[2]);
    print(count);
    ^ 0;
]
