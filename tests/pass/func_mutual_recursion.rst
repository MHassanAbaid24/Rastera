# expect-stdout: 1
# expect-stdout: 0
# expect-exit: 0
@main() [
    print(is_even(10));
    print(is_even(7));
    ^ 0;
]
@is_even(n) [
    ? (n == 0) [
        ^ 1;
    ]
    ^ is_odd(n - 1);
]
@is_odd(n) [
    ? (n == 0) [
        ^ 0;
    ]
    ^ is_even(n - 1);
]
