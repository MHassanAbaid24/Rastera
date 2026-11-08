# expect-exit: 24
@sq(n) [
    ^ n * n;
]
@main() [
    x := 3;
    ^ sq(x) + sq(x + 1) - 1;
]
