# expect-error: operand must be an integer
@f(a[]) [
    ^ -a;
]
@main() [
    ^ f(<1>);
]
