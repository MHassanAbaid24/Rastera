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
