# expect-exit: 0
# expect-stdout: -2147483648
@main() [
    print(2147483647 + 1);
    ^ 0;
]
