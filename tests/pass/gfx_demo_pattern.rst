# examples/pattern.rst (nested rects, a fan of lines, a frame), checked by sha256 of out.ppm.
# The hash was confirmed against an independent Python renderer.
# expect-ppm-size: 128 128
# expect-ppm-pixel: 0 0 ffffff
# expect-ppm-pixel: 64 127 ffffff
# expect-ppm-sha256: 02c039e36a8310591e1499af700b59ac042c40efbd0e9fd3b57f7d4a47f0f4cb
# Generative pattern: nested filled squares, then a fan of lines.
# A 128x128 canvas is written to out.ppm in the current directory.
@main() [
    canvas(128, 128);

    # Eight concentric squares, outermost first; each is 16 pixels smaller
    # than the one before and shifts from blue-green towards red-green.
    * i :: 0..7 [
        rect(i * 8, i * 8, 128 - i * 16, 128 - i * 16, rgb(i * 32, 255 - i * 28, 160 - i * 16));
    ]

    # Seventeen lines from the bottom centre to evenly spaced points on the top edge.
    * k :: 0..16 [
        line(64, 127, k * 8, 0, rgb(255, k * 15, 0));
    ]

    # A white frame around the canvas.
    line(0, 0, 127, 0, rgb(255, 255, 255));
    line(127, 0, 127, 127, rgb(255, 255, 255));
    line(127, 127, 0, 127, rgb(255, 255, 255));
    line(0, 127, 0, 0, rgb(255, 255, 255));
    export();
    ^ 0;
]
