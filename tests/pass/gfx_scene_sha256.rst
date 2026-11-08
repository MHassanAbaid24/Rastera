# A small scene of rectangles, lines (including clipped ones) and a pixel, checked as a
# whole by sha256 of out.ppm. The hash comes from an independent reference renderer.
# expect-ppm-size: 16 12
# expect-ppm-sha256: 2b025b574da03ef06d9b51c337c08f2df8c95cd527f20abcb1659078dd3c65e9
@main() [
    canvas(16, 12);
    rect(2, 2, 8, 5, rgb(200, 100, 50));
    line(0, 0, 15, 11, rgb(255, 255, 255));
    line(15, 0, 0, 11, rgb(0, 255, 255));
    line(3, 10, 12, 1, rgb(255, 0, 255));
    line(-4, 5, 20, 9, rgb(1, 2, 3));
    rect(12, 9, 10, 10, rgb(0, 0, 255));
    rect(-2, -2, 4, 3, rgb(9, 9, 9));
    pixel(7, 7, rgb(255, 255, 0));
    export();
    ^ 0;
]
