# expect-exit: 3
@first_multiple_of_three(limit) [
    * i :: 1..limit [
        ? (i % 3 == 0) [
            ^ i;
        ]
    ]
    ^ 0 - 1;
]
@main() [
    ^ first_multiple_of_three(10);
]
