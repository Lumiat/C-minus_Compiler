# 测试词法分析器

内容：

- `scan/scanner_test.c`：单文件扫描器测试驱动。
- `scan/run_tests.c`：扫描器批量测试运行器。
- `scan/inputs/`：若干待测的 C‑minus 源文件（扩展名 `.cm`）。
- `scan/expected/`：每个输入对应的期望输出，格式与 `printToken()` 的输出一致。
- `parse/run_tests.c`：语法分析器批量测试运行器。
- `parse/inputs/`：语法测试输入文件。
- `parse/expected/`：语法测试期望输出文件。

运行方法（在项目根目录）：

1. 使用 Makefile 的 C 测试运行器（推荐）：

```bat
make test-scan    # 构建并运行扫描器批量测试
make test-parse   # 构建并运行语法分析器批量测试
```

批量运行器会分别在 `test/scan/outputs/` 和 `test/parse/outputs/` 生成每个测试的输出，并与对应的 `expected/` 目录比较，打印 `PASS`/`FAIL`。

2. 单文件快速测试（扫描器）：

```bat
make test_file FILE=test/scan/inputs/simple_tokens.cm
```

3. 失败诊断：

查看 `test/scan/outputs/*.out` 或 `test/parse/outputs/*.out` 并与 `test/*/expected/*.expected` 对比差异。

说明：测试覆盖关键词、标识符、数字、注释、运算符、多字符运算符、分界符以及错误字符等常见词法情形。
