# expect-stdout: 55
@main() [
    total := 0;
    * i :: 1..10 [
        total := total + i;
    ]
    print(total);
    ^ 0;
]
