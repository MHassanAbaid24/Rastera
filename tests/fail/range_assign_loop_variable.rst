# expect-error: cannot assign to loop variable 'i'
@main() [
    * i :: 1..3 [
        i := i + 1;
    ]
    ^ 0;
]
