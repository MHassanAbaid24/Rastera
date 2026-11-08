# expect-stdout: 1
# expect-stdout: 1
# expect-stdout: 1
# expect-stdout: 1
# expect-exit: 0
@main() [
    print(5 & 7);
    print(1 | 0 & 0);
    print(!0 == 1);
    print(0 | 5 & 7);
    ^ 0;
]
