# expect-error: duplicate parameter 'a'
@f(a, a) [
    ^ a;
]
@main() [
    ^ 0;
]
