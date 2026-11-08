# expect-error: cannot index a non-array value
@f(a, b[]) [
    ^ a[0] + b[0];
]
@main() [
    ^ f(1, <1>);
]
