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
# examples/merge_sort.rst unchanged: merge sort on already sorted, reverse sorted, mixed, duplicate,
# single-element and two-element arrays. Each array's output ends with -1.
# Merge sort: recursive, with an arr(n) temporary buffer (the proposal's section 4 remark).
# `merge_sort` allocates the buffer once and `msort` sorts a[lo..hi] by sorting both halves
# and merging them through `tmp`. The inputs are the ones insertion_sort.rst uses, and
# `print_all` prints one array and then -1, which cannot be an element here.
@merge(a[], tmp[], lo, mid, hi) [
    i := lo;
    j := mid + 1;
    k := lo;
    * (i <= mid & j <= hi) [
        ? (a[i] <= a[j]) [
            tmp[k] := a[i];
            i := i + 1;
        ] : [
            tmp[k] := a[j];
            j := j + 1;
        ]
        k := k + 1;
    ]
    * (i <= mid) [
        tmp[k] := a[i];
        i := i + 1;
        k := k + 1;
    ]
    * (j <= hi) [
        tmp[k] := a[j];
        j := j + 1;
        k := k + 1;
    ]
    * c :: lo..hi [
        a[c] := tmp[c];
    ]
    ^ 0;
]

@msort(a[], tmp[], lo, hi) [
    ? (lo < hi) [
        mid := (lo + hi) / 2;
        msort(a, tmp, lo, mid);
        msort(a, tmp, mid + 1, hi);
        merge(a, tmp, lo, mid, hi);
    ]
    ^ 0;
]

@merge_sort(a[], n) [
    tmp := arr(n);
    msort(a, tmp, 0, n - 1);
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
    merge_sort(sorted, 5);
    print_all(sorted, 5);

    reverse := <9, 7, 5, 3, 1>;
    merge_sort(reverse, 5);
    print_all(reverse, 5);

    mixed := <5, 2, 9, 1, 5, 6>;
    merge_sort(mixed, 6);
    print_all(mixed, 6);

    dups := <4, 2, 4, 1, 2, 4>;
    merge_sort(dups, 6);
    print_all(dups, 6);

    one := <7>;
    merge_sort(one, 1);
    print_all(one, 1);

    two := <2, 1>;
    merge_sort(two, 2);
    print_all(two, 2);
    ^ 0;
]
