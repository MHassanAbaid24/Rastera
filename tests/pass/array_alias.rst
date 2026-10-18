# expect-exit: 99
@main() [
    a := <1, 2, 3>;
    b := a;
    b[0] := 99;
    ^ a[0];
]
