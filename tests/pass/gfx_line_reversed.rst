# A shallow line drawn right to left, from (6,2) to (0,0):
# (6,2) (5,2) (4,1) (3,1) (2,1) (1,0) (0,0).
# expect-ppm-size: 7 3
# expect-ppm-pixel: 6 2 00ff00
# expect-ppm-pixel: 5 2 00ff00
# expect-ppm-pixel: 4 1 00ff00
# expect-ppm-pixel: 3 1 00ff00
# expect-ppm-pixel: 2 1 00ff00
# expect-ppm-pixel: 1 0 00ff00
# expect-ppm-pixel: 0 0 00ff00
# expect-ppm-pixel: 0 2 000000
# expect-ppm-pixel: 6 0 000000
@main() [
    canvas(7, 3);
    line(6, 2, 0, 0, rgb(0, 255, 0));
    export();
    ^ 0;
]
