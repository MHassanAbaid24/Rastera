# Recursion and arrays: fill an array with Fibonacci numbers computed
# recursively, then compare one element with a literal array.
@fib(n) [
    ? (n < 2) [
        ^ n;
    ]
    ^ fib(n - 1) + fib(n - 2);
]

@main() [
    expected := <0, 1, 1, 2, 3, 5, 8, 13>;
    results := arr(8);
    i := 0;
    * (i < 8) [
        results[i] := fib(i);
        i := i + 1;
    ]
    print(results[7]);
    print(results[7] == expected[7]);
    ^ 0;
]
