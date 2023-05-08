# 哈工大《编译系统》实验 - 2023年春

## 实验内容

### Assignment 01

+ 了解使用 `Flex` 和 `Bison` 进行词法分析和语法分析
+ 通过语法分析将 `C--` 语言源文件转换成一棵语法分析树并打印出来

### Assignment 02

+ 通过对 `Assignment 01` 输出的语法分析树进行树上操作，实现 `C--` 语言的语义分析和类型检查
+ 针对 `17` 种可能的语义错误进行检查，并打印输出

### Assignment 03

+ 根据 `Assignment 01` 输出的语法分析树生成等价的中间代码
+ 生成的中间代码可通过 [IR虚拟机](https://ernestthepoet.github.io/ir-virtual-machine) 进行检验
