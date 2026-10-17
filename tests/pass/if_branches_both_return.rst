# expect-exit: 15
@main() [
    x := 5;
    ? (x > 2) [
        ^ 15;
    ] : [
        ^ 99;
    ]
]
