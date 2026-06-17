# C-Minus 编译器 —— 语法分析器实现讲稿

本文档用于课堂讲解，覆盖三个部分：语法分析步骤的实现、语法树的打印输出、以及测试流程。配套代码位于项目中，演示时请打开相应文件。

## 目录

- 实现概览
- 语法分析步骤（递归下降实现）
- 语法树打印与输出
- 测试与运行

---

## 实现概览

- 主要文件：[parse.h](parse.h)、[parse.c](parse.c)、[util.c](util.c)、[scan.c](scan.c)。
- 入口函数：`parse()`（定义于 `parse.c`），先调用 `getToken()` 初始化记号流，再调用 `declaration_list()` 构建顶层 AST。
- AST 节点构造由 `util.c` 中的 `newDeclNode`、`newStmtNode`、`newExpNode` 完成，打印由 `printTree()` 实现。

## 语法分析步骤（递归下降实现）

实现采用经典递归下降解析器：每个语法产生式对应一个函数，按自顶向下顺序调用。解析器使用全局变量 `token` 保存当前记号，`match(expected)` 用于匹配并消费记号。

关键点：

- 通过 `token` 做判断与预读（lookahead）来区分变量声明与函数声明（例如在 `declaration()` 中通过检测 `LPAREN` 判定）。
- 出错处理通过 `syntaxError()` 报告后使用循环跳到同步符号以继续解析（典型同步集合包括 `;`、`)`、`}`、`ENDFILE`）。

下面给出讲解时应展示的核心代码片段（选自 `parse.c`），便于逐行讲解：

### `parse()`（入口）

```c
/* primary parse entry */
TreeNode *parse(void)
{
    TreeNode *t;
    token = getToken();
    t = declaration_list();
    if (token != ENDFILE)
        syntaxError("Code ends before file\n");
    return t;
}
```

说明：初始化 `token`，调用 `declaration_list()` 解析整个文件，如果文件未结束报告语法错误。

### `match()` 与 `syntaxError()`（记号驱动与错误报告）

```c
static void syntaxError(char *message)
{
    fprintf(listing, "\n>>> ");
    fprintf(listing, "Syntax error at line %d: %s", lineno, message);
    Error = TRUE;
}

static void match(TokenType expected)
{
    if (token == expected)
        token = getToken();
    else
    {
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        fprintf(listing, "      ");
    }
}
```

说明：`match` 用于消费预期的记号，若不匹配则调用 `syntaxError` 并打印当前记号（便于调试与展示）。

### 声明解析（`declaration()`）示例 — 区分变量与函数

```c
TreeNode *declaration(void)
{
    TreeNode *t = NULL;
    TreeNode *typeNode = NULL;
    char *idname = NULL;

    typeNode = type_specifier(); /* consume INT/VOID */

    if (token != ID) { /* 错误恢复略 */ }

    idname = copyString(tokenString);
    match(ID);

    if (token == LPAREN)
        t = fun_declaration(idname, typeNode);
    else
        t = var_declaration(idname, typeNode);

    return t;
}
```

说明：通过查看下一个记号（`LPAREN`）判别后续是函数声明还是变量声明。

### 表达式解析（`expression()` / `factor()`）示例 — 常见策略

```c
/* expression -> var = expression | simple_expression */
TreeNode *expression(void)
{
    TreeNode *t = simple_expression();
    if (token == ASSIGN)
    {
        if (t != NULL && t->nodekind == ExpK &&
            (t->kind.exp == IdK || t->kind.exp == ArrIdK))
        {
            TreeNode *p = newExpNode(AssignK);
            p->child[0] = t;
            match(ASSIGN);
            p->child[1] = expression();
            t = p;
        }
        else
        {
            syntaxError("illegal assignment");
        }
    }
    return t;
}

/* factor -> ( expression ) | var | call | NUM */
TreeNode *factor(void)
{
    TreeNode *t = NULL;
    switch (token)
    {
    case LPAREN:
        match(LPAREN);
        t = expression();
        match(RPAREN);
        break;
    case ID:
        /* 决定是函数调用还是变量/数组访问 */
        /* 保存名字，调用 match(ID)，再根据下一个 token 判别 */
        break;
    case NUM:
        t = newExpNode(ConstK);
        t->attr.val = atoi(tokenString);
        match(NUM);
        break;
    default:
        syntaxError("unexpected token in factor");
        token = getToken();
        break;
    }
    return t;
}
```

说明：表达式采用自下而上的优先级分层（`simple_expression` → `additive_expression` → `term` → `factor`），并在 `factor` 中通过记号查看实现调用/数组访问/标识符与常量的区分。

## 语法树的打印输出

- 打印函数：`printTree(TreeNode *tree)`（定义在 [util.c](util.c)），内部通过 `printTreeRec(tree, indent)` 递归输出，按节点种类区分打印标签（`FuncK`、`Var_DeclK`、`If`、`Op:`、`ConstK`、`IdK` 等）。
- 打印使用缩进（传入 `indent`）来反映树的层次结构，便于课堂直观展示。打印算子时会调用 `printToken()` 输出人类可读的操作符文本。

演示要点：运行解析后打印出的 `Syntax tree:` 段落是理解 AST 结构的关键输出。

## 测试与运行

- 测试代码位于 [test/parse/](test/parse/)。核心测试驱动是 `test/parse/run_tests.c`：它会为 `tests.list` 中列出的每个用例加载 `inputs/*.cm`，调用 `parse()`，并将 `printTree()` 输出写入 `outputs/`；最后与 `expected/` 中的期望输出比较。
- 在项目根目录编译并运行（示例命令）：

```bash
gcc -I. parse.c scan.c util.c test/parse/run_tests.c -o run_parse_tests
./run_parse_tests
```

在 Windows 上使用对应的 `gcc` 可执行文件与 `.exe` 运行。

---

如果需要，我可以：

- 将该讲稿生成为 PPT 要点（每张幻灯片一节）；
- 或者把讲稿内容进一步精简成课堂口头串词。

文件已保存为 `PARSER_EXPLAINER.md` 于项目根目录。
