# pixel is a built-in name and cannot be a parameter.
# expect-error: using built-in name 'pixel' as a parameter name
@f(pixel) [
    ^ pixel;
]

@main() [
    ^ f(1);
]
