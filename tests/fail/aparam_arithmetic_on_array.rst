# expect-error: operands must be integers
@f(a[]) [
    ^ a + 1;
]
@main() [
    ^ f(<1>);
]
