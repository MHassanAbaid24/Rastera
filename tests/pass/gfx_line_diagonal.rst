# Both diagonals of a 5x5 canvas, one drawn left to right and one right to left.
# expect-ppm-size: 5 5
# expect-ppm-pixel: 0 0 ffffff
# expect-ppm-pixel: 2 2 ffffff
# expect-ppm-pixel: 4 4 ffffff
# expect-ppm-pixel: 4 0 ffffff
# expect-ppm-pixel: 0 4 ffffff
# expect-ppm-pixel: 1 0 000000
# expect-ppm-pixel: 2 1 000000
@main() [
    canvas(5, 5);
    line(0, 0, 4, 4, 16777215);
    line(4, 0, 0, 4, 16777215);
    export();
    ^ 0;
]
