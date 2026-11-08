# a '[' right after the upper bound opens the body, so an indexed bound must be parenthesised
# expect-error: syntax error
@main() [
    values := <1, 2, 3>;
    * i :: 0..values[2] [
    ]
    ^ 0;
]
