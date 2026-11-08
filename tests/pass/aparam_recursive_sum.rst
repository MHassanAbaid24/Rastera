# expect-stdout: 21
# expect-exit: 0
# Recursion with an array parameter: every frame sees the same array.
@sum(a[], i, n) [
    ? (i >= n) [
        ^ 0;
    ]
    ^ a[i] + sum(a, i + 1, n);
]
@main() [
    print(sum(<1, 2, 3, 4, 5, 6>, 0, 6));
    ^ 0;
]
