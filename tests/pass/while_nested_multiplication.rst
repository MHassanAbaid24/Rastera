# expect-stdout: 6
@main() [
    outer := 0;
    total := 0;
    * (outer < 2) [
        inner := 0;
        * (inner < 3) [
            total := total + 1 * 1;
            inner := inner + 1;
        ]
        outer := outer + 1;
    ]
    print(total);
    ^ 0;
]
