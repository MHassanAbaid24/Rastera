# expect-error: if condition must be an integer
@main() [
    values := <1>;
    ? (values) [
        print(1);
    ]
    ^ 0;
]
