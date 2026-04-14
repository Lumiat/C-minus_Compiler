```
C-minus_Compiler
├─ 📄ANALYZE.C      # 语义分析实现，做符号表构建和类型检查，发现未声明变量等错误
├─ 📄ANALYZE.H      # 配套 ANALYZE.C，语义分析实现接口
├─ 📄CGEN.C         # 代码生成器实现，遍历语法树，把语句和表达式翻译成 TM 指令
├─ 📄CGEN.H         # 配套 CGEN.C，代码生成器接口
├─ 📄CODE.C         # TM 代码输出工具
├─ 📄CODE.H         # TM 指令发射接口与寄存器约定定义
├─ 📄DOC.md         # 工作记录（方便报告书写）
├─ 📄GLOBALS.H      # 定义 C-minus 的 Token 类型和语法树节点结构
├─ 📄PARSE.C        # 手写递归下降语法分析器实现
├─ 📄PARSE.H        # 配套 PARSE.C，语法分析器接口
├─ 📄README.md      # README
├─ 📄SCAN.C         # 手写词法分析器实现
├─ 📄SCAN.H         # 配套 SCAN.C，词法分析器接口
├─ 📄STMTAB.C       # 符号表实现
├─ 📄STMTAB.H       # 配套 STMTAB.C，符号表接口
├─ 📄UTIL.C         # 通用工具实现
└─ 📄UTIL.H         # 配套 UTIL.C，通用工具接口
```
