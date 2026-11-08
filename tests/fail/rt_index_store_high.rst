# expect-runtime-error: array index out of range
@main() [
    values := <1>;
    values[1] := 2;
    ^ 0;
]
