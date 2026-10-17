# expect-exit: 5
@add(a, b) [
    ^ a + b;
]
@main() [
    ^ add(2, 3);
]
