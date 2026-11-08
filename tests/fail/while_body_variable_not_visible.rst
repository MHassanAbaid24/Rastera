# expect-error: undefined variable 'hidden'
@main() [
    * (0) [
        hidden := 1;
    ]
    print(hidden);
    ^ 0;
]
