# expect-error: error: undefined variable 'branch_var'
@main() [
    ? (1) [
        branch_var := 42;
    ]
    ^ branch_var;
]
