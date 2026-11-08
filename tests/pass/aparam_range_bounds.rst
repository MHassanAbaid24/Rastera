# expect-stdout: 10
# expect-stdout: 30
# expect-exit: 0
# Array parameters are used in range-loop bounds only through indexing: the last element sets the bound.
@sum_to_last(a[]) [
    s := 0;
    * i :: 0..(a[2]) [
        s := s + i;
    ]
    ^ s;
]
@main() [
    print(sum_to_last(<9, 9, 4>));
    print(sum_to_last(<0, 0, 7>) + 2);
    ^ 0;
]
