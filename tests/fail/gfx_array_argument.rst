# Built-ins take integers only: an array is not a colour.
# expect-error: builtin 'rgb' expects an integer argument
@main() [
    a := <1, 2, 3>;
    print(rgb(a, 0, 0));
    ^ 0;
]
