# rect before any canvas is a runtime error.
# expect-runtime-error: no canvas
@main() [
    rect(0, 0, 3, 3, 16777215);
    ^ 0;
]
