# expect-exit: 0
# recursion with a base case needs if (T05); this checks resolution and IR only
@spin(n) [
    ^ spin(n - 1);
]
@ping(n) [
    ^ pong(n);
]
@pong(n) [
    ^ ping(n);
]
@main() [
    ^ 0;
]
