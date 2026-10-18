# expect-exit: 7
@main() [
    a := arr(3);
    a[0] := 7;
    ^ a[0];
]
