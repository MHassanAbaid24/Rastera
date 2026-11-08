# the counter must not wrap past 2147483647: exactly two iterations
# expect-stdout: 2147483646
# expect-stdout: 2147483647
# expect-stdout: 2
@main() [
    count := 0;
    * i :: 2147483646..2147483647 [
        print(i);
        count := count + 1;
    ]
    print(count);
    ^ 0;
]
