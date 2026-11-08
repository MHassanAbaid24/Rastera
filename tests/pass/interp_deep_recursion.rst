# expect-exit: 109
@fib(n) [
    ? (n < 2) [ ^ n; ]
    ^ fib(n - 1) + fib(n - 2);
]
@main() [
    ^ fib(20);
]
