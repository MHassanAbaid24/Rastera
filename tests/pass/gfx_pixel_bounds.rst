# pixel sets one pixel, ignores coordinates outside the canvas (no wrap to the next row)
# and keeps only the low 24 bits of the colour.
# expect-ppm-size: 4 3
# expect-ppm-pixel: 1 2 ff0000
# expect-ppm-pixel: 3 0 00ff00
# expect-ppm-pixel: 0 0 000000
# expect-ppm-pixel: 0 1 000000
# expect-ppm-pixel: 0 2 000000
# expect-ppm-pixel: 2 1 123456
# expect-ppm-pixel: 3 2 000000
@main() [
    canvas(4, 3);
    pixel(1, 2, rgb(255, 0, 0));
    pixel(3, 0, rgb(0, 255, 0));
    pixel(4, 0, 16777215);
    pixel(-1, 0, 16777215);
    pixel(0, 3, 16777215);
    pixel(0, -1, 16777215);
    pixel(4, 2, 16777215);
    pixel(-2147483647, 2147483647, 16777215);
    pixel(2, 1, 16777216 + 1193046);
    export();
    ^ 0;
]
