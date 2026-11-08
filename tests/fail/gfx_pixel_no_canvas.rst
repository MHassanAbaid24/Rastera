# pixel before any canvas is a runtime error.
# expect-runtime-error: no canvas
@main() [
    pixel(0, 0, 16777215);
    ^ 0;
]
