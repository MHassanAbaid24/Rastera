# A line whose endpoints are equal draws exactly that one pixel.
# expect-ppm-size: 3 3
# expect-ppm-pixel: 1 1 ff8000
# expect-ppm-pixel: 0 0 000000
# expect-ppm-pixel: 2 2 000000
# expect-ppm-pixel: 1 0 000000
# expect-ppm-pixel: 1 2 000000
@main() [
    canvas(3, 3);
    line(1, 1, 1, 1, rgb(255, 128, 0));
    export();
    ^ 0;
]
