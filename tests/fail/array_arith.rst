# expect-error: array literal elements must be integers
@main() [
    a := <1, <2, 3>>;
    ^ 0;
]
