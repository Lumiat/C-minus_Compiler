# C-minus Compiler

This project implements the lexical, syntax, and semantic analysis stages of a
C-minus compiler. Tests are separated by responsibility so that each stage is
validated with inputs satisfying the preconditions of that stage.

A semantically valid C-minus program must declare its entry function as
`int main(void)`.

```text
C-minus_Compiler/
├── compiler.c / compiler.h   # Complete compiler pipeline
├── scan.c / scan.h           # Lexical analyzer
├── parse.c / parse.h         # Syntax analyzer
├── analyze.c / analyze.h     # Semantic analyzer
├── globals.c / globals.h     # Shared compiler state and types
├── util.c / util.h           # AST and output helpers
└── test/
    ├── test_runner.c / .h    # Shared test-runner utilities
    ├── scan/                 # Token recognition and lexical errors
    ├── parse/                # Grammar and syntax errors
    ├── analyze/              # Semantic rules and type checking
    └── compiler/             # End-to-end compiler behavior
```

## Test responsibilities

- `test/scan` invokes the scanner and verifies valid tokens and lexical errors.
- `test/parse` uses only lexically valid programs and verifies syntax trees or
  syntax diagnostics. Its runner rejects fixtures containing lexical errors.
- `test/analyze` uses lexically and syntactically valid programs and verifies
  only semantic diagnostics. Its runner rejects invalid front-end fixtures.
- `test/compiler` verifies the complete lexical → syntax → semantic pipeline.

The complete compiler stops after the first failing stage. A lexical failure
prevents parsing and semantic analysis; a syntax failure prevents semantic
analysis.

## Commands

Run commands from the repository root:

```sh
make test-scan
make test-parse
make test-analyze
make test-compiler
make test
```

`make test-complier` is retained as a compatibility alias for
`make test-compiler`.

To inspect scanner output for one file:

```sh
make test_file FILE=test/scan/inputs/simple_tokens.cm
```

Each suite writes generated output to its own `outputs/` directory and compares
it with the versioned files under `expected/`.
