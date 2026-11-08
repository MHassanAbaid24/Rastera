# expect-error: function 'first' expects an integer for argument 1, got an array
@first(value) [
    ^ value;
]
@main() [
    values := <1, 2>;
    ^ first(values);
]
