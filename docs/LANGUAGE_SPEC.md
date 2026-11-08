# Rastera Language Specification

Status: **draft v0.1**. Any change to this file is a team decision and goes through a PR like code.
Each rule names the task (`Txx`) that introduces it; rules marked **implemented** are facts.

## 1. Lexical structure

| Element | Rule |
|---|---|
| Comment | `#` to end of line |
| Whitespace | spaces, tabs, newlines; insignificant |
| Identifier | `[A-Za-z_][A-Za-z0-9_]*` |
| Integer literal | `[0-9]+`, must fit in signed 32-bit (`0 .. 2147483647`); negative values come from unary minus |
| Reserved names | `arr`, and the built-in function names in §8 |

Symbols: `@ ^ * ? : :: .. := ( ) [ ] < > , ; + - / % == != <= >= & | !`

`*` is both multiplication and the loop marker; `<`/`>` are both comparisons and array-literal brackets.
The grammar disambiguates by position (statement start vs. inside an expression).

## 2. Program structure

A program is a sequence of function definitions. Exactly one must be `@main()` with no parameters;
its return value is the process exit code. Functions may be called before they are defined (T07).

```
@main() [
    ^ 0;
]
```

## 3. Types

Two types, never written by the programmer:

- `i32` — signed 32-bit integer. Arithmetic wraps on overflow (two's complement).
- `array` — reference to a heap-allocated, fixed-length sequence of `i32`.

A variable's type is fixed by its first assignment. Comparisons and logical operators produce `i32` `0` or `1`. Logical operators require `i32` operands; arrays are never conditions or operands.
In a condition, any non-zero `i32` is true. Arrays are never operands of arithmetic, comparison or unary
operators: each is the semantic error `operands must be integers` (`operand must be an integer` for unary `-`).

## 4. Variables and scope (T02)

- Every `[ ... ]` block opens a new scope.
- `x := e;` — if `x` is visible in the current or any enclosing scope, it is **reassigned** (types must match);
  otherwise a **new** variable is created in the current scope.
- A variable created inside a block is not visible after the block ends.
- Using a variable that is not visible is a compile error `undefined variable 'x'` at the line of the use.
- Function parameters live in the function's outermost scope. Functions cannot see each other's variables (no globals).

## 5. Statements

| Form | Meaning | Task |
|---|---|---|
| `x := e;` | create or reassign | T02 |
| `a[i] := e;` | store into array element | T09 |
| `e;` | evaluate expression for its side effects (calls) | T03 |
| `? (c) [ ... ]` / `? (c) [ ... ] : [ ... ]` | if / if-else; `: ? (c2) [...]` chains else-if | T05 |
| `* (c) [ ... ]` | while loop; checks `c` before every iteration and runs the body while `c` is non-zero | T06 |
| `* i :: a..b [ ... ]` | range loop, inclusive; runs the body for `i = a, a+1, ..., b` | T08 |
| `^ e;` | return | T00 (literal), T07 (general) |

### While loop rules (T06)

- `c` must be an `i32`; an array condition is a compile error.
- The body is a block with its own lexical scope.
- `^` in the body returns from the enclosing function.

### Range loop rules (T08, implemented)

- `a` and `b` must be `i32`; an array bound is a compile error (`range bound must be an integer`).
- `a` and `b` are evaluated **once**, before the first iteration, in that order (left to right).
- The loop is inclusive: the body runs for `i = a, a+1, ..., b`. If `a > b` it runs zero times.
- `i` is a new `i32` variable scoped to the loop; it is not visible after the loop. If a variable named
  `i` is already visible, the loop variable **hides** it for the whole loop (bounds are evaluated before the
  loop variable exists, so `* i :: i..i + 1` starts from the outer `i`) and the outer variable is unchanged
  afterwards. This is the only place a name is shadowed: assignment `x := e;` still reassigns a visible `x` (§4).
- `i` is read-only: any assignment to it inside the body, including from nested blocks and loops, is a compile
  error `cannot assign to loop variable 'i'`. Built-in names cannot be loop variables.
- The loop terminates when `b` is `2147483647`: `i` is compared with `b` before it is incremented, so the
  counter never overflows.
- The body is a block with its own scope, so variables declared in it are new on every iteration. `^` in the
  body returns from the enclosing function.
- Syntax: `*` followed by an identifier and `::` is a range loop, `*` followed by `(` is a while loop. After
  `..`, a `[` that is not inside parentheses opens the body, so an upper bound that ends in an index must be
  parenthesised: `* i :: 0..(v[2]) [ ... ]` (`0..v[2] [ ... ]` is a syntax error).

### Return (T00, T07)

Reaching the end of a function without `^` returns `0`.

## 6. Expressions

Precedence, lowest to highest:

| Level | Operators | Associativity | Task |
|---|---|---|---|
| 1 | `\|` | left | T04 (implemented) |
| 2 | `&` | left | T04 (implemented) |
| 3 | `== != < <= > >=` | non-associative (`a < b < c` is a syntax error) | T05 |
| 4 | `+ -` | left | T01 |
| 5 | `* / %` | left | T01 |
| 6 | unary `-`, `!` | right | T01, T04 (implemented) |
| 7 | call `f(...)`, index `a[i]` | left | T03, T09 |

- `&` and `|` short-circuit: the right operand is evaluated only if needed. They normalize their result to `0` or `1`; `!x` is `1` exactly when `x` is `0` (T04).
- `/` and `%` truncate toward zero (C semantics). A zero divisor and `-2147483648 / -1` or `-2147483648 % -1` are runtime errors (T15).
- Operands are evaluated left to right.

## 7. Functions (T07, T10)

```
@name(p1, p2) [ ... ]
@name(p1, buf[]) [ ... ]
```

- `@main` takes no parameters.
- A plain parameter `p` is a mutable `i32` variable local to the call (assigning it never changes the caller's value).
- A parameter written with `[]` (`buf[]`) is an **array passed by reference** (T10, implemented): it refers to
  the caller's array, so an element store `buf[i] := e;` is visible to the caller after the call. Nothing is
  copied. The length travels with the array, so `buf[i]` is bounds-checked like any other index (§8).
- An array parameter can be indexed, stored into, assigned to another variable (`b := buf;` aliases, §8), and
  passed on to another function. Its only use as a range-loop bound is through indexing (`0..(buf[2])`).
- Assigning a whole array to an array parameter (`buf := arr(3);`, `buf := other;`) **rebinds the parameter name
  only**, exactly like reassigning any array variable: later uses of `buf` in this call refer to the new array,
  the caller's array and the caller's variable are unchanged. Stores made before the rebind stay visible to the
  caller. Assigning an `i32` to an array parameter, or an array to an `i32` parameter, is a type mismatch (§3).
- An argument must match its parameter's kind: a plain parameter takes an `i32` expression and an array
  parameter takes an array expression (a variable, `<...>` or `arr(n)`). Passing an `i32` where an array is
  expected, or an array where an `i32` is expected, is a semantic error:
  `function 'f' expects an array for argument 2, got an integer` (and `expects an integer ... got an array`).
  Built-ins take `i32` only (§9).
- Functions return `i32` only. An array cannot be returned (`^ buf;` is a semantic error).
- Recursion is allowed. Calling with the wrong number of arguments is a semantic error.
- Duplicate function names, duplicate parameter names and calls to unknown functions are semantic errors.

## 8. Arrays (T09)

| Form | Meaning |
|---|---|
| `<1, 2, 3>` | new array with these elements (at least one) |
| `arr(n)` | new array of `n` zeros; `n < 0` is a runtime error (T15) |
| `a[i]` | read element |
| `a[i] := e;` | write element |
| `b := a;` | `b` refers to the **same** array (aliasing, no copy) |

Arrays are allocated on the heap and never freed before program exit (no garbage collector).
Indexing requires `0 <= i < length`; an out-of-range read or write is a runtime error (T15).

## 9. Built-in functions

| Call | Effect | Task |
|---|---|---|
| `print(x)` | print `x` and a newline to stdout; returns 0 | T03 |
| `canvas(w, h)` | create a `w`×`h` canvas filled with black; calling it again replaces the canvas; returns 0 | T11 |
| `rgb(r, g, b)` | returns `(r << 16) \| (g << 8) \| b`, each channel clamped to `0..255` | T11 |
| `pixel(x, y, c)` | set one pixel to colour `c`; out-of-canvas coordinates are ignored; returns 0 | T11 |
| `export()` | write the canvas to `out.ppm` in the current directory (binary P6); returns 0 | T11 |
| `line(x0, y0, x1, y1, c)` | Bresenham line, both endpoints inclusive; returns 0 | T12 |
| `rect(x, y, w, h, c)` | filled rectangle, clipped to the canvas; returns 0 | T12 |

All built-ins take and return `i32`. The argument count is checked at compile time (as for `print`)
and every argument must be an integer. Using a built-in name for a user function, parameter or
variable is a semantic error.

Graphics rules:

- **Canvas.** There is at most one canvas at a time. `w` and `h` must each be in `1..4096`;
  otherwise the program stops with the runtime error `invalid canvas size`. Every pixel starts black.
  `canvas` called a second time discards the old image and starts a new black one.
- **No canvas.** `pixel`, `line`, `rect` and `export` before the first `canvas` call stop the program
  with the runtime error `no canvas`.
- **Colours.** A colour is an `i32` read as `0xRRGGBB`: only its low 24 bits are used (red is bits
  16-23, green bits 8-15, blue bits 0-7). `rgb` builds such a value; any other integer is also a
  valid colour.
- **Coordinates.** `(0, 0)` is the top-left pixel, `x` grows to the right and `y` downwards. A pixel
  outside `0 <= x < w`, `0 <= y < h` is silently skipped; this clipping applies to every pixel of
  `line` and `rect`, so shapes may start or end outside the canvas.
- **`line`.** Integer Bresenham for all octants: it draws `max(|x1-x0|, |y1-y0|) + 1` pixels,
  including both endpoints; `x0 == x1 and y0 == y1` draws one pixel. Arithmetic is done in 64 bits,
  so extreme coordinates do not overflow.
- **`rect`.** Fills the pixels `x <= px < x + w`, `y <= py < y + h` that lie inside the canvas;
  `w <= 0` or `h <= 0` draws nothing.
- **`export`.** Writes `out.ppm` (replacing any existing file) as binary PPM: the header is exactly
  `P6\n<w> <h>\n255\n`, followed by `w*h` pixels of three bytes (R, G, B), row by row from the
  top-left. The canvas stays usable afterwards. If the file cannot be written the runtime error
  `cannot write out.ppm` is reported. Both implementations write byte-identical files for the same
  program.

## 10. Errors

- Compile-time errors are reported as `file:line: error: message` and the compiler exits with status 1.
- Runtime errors print `rastera: runtime error: message` to stderr and exit with status 2.

## 11. Implementations

`rasteri` is the Phase 1 Flex/Bison interpreter: its grammar builds execution
nodes and executes `@main` after static checking. `rasterc` is the Phase 2 AOT
compiler: it builds an AST, emits LLVM IR, and produces a native executable.
Both implement this language and are checked by the same test suite.

## 12. Implementation notes

- User function `f` is emitted as the symbol `rs_f`, so it can never clash with C library names
  (`abs`, `exit`, ...). The runtime's C `main` calls `rs_main`.
- Built-ins are implemented in C in `runtime/` and called from generated IR as `rt_<name>`.
