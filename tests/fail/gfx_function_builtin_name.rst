# canvas is a built-in name and cannot be a function.
# expect-error: using built-in name 'canvas' as a function name
@canvas(w, h) [
    ^ w * h;
]

@main() [
    ^ 0;
]
