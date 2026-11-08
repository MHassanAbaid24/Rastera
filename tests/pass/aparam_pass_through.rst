# expect-stdout: 42
# expect-stdout: 43
# expect-exit: 0
# An array parameter can be handed on to another function; the store lands in the original array.
@inner(buf[]) [
    buf[1] := 42;
    ^ 0;
]
@outer(data[]) [
    inner(data);
    data[2] := data[1] + 1;
    ^ 0;
]
@main() [
    v := arr(3);
    outer(v);
    print(v[1]);
    print(v[2]);
    ^ 0;
]
