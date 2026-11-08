# The proposal section 5 demo (examples/gradient.rst), checked by sha256 of out.ppm. The hash
# was confirmed against an independent model of the picture.
# expect-ppm-size: 256 256
# expect-ppm-sha256: cc3c3f4999df5e7d1a18c50bb9683859bba7dbd4dc6f06cd61e56b74acdff4ef
@main() [
    canvas(256, 256);
    * y :: 0..255 [
         * x :: 0..255 [
             shade := (x * x + y * y) % 255;
             ? (shade > 128) [
                 pixel(x, y, rgb(255, 255, 255));
             ] : [
                 pixel(x, y, rgb(20, 20, 20));
             ]
         ]
    ]
    export();
    ^ 0;
]
