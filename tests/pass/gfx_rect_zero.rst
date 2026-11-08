# A rectangle with width or height <= 0 draws nothing: the file is the header plus 27 zero bytes.
# expect-ppm-size: 3 3
# expect-ppm-sha256: adbed3dcb9ea288ca18a53e93022d69f3bf9398a19effe7a289f32d2789b7234
@main() [
    canvas(3, 3);
    rect(0, 0, 0, 3, 16777215);
    rect(0, 0, 3, 0, 16777215);
    rect(0, 0, -2, 3, 16777215);
    rect(0, 0, 3, -1, 16777215);
    rect(1, 1, 0, 0, 16777215);
    export();
    ^ 0;
]
