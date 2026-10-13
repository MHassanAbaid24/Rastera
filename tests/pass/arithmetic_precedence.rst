# expect-exit: 14
# 2 + 3 * 4 should do multiplication first: 3 * 4 = 12, + 2 = 14
@main() [
    ^ 2 + 3 * 4;
]
