# a is evaluated first, b second, and neither is re-evaluated by the loop
# expect-stdout: 10
# expect-stdout: 20
# expect-stdout: 1
# expect-stdout: 2
# expect-stdout: 3
@low() [
    print(10);
    ^ 1;
]
@high() [
    print(20);
    ^ 3;
]
@main() [
    * i :: low()..high() [
        print(i);
    ]
    ^ 0;
]
