# expect-exit: 7
@choose() [
    ? (<1, 2>[0] < 3) [
        ^ 7;
    ]
    ^ 0;
]
@main() [
    ^ choose();
]
