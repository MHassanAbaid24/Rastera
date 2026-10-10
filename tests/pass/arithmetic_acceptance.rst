# expect-exit: 250
# Task T01 acceptance case: (2 + 3) * -4 % 7 = -20 % 7 = -6. Linux exit code is 250 (-6 + 256).
@main() [
    ^ (2 + 3) * -4 % 7;
]
