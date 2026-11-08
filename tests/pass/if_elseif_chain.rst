# expect-exit: 44
# 300 mod 256 = 44 (OS exit status keeps low 8 bits)
@main() [
    x := 3;
    ans := 0;
    ? (x == 1) [
        ans := 100;
    ] : ? (x == 2) [
        ans := 200;
    ] : ? (x == 3) [
        ans := 300;
    ] : [
        ans := 400;
    ]
    ^ ans;
]
