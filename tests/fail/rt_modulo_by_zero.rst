# expect-runtime-error: modulo by zero
@main() [
    x := 1;
    ^ x % 0;
]
