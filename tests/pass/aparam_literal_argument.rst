# expect-stdout: 6
# expect-stdout: 0
# expect-exit: 0
# Any array-valued expression can be passed: a literal or an `arr(n)` allocation.
@total(a[], n) [
    s := 0;
    * i :: 0..(n - 1) [
        s := s + a[i];
    ]
    ^ s;
]
@main() [
    print(total(<1, 2, 3>, 3));
    print(total(arr(4), 4));
    ^ 0;
]
