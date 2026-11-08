# expect-stdout: 1
# expect-stdout: 2
# expect-stdout: 3
# expect-stdout: 2
# expect-exit: 0
# Several array parameters in one function, each bound to its own caller array.
@copy(dst[], src[], n) [
    * i :: 0..(n - 1) [
        dst[i] := src[i];
    ]
    ^ 0;
]
@main() [
    a := <1, 2, 3>;
    b := arr(3);
    copy(b, a, 3);
    * i :: 0..2 [
        print(b[i]);
    ]
    b[0] := 9;
    print(a[1]);
    ^ 0;
]
