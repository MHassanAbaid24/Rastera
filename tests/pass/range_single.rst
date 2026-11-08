# expect-stdout: 1
# expect-stdout: 3
@main() [
    count := 0;
    last := 0;
    * i :: 3..3 [
        count := count + 1;
        last := i;
    ]
    print(count);
    print(last);
    ^ 0;
]
