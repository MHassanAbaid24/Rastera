# expect-error: logical operands must be integers
@main() [
    values := <1, 2>;
    ^ values & 1;
]
