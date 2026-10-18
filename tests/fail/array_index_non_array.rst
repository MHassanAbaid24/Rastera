# expect-error: cannot index a non-array value
@main() [
    x := 5;
    ^ x[0];
]
