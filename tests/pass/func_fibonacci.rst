# expect-stdout: 55
# expect-exit: 0
@fib(n) [
    ? (n < 2) [
        ^ n;
    ]
    ^ fib(n - 1) + fib(n - 2);
]
@main() [
    print(fib(10));
    ^ 0;
]
