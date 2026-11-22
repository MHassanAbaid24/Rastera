# expect-exit: 7
# expect-stdout: 3
# Indexing is an ordinary expression statement; a parenthesised variable is a store target.
@main() [
    a := <1, 2>;
    a[0];
    a[1] + 1;
    (a)[0] := 7;
    print(a[1] + 1);
    ^ a[0];
]
