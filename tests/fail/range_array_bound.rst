# expect-error: range bound must be an integer
@main() [
    values := <1, 2, 3>;
    * i :: 0..values [
    ]
    ^ 0;
]
