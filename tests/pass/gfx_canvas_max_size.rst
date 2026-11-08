# The largest allowed canvas is 4096 x 4096.
# expect-ppm-size: 4096 1
# expect-ppm-pixel: 4095 0 0a0b0c
@main() [
    canvas(4096, 4096);
    canvas(4096, 1);
    pixel(4095, 0, 658188);
    export();
    ^ 0;
]
