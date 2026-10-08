# expect-exit: 7
# Statements after `^` are unreachable and must not break code generation.
@helper() [
    ^ 1;
]

@main() [
    ^ 7;
    ^ 8;
]
