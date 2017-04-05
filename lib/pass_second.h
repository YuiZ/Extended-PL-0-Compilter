#pragma once

#include "pass_first.h"

/*目标代码生成_Object Code Producer*/
#define ObjCode_Max_Size 2048
extern int P_obj;
extern string ObjectCode[ObjCode_Max_Size];

void Obj_generate();
void obj_print();
void obj_print_toFile();

/*代码优化_Optimizer*/
#define Proc_Var_Size 32

struct BaseBlock {
    int begin;
    int end;

    bool Def_set[Proc_Var_Size];
    bool Use_set[Proc_Var_Size];
    bool In_set[Proc_Var_Size];
    bool Out_set[Proc_Var_Size];

    BaseBlock(int new_begin, int new_end);

    void print_sets(int P_vn);

    bool temp_isSaved(string tar, string *temp_regs, int p_tr);
    void temp_fill_table(Sym_table *tar_table, string t_name);
    void temp_count(Sym_table *owner_table);
};
struct SubProgram {
    Sym_table* table;
    int begin, end;//分程序起止点

    string Var_name[Proc_Var_Size];//该分程序下的局部变量
    int P_vn;

    BaseBlock** Blocks;//该分程序下的基本块
    int Blocks_Size;

    bool** Conflict;//冲突图

    string Var_glb[3];
    int P_vg;

    SubProgram(Sym_table *sp_table);

    int var_search(string tar);
    int block_search(int tar_begin);

    void set_DEF_USE();
    void set_IN_OUT();
    void print_blocks();
    void build_blocks();

    void tmp_reg_pre_distribute();
    void glb_reg_distribute();
};

extern int Block_sign[MidCode_Max_Size];
extern SubProgram* SubProg[32];
extern int P_sp;

void Opt_process();

/*通用寄存器堆_General Purpose Registers*/
#define Temp_Reg_Size 12

struct GPR {
    int gpr_base;//起始临时寄存器编号
    int gpr_number;//选用的临时寄存器数
    int gpr_pointer;//当前分配指针
    string *regs_pool;//当前分配的寄存器及其对应的临时变量

    string glb_pool[3];

    GPR(int base, int number);

    void GPR_refresh();
    string GPR_get_reg(string temp_name);
    void GPR_print();

    void GPR_addto_glb(string v1, string v2, string v3);
    string GPR_get_glb(string tar);
    string GPR_search_glb(string tar);
};

extern GPR *mips_reg;
