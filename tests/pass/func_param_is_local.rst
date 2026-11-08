# expect-stdout: 10
# expect-stdout: 11
# expect-exit: 0
@bump(x) [
    x := x + 1;
    ^ x;
]
@main() [
    x := 10;
    y := bump(x);
    print(x);
    print(y);
    ^ 0;
]
