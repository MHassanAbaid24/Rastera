# expect-error: cannot index a non-array value
@f(a) [
    a[0] := 1;
    ^ 0;
]
@main() [
    ^ f(1);
]
