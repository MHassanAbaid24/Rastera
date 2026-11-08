# expect-stdout: 3
# expect-stdout: 4
# expect-exit: 3
@mark(x) [
    print(x);
    ^ x;
]
@main() [
    a := 0 & mark(1);
    b := 5 | mark(2);
    c := 1 & mark(3);
    d := 0 | mark(4);
    ^ a + b + c + d;
]
