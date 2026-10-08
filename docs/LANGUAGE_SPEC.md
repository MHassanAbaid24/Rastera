# Rastera Language Specification

Status: **draft v0.1**. Any change to this file is a team decision and goes through a PR like code.
Each rule names the task (`Txx`) that implements it; until that task is merged the rule is a promise, not a fact.

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

A variable's type is fixed by its first assignment. Comparisons and logical operators produce `i32` `0` or `1`.
In a condition, any non-zero `i32` is true. Arrays are never conditions or arithmetic operands.

## 4. Variables and scope (T02)

- Every `[ ... ]` block opens a new scope.
- `x := e;` — if `x` is visible in the current or any enclosing scope, it is **reassigned** (types must match);
  otherwise a **new** variable is created in the current scope.
- A variable created inside a block is not visible after the block ends.
- Function parameters live in the function's outermost scope. Functions cannot see each other's variables (no globals).

## 5. Statements

| Form | Meaning | Task |
|---|---|---|
| `x := e;` | create or reassign | T02 |
| `a[i] := e;` | store into array element | T09 |
| `e;` | evaluate expression for its side effects (calls) | T03 |
| `? (c) [ ... ]` / `? (c) [ ... ] : [ ... ]` | if / if-else; `: ? (c2) [...]` chains else-if | T05 |
| `* (c) [ ... ]` | while loop | T06 |
| `* i :: a..b [ ... ]` | range loop, inclusive | T08 |
| `^ e;` | return | T00 (literal), T07 (general) |

### Range loop rules (T08)

- `a` and `b` are evaluated **once**, before the first iteration, in that order.
- If `a > b` the body runs zero times.
- `i` is a new variable scoped to the loop body; assigning to it inside the body is a semantic error.
- The loop must terminate when `b` is `2147483647` (no overflow of the counter).

### Return (T00, T07)

Reaching the end of a function without `^` returns `0`.

## 6. Expressions

Precedence, lowest to highest:

| Level | Operators | Associativity | Task |
|---|---|---|---|
| 1 | `\|` | left | T04 |
| 2 | `&` | left | T04 |
| 3 | `== != < <= > >=` | non-associative (`a < b < c` is a syntax error) | T05 |
| 4 | `+ -` | left | T01 |
| 5 | `* / %` | left | T01 |
| 6 | unary `-`, `!` | right | T01, T04 |
| 7 | call `f(...)`, index `a[i]` | left | T03, T09 |

- `&` and `|` short-circuit: the right operand is evaluated only if needed (T04).
- `/` and `%` truncate toward zero (C semantics). Division or modulo by zero is a runtime error (T15).
- Operands are evaluated left to right.

## 7. Functions (T07, T10)

```
@name(p1, p2, buf[]) [ ... ]
```

- Parameters are `i32` unless written with `[]`, which makes them arrays passed **by reference**:
  stores through the parameter are visible to the caller.
- Functions return `i32` only. Arrays cannot be returned.
- Recursion is allowed. Calling with the wrong number or kind of arguments is a semantic error.
- Duplicate function names, duplicate parameter names and calls to unknown functions are semantic errors.

## 8. Arrays (T09, T10)

| Form | Meaning |
|---|---|
| `<1, 2, 3>` | new array with these elements (at least one) |
| `arr(n)` | new array of `n` zeros; `n < 0` is a runtime error |
| `a[i]` | read element |
| `a[i] := e;` | write element |
| `b := a;` | `b` refers to the **same** array (aliasing, no copy) |

Arrays are allocated on the heap and never freed before program exit (no garbage collector).
Index out of range is a runtime error (T15).

## 9. Built-in functions

| Call | Effect | Task |
|---|---|---|
| `print(x)` | print `x` and a newline to stdout; returns 0 | T03 |
| `canvas(w, h)` | create a `w`×`h` canvas filled with black | T11 |
| `rgb(r, g, b)` | returns `(r << 16) \| (g << 8) \| b`, each clamped to `0..255` | T11 |
| `pixel(x, y, c)` | set one pixel; out-of-canvas coordinates are ignored | T11 |
| `export()` | write the canvas to `out.ppm` (binary P6) | T11 |
| `line(x0, y0, x1, y1, c)` | Bresenham line | T12 |
| `rect(x, y, w, h, c)` | filled rectangle, clipped to canvas | T12 |

All built-ins return `i32`. Using a built-in name for a user function or variable is a semantic error.

## 10. Errors

- Compile-time errors are reported as `file:line: error: message` and the compiler exits with status 1.
- Runtime errors print `rastera: runtime error: message` to stderr and exit with status 2.

## 11. Implementation notes

- User function `f` is emitted as the symbol `rs_f`, so it can never clash with C library names
  (`abs`, `exit`, ...). The runtime's C `main` calls `rs_main`.
- Built-ins are implemented in C in `runtime/` and called from generated IR as `rt_<name>`.
