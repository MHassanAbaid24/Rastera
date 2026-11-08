# canvas sizes must be at most 4096.
# expect-runtime-error: invalid canvas size
@main() [
    canvas(4097, 4);
    ^ 0;
]
