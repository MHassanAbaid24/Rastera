# export before any canvas is a runtime error; output printed before it is kept.
# expect-stdout: 1
# expect-runtime-error: no canvas
@main() [
    print(1);
    export();
    ^ 0;
]
