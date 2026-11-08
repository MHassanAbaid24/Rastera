## Task

T?? — <title>

## What changed, stage by stage

- Lexer:
- Parser:
- AST:
- Sema:
- Codegen / runtime:

## Example

```
<small Rastera program>
```

```llvm
<relevant part of `rasterc --ir` output>
```

## Checklist

- [ ] New `tests/pass` and `tests/fail` cases cover the feature and its errors
- [ ] `docs/LANGUAGE_SPEC.md` matches the implemented behaviour
- [ ] `README.pdf` regenerated if `README.md` changed (`python3 docs/build_readme_pdf.py`)
