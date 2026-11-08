# fill an array with a range loop, then drain it with a while loop
# expect-stdout: 55
@main() [
    squares := arr(5);
    * i :: 0..4 [
        squares[i] := (i + 1) * (i + 1);
    ]
    n := 4;
    sum := 0;
    * (n >= 0) [
        sum := sum + squares[n];
        n := n - 1;
    ]
    print(sum);
    ^ 0;
]
