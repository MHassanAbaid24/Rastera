# expect-stdout: 1
# expect-stdout: 2
# expect-stdout: 5
# expect-stdout: 5
# expect-stdout: 6
# expect-stdout: 9
# expect-exit: 0
# The proposal's insertion sort (section 4), unchanged, sorting a caller's array in place.
# The smallest key (1) walks j down to -1: `j >= 0 & a[j] > key` must short-circuit, or a[-1]
# would be a runtime error.
@insertion(a[], n) [
    * i :: 1..(n - 1) [
         key := a[i];
         j := i - 1;
         * (j >= 0 & a[j] > key) [
             a[j + 1] := a[j];
             j := j - 1;
         ]
         a[j + 1] := key;
    ]
    ^ 0;
]
@main() [
    v := <5, 2, 9, 1, 5, 6>;
    insertion(v, 6);
    * k :: 0..5 [
        print(v[k]);
    ]
    ^ 0;
]
