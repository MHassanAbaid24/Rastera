# Graphics demo: a 64x64 gradient with a white rectangle and two diagonal
# lines, written to out.ppm in the current directory.
@main() [
    canvas(64, 64);

    # Red grows to the right, green grows downwards, blue is constant.
    y := 0;
    * (y < 64) [
        x := 0;
        * (x < 64) [
            pixel(x, y, rgb(x * 4, y * 4, 128));
            x := x + 1;
        ]
        y := y + 1;
    ]

    rect(8, 8, 20, 12, rgb(255, 255, 255));
    line(0, 0, 63, 63, rgb(255, 0, 0));
    line(0, 63, 63, 0, rgb(0, 255, 0));
    export();
    ^ 0;
]
