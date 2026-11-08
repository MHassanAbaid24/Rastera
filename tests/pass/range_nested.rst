# expect-stdout: 36
# expect-stdout: 6
@main() [
    total := 0;
    pairs := 0;
    * i :: 1..3 [
        * j :: i..3 [
            pairs := pairs + 1;
        ]
        * j :: 1..3 [
            total := total + i * j;
        ]
    ]
    print(total);
    print(pairs);
    ^ 0;
]
