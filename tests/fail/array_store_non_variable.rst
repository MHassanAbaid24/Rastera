# expect-error: array store target must be a variable
@main() [
    a := <1, 2>;
    <5, 6>[0] := 9;
    ^ a[0];
]
