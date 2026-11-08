# expect-error: function 'first' expects an array for argument 1, got an integer
@first(a[]) [
    ^ a[0];
]
@main() [
    ^ first(3);
]
