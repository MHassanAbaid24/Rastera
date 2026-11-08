# A rejected canvas call stops the program (exit 2); later statements do not run.
# expect-stdout: 5
# expect-runtime-error: invalid canvas size
@main() [
    canvas(4, 4);
    print(5);
    canvas(4, 5000);
    print(6);
    ^ 0;
]
