# A line that starts and ends outside the canvas is clipped per pixel: the part inside
# is drawn, nothing wraps around, and an entirely outside line draws nothing.
# expect-ppm-size: 5 5
# expect-ppm-pixel: 0 0 ffffff
# expect-ppm-pixel: 2 2 ffffff
# expect-ppm-pixel: 4 4 ffffff
# expect-ppm-pixel: 1 0 000000
# expect-ppm-pixel: 4 0 000000
# expect-ppm-pixel: 0 4 000000
@main() [
    canvas(5, 5);
    line(-3, -3, 8, 8, 16777215);
    line(-100, 2, -10, 2, 16777215);
    line(10, 0, 10, 100, 16777215);
    export();
    ^ 0;
]
