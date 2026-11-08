# expect-runtime-error: integer modulo overflow
@main() [
    minimum := -2147483647 - 1;
    ^ minimum % -1;
]
