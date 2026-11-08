# expect-stdout: 8
# expect-exit: 0
# `b := a;` inside a function aliases the caller's array (no copy), as it does for local arrays.
@poke(a[]) [
    b := a;
    b[0] := 8;
    ^ 0;
]
@main() [
    v := <1, 2>;
    poke(v);
    print(v[0]);
    ^ 0;
]
