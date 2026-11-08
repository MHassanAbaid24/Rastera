# expect-error: wrong argument count for 'f': expected 2, got 1
@f(a[], n) [
    ^ a[n];
]
@main() [
    ^ f(<1, 2>);
]
