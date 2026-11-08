# canvas sizes must be at least 1.
# expect-runtime-error: invalid canvas size
@main() [
    canvas(0, 10);
    ^ 0;
]
