# expect-error: function 'f' expects an array for argument 2, got an integer
@f(n, a[]) [
    ^ a[n];
]
@main() [
    v := <1, 2>;
    ^ f(v, 1);
]
