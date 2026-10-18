# expect-runtime-error: negative array size
@main() [
    a := arr(-1);
    ^ 0;
]
