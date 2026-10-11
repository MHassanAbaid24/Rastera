# expect-exit: 4
# Left-associativity: (10 - 4) - 2 = 4; if right-associative, 10 - (4 - 2) = 8
@main() [
    ^ 10 - 4 - 2;
]