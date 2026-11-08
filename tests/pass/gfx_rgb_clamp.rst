# rgb packs (r << 16) | (g << 8) | b and clamps each channel to 0..255.
# expect-stdout: 66051
# expect-stdout: 16777215
# expect-stdout: 16711808
# expect-stdout: 0
# expect-stdout: 255
@main() [
    print(rgb(1, 2, 3));
    print(rgb(255, 255, 255));
    print(rgb(300, -5, 128));
    print(rgb(-1, -1, -1));
    print(rgb(0, 0, 1000));
    ^ 0;
]
