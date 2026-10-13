# expect-exit: 2
@main() [
    x := 1;
    [ x := 2; ]
    ^ x;
]
