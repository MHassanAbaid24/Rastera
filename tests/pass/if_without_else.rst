# expect-exit: 42
@main() [
    x := 10;
    ? (1) [
        x := 42;
    ]
    ? (0) [
        x := 99;
    ]
    ^ x;
]
