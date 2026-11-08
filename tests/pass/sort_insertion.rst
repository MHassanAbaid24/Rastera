# expect-stdout: 1
# expect-stdout: 2
# expect-stdout: 3
# expect-stdout: 4
# expect-stdout: 5
# expect-stdout: -1
# expect-stdout: 1
# expect-stdout: 3
# expect-stdout: 5
# expect-stdout: 7
# expect-stdout: 9
# expect-stdout: -1
# expect-stdout: 1
# expect-stdout: 2
# expect-stdout: 5
# expect-stdout: 5
# expect-stdout: 6
# expect-stdout: 9
# expect-stdout: -1
# expect-stdout: 1
# expect-stdout: 2
# expect-stdout: 2
# expect-stdout: 4
# expect-stdout: 4
# expect-stdout: 4
# expect-stdout: -1
# expect-stdout: 7
# expect-stdout: -1
# expect-stdout: 1
# expect-stdout: 2
# expect-stdout: -1
# expect-exit: 0
# examples/insertion_sort.rst unchanged: insertion sort on already sorted, reverse sorted, mixed, duplicate,
# single-element and two-element arrays. Each array's output ends with -1.
# Insertion sort (the proposal's section 4 program) on six arrays. `insertion` sorts the
# caller's array in place; `print_all` prints one array and then -1, which cannot be an element
# here, so the output shows where each array ends. The smallest key walks j down to -1:
# `j >= 0 & a[j] > key` short-circuits, so a[-1] is never read.
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

@print_all(a[], n) [
    * i :: 0..(n - 1) [
        print(a[i]);
    ]
    print(-1);
    ^ 0;
]

@main() [
    sorted := <1, 2, 3, 4, 5>;
    insertion(sorted, 5);
    print_all(sorted, 5);

    reverse := <9, 7, 5, 3, 1>;
    insertion(reverse, 5);
    print_all(reverse, 5);

    mixed := <5, 2, 9, 1, 5, 6>;
    insertion(mixed, 6);
    print_all(mixed, 6);

    dups := <4, 2, 4, 1, 2, 4>;
    insertion(dups, 6);
    print_all(dups, 6);

    one := <7>;
    insertion(one, 1);
    print_all(one, 1);

    two := <2, 1>;
    insertion(two, 2);
    print_all(two, 2);
    ^ 0;
]
