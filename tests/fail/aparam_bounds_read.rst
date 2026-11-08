# expect-runtime-error: array index out of range
# expect-stdout: 1
# The bounds check works on a parameter: the callee reads past the end of the caller's array.
@at(a[], i) [
    ^ a[i];
]
@main() [
    v := <1, 2>;
    print(at(v, 1) - 1);
    print(at(v, 2));
    ^ 0;
]
