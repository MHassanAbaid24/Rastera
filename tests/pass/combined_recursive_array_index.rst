# expect-exit: 6
@sum_to(n) [
    values := <n, n + 1>;
    ? (n == 0) [
        ^ values[0];
    ]
    ^ values[0] + sum_to(n - 1);
]
@main() [
    ^ sum_to(3);
]
