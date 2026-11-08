# upper bounds may end in an operator, a call, or a parenthesised index
# expect-stdout: 15
# expect-stdout: 3
# expect-stdout: 9
@twice(x) [
    ^ x * 2;
]
@main() [
    n := 5;
    sum := 0;
    * i :: 1..n [
        sum := sum + i;
    ]
    print(sum);
    values := <1, 2, 3>;
    count := 0;
    * i :: 0..(values[2] - 1) [
        count := count + 1;
    ]
    print(count);
    * i :: 0..twice(n) - 1 [
        sum := i;
    ]
    print(sum);
    ^ 0;
]
