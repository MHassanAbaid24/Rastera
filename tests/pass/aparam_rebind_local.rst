# expect-stdout: 3
# expect-stdout: 5
# expect-stdout: 2
# expect-exit: 0
# Assigning a new array to an array parameter rebinds the parameter name only (like `b := a;`):
# the caller's array is not replaced. The store before the rebind reached the caller's array,
# the store after it (77) went to the new array.
@rebind(a[]) [
    a[0] := 5;
    a := arr(3);
    a[0] := 77;
    ^ a[2] + 3;
]
@main() [
    v := <1, 2>;
    print(rebind(v));
    print(v[0]);
    print(v[1]);
    ^ 0;
]
