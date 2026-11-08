# the loop variable hides an outer variable of the same name; the outer one is untouched
# bounds are evaluated before the loop variable exists, so `i..i + 1` reads the outer i
# expect-stdout: 5
# expect-stdout: 6
# expect-stdout: 5
@main() [
    i := 5;
    * i :: i..i + 1 [
        print(i);
    ]
    print(i);
    ^ 0;
]
