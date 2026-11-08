# line takes five arguments.
# expect-error: wrong argument count for 'line': expected 5, got 4
@main() [
    canvas(4, 4);
    line(0, 0, 3, 3);
    ^ 0;
]
