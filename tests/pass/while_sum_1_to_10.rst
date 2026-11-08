# expect-stdout: 55
@main() [
    i := 1;
    sum := 0;
    * (i <= 10) [
        sum := sum + i;
        i := i + 1;
    ]
    print(sum);
    ^ 0;
]
