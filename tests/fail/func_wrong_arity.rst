# expect-error: wrong argument count for 'add': expected 2, got 1
@add(a, b) [
    ^ a + b;
]
@main() [
    ^ add(1);
]
