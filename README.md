# Extended-PL-0 Compilter
简述 Introduction
---
这是一个针对一种扩充PL/0语法的简单编译器，可将读入的扩充PL/0源码编译为32位MIPS汇编程序。

Thi is a simple compiler for an extended PL/0 syntax. The object code is a subset belonging to 32-bit MIPS instructions. 

本项目使用C/C++代码编写。

This compiler is accomplished by C/C++.

程序结构及主要流程 Structure & Main Process
---
![compiler structure](structure.png)
```
参照上图，3遍扫描过程如下：
第1遍：扫描源程序，获得符号表与中间代码（两者在代码优化部分会有进一步完善，目标代码生成的）
    参与者：词法分析、语法分析、语义分析及中间代码生成、符号表、错误处理
    过程：
        （1）源程序代码读入;
        （2）进入语法分析程序，对源程序采用自顶向下递归的分析方式，调用[词法分析程序取词;
        （3）语义分析伴随语法分析过程中实现;
        （4）符号表伴随语法分析过程逐渐构建形成
    结果：基本完整的符号表和中间代码-四元式
	
第2遍：扫描中间代码，划分基本块并完善符号表
    参与者：符号表、代码优化、错误处理
    过程：
        （1）在中间代码上划分基本块
        （2）将中间代码包含的各子过程划分开，建立各子过程的基本块信息记录
        （3）对各子过程进行活跃变量分析和全局寄存器分配
        （4）对各子过程所用临时变量进行预先统计（实际分配在目标代码生成中完成），若出现临时寄存器不足的情况，则在符号表中添加伪寄存器的表项（即为临时寄存器预留空间），并调整四元式（将表示临时寄存器的_Tx置换为伪寄存器M_Tx）
    结果：符号表、中间代码

第3遍：扫描完善后的中间代码，结合符号表从而生成目标代码
    参与者：符号表、目标代码生成、错误处理
    过程：逐条读中间代码，生成实现相应功能的mips汇编代码
    结果：目标代码（MIPS汇编）
```
代码文件说明 Source Code Description
---
| 文件名 File Name | 说明 Description |
| :---: | :--- |
| pass_first.h | 第一遍相关的数据结构和公用函数定义 Functions and data structure declarations for the first pass|
| pass_second.h | 第二遍相关的数据结构和公用函数定义 Functions and data structure declarations for the second pass|
| yui_tools.h | 自定义工具函数 Custom tools|
| LexiconAnalysis.cpp | 词法分析 Lexical Analysis |
| SyntaxAnalysis.cpp | 语法分析 Syntactic Analysis | 
| SymbolTable.cpp | 符号表 Symbol Table |
| SemanticAnalysis.cpp | 语义分析 Semantic Analysis |
| Optimizer.cpp | 代码优化 Object Code optimizing | 
| ObjectCodeProducer.cpp | 目标代码生成 Object Code Generate |
| GeneralPurposeRegisters.cpp | 模拟MIPS通用寄存器堆 Simulate MIPS General Purpose Registers |
| ErrorHandler.cpp | 错误处理 Error Processing | 
| tools.cpp | 自定义工具实现 Custom tool functions|
| main.cpp | 主函数 Main function |
