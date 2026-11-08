# expect-error: while condition must be an integer
@main() [
    values := <1>;
    * (values) [
    ]
    ^ 0;
]
