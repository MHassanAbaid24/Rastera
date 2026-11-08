# expect-runtime-error: negative array size
@main() [
    values := arr(-1);
    ^ 0;
]
