# expect-error: operands must be integers
@main() [
    a := <1>;
    b := <1>;
    ? (a == b) [
        ^ 1;
    ]
    ^ 0;
]
