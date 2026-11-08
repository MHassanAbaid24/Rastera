# expect-error: type mismatch: cannot assign integer to variable 'a' of type array
@f(a[]) [
    a := 3;
    ^ 0;
]
@main() [
    ^ f(<1>);
]
