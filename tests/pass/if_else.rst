# expect-exit: 7
@main() [
    x := 0;
    ? (0) [
        x := 1;
    ] : [
        x := 7;
    ]
    ^ x;
]
