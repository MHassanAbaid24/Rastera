# A second canvas call replaces the first one: new size, black again.
# expect-ppm-size: 2 5
# expect-ppm-pixel: 0 0 000000
# expect-ppm-pixel: 1 4 000000
@main() [
    canvas(8, 8);
    pixel(0, 0, 16777215);
    canvas(2, 5);
    export();
    ^ 0;
]
