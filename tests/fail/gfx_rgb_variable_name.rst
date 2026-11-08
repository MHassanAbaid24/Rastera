# rgb is a built-in name and cannot be a variable.
# expect-error: using built-in name 'rgb' as a variable name
@main() [
    rgb := 5;
    ^ 0;
]
