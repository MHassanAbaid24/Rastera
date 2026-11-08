# expect-runtime-error: integer division overflow
@main() [
    minimum := -2147483647 - 1;
    ^ minimum / -1;
]
