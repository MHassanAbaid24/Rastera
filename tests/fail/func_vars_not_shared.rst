# expect-error: undefined variable 'x'
@peek() [
    ^ x;
]
@main() [
    x := 1;
    ^ peek();
]
