# expect-runtime-error: negative array size
# expect-stdout: 7
@main() [
    print(7);
    values := arr(-1);
    ^ 0;
]
