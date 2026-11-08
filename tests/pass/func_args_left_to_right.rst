# expect-stdout: 1
# expect-stdout: 2
# expect-exit: 12
@pair(a, b) [
    ^ a * 10 + b;
]
@main() [
    ^ pair(print(1) + 1, print(2) + 2);
]
