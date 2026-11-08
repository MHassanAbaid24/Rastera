# expect-stdout: 0
# expect-stdout: 7
@main() [
    count := 0;
    * i :: 5..4 [
        count := count + 1;
    ]
    * i :: 0 - 1..0 - 3 [
        count := count + 1;
    ]
    print(count);
    print(7);
    ^ 0;
]
