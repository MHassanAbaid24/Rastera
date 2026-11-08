# expect-stdout: 6
# expect-stdout: 5
# expect-stdout: 4
# expect-stdout: 3
# expect-stdout: 2
# expect-stdout: 1
# expect-exit: 0
# A recursive function that stores through its array parameter at every level.
@reverse(a[], lo, hi) [
    ? (lo >= hi) [
        ^ 0;
    ]
    t := a[lo];
    a[lo] := a[hi];
    a[hi] := t;
    ^ reverse(a, lo + 1, hi - 1);
]
@main() [
    v := <1, 2, 3, 4, 5, 6>;
    reverse(v, 0, 5);
    * i :: 0..5 [
        print(v[i]);
    ]
    ^ 0;
]
