# expect-stdout: 3
# expect-exit: 0
# The signature (including array kinds) is known before the body, so calls may precede the definition.
@main() [
    v := arr(2);
    set(v, 1, 3);
    print(v[1]);
    ^ 0;
]
@set(a[], i, x) [
    a[i] := x;
    ^ 0;
]
