# export takes no arguments.
# expect-error: wrong argument count for 'export': expected 0, got 1
@main() [
    canvas(4, 4);
    export(1);
    ^ 0;
]
