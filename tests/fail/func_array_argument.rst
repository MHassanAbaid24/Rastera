# expect-error: function 'first' expects an integer argument
@first(value) [
    ^ value;
]
@main() [
    values := <1, 2>;
    ^ first(values);
]
