# canvas and export: both return 0; the file has a P6 header for 4x3 and starts black.
# expect-stdout: 0
# expect-stdout: 0
# expect-ppm-size: 4 3
# expect-ppm-pixel: 0 0 000000
# expect-ppm-pixel: 3 2 000000
@main() [
    print(canvas(4, 3));
    print(export());
    ^ 0;
]
