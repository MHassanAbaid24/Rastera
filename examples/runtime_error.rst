# Runtime error: valid output first, then an out-of-range array read.
@main() [
    values := <10, 20>;
    print(values[1]);
    ^ values[2];
]
