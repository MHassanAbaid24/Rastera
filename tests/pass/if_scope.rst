# expect-exit: 10
@main() [
    x := 10;
    ? (x == 10) [
        inner := 50;
        x := 10;
    ]
    ^ x;
]
