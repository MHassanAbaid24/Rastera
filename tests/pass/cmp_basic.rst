# expect-exit: 6
@main() [
    count := 0;
    ? (5 == 5) [ count := count + 1; ]
    ? (5 != 4) [ count := count + 1; ]
    ? (3 < 5) [ count := count + 1; ]
    ? (5 <= 5) [ count := count + 1; ]
    ? (6 > 5) [ count := count + 1; ]
    ? (6 >= 6) [ count := count + 1; ]
    ^ count;
]
