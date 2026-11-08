# expect-stdout: 10
# expect-stdout: 32
# expect-exit: 0
@main() [
    a := arr(5);
    a[0] := 10;
    a[1] := 32;
    print(a[0]);
    print(a[1]);
    ^ 0;
]
