# expect-stdout: 10
# expect-stdout: 20
# expect-stdout: 30
# expect-exit: 0
@main() [
    print(10);
    print(20);
    print(30);
    ^ 0;
]
