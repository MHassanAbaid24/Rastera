# While loop and logic: sum the numbers 1..20 that are divisible by 3 or 5,
# then show that & and | short-circuit.
@main() [
    i := 1;
    total := 0;
    * (i <= 20) [
        ? (i % 3 == 0 | i % 5 == 0) [
            total := total + i;
        ]
        i := i + 1;
    ]
    print(total);

    zero := 0;
    # The right side of & would divide by zero, but it is never evaluated.
    print(zero != 0 & 10 / zero > 1);
    # The right side of | is skipped because the left side is already true.
    print(1 | 10 / zero);
    print(!zero);
    ^ 0;
]
