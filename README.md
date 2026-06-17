```
C-minus_Compiler
├─ .vscode/
├─ test/
│  ├─ scan/
│  │  ├─ inputs/
│  │  ├─ expected/
│  │  ├─ outputs/
│  │  ├─ scanner_test.c
│  │  └─ run_tests.c
│  └─ parse/
│     ├─ inputs/
│     ├─ expected/
│     ├─ outputs/
│  │  ├─ parser_test.c
│     └─ run_tests.c
├─ globals.h
├─ mermaid-diagram.png
├─ parse.c
├─ parse.h
├─ README.md
├─ scan.c
├─ scan.h
├─ util.c
└─ util.h
```

## Testing

This repository includes C test runners for the scanner and parser under `test/scan` and `test/parse` respectively. Use the provided Makefile targets from the project root:

```bat
make test-scan   # build and run scanner batch tests
make test-parse  # build and run parser batch tests
```

You can also run the single-file scanner test against a specific input with:

```bat
make test_file FILE=test/scan/inputs/simple_tokens.cm
```

Test outputs are written to `test/scan/outputs/` and `test/parse/outputs/` and compared automatically with the corresponding `expected/` files.
