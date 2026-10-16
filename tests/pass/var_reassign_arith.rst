# expect-exit: 7
@main() [
    x := 3;
    x := x * 2 + 1;
    ^ x;
]
