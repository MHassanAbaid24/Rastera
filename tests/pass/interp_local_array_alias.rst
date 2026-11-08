# expect-exit: 0
# expect-stdout: 9
@local_alias() [
    source := <1>;
    alias := source;
    alias[0] := 9;
    print(source[0]);
    ^ 0;
]
@main() [
    ^ local_alias();
]
