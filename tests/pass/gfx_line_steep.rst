# A steep line (|dy| > |dx|) from (0,0) to (2,6) takes one pixel per row:
# (0,0) (0,1) (1,2) (1,3) (1,4) (2,5) (2,6).
# expect-ppm-size: 3 7
# expect-ppm-pixel: 0 0 ff0000
# expect-ppm-pixel: 0 1 ff0000
# expect-ppm-pixel: 1 2 ff0000
# expect-ppm-pixel: 1 3 ff0000
# expect-ppm-pixel: 1 4 ff0000
# expect-ppm-pixel: 2 5 ff0000
# expect-ppm-pixel: 2 6 ff0000
# expect-ppm-pixel: 2 0 000000
# expect-ppm-pixel: 0 6 000000
@main() [
    canvas(3, 7);
    line(0, 0, 2, 6, rgb(255, 0, 0));
    export();
    ^ 0;
]
