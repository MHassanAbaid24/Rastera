# expect-exit: 120
@main() [
    ^ fact(5);
]
@fact(n) [
    ? (n <= 1) [
        ^ 1;
    ]
    ^ n * fact(n - 1);
]
