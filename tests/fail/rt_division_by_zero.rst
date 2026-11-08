# expect-runtime-error: division by zero
# expect-stdout: 7
@main() [
    print(7);
    x := 1;
    ^ x / 0;
]
