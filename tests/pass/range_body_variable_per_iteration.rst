# a variable declared in the body is new on every iteration
# expect-stdout: 1
# expect-stdout: 2
# expect-stdout: 3
@main() [
    * i :: 1..3 [
        fresh := 0;
        fresh := fresh + i;
        print(fresh);
    ]
    ^ 0;
]
