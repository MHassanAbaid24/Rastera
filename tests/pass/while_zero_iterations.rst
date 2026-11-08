# expect-stdout: 0
@main() [
    ran := 0;
    * (0) [
        ran := 1;
    ]
    print(ran);
    ^ 0;
]
