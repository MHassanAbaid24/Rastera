# A horizontal line covers both endpoints and nothing around it.
# expect-ppm-size: 6 3
# expect-ppm-pixel: 0 1 ffffff
# expect-ppm-pixel: 2 1 ffffff
# expect-ppm-pixel: 5 1 ffffff
# expect-ppm-pixel: 0 0 000000
# expect-ppm-pixel: 3 2 000000
@main() [
    canvas(6, 3);
    line(0, 1, 5, 1, rgb(255, 255, 255));
    export();
    ^ 0;
]
