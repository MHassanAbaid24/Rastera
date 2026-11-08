# A negative canvas height is rejected.
# expect-runtime-error: invalid canvas size
@main() [
    canvas(10, -1);
    ^ 0;
]
