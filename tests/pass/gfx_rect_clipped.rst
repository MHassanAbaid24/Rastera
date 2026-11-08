# A rectangle hanging off the top-left and one hanging off the bottom-right are clipped
# to the canvas; pixels just outside the visible part stay black.
# expect-ppm-size: 6 4
# expect-ppm-pixel: 0 0 ff0000
# expect-ppm-pixel: 1 1 ff0000
# expect-ppm-pixel: 2 0 000000
# expect-ppm-pixel: 0 2 000000
# expect-ppm-pixel: 4 2 0000ff
# expect-ppm-pixel: 5 3 0000ff
# expect-ppm-pixel: 3 3 000000
# expect-ppm-pixel: 4 1 000000
@main() [
    canvas(6, 4);
    rect(-5, -5, 7, 7, rgb(255, 0, 0));
    rect(4, 2, 100, 100, rgb(0, 0, 255));
    export();
    ^ 0;
]
