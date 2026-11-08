# A vertical line drawn bottom-up (reversed endpoints) covers both endpoints.
# expect-ppm-size: 3 5
# expect-ppm-pixel: 1 0 0000ff
# expect-ppm-pixel: 1 2 0000ff
# expect-ppm-pixel: 1 4 0000ff
# expect-ppm-pixel: 0 2 000000
# expect-ppm-pixel: 2 2 000000
@main() [
    canvas(3, 5);
    line(1, 4, 1, 0, rgb(0, 0, 255));
    export();
    ^ 0;
]
