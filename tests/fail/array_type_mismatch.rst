# expect-error: type mismatch
@main() [
    x := 5;
    a := <1, 2, 3>;
    x := a;
    ^ 0;
]
