#pragma once

#include "pass_first.h"

/*Ŀ���������_Object Code Producer*/
#define ObjCode_Max_Size 2048
extern int P_obj;
extern string ObjectCode[ObjCode_Max_Size];

void Obj_generate();
void obj_print();
void obj_print_toFile();

/*�����Ż�_Optimizer*/
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
	int begin, end;//�ֳ�����ֹ��

	string Var_name[Proc_Var_Size];//�÷ֳ����µľֲ�����
	int P_vn;

	BaseBlock** Blocks;//�÷ֳ����µĻ�����
	int Blocks_Size;

	bool** Conflict;//��ͻͼ

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

/*ͨ�üĴ�����_General Purpose Registers*/
#define Temp_Reg_Size 12

struct GPR {
	int gpr_base;//��ʼ��ʱ�Ĵ������
	int gpr_number;//ѡ�õ���ʱ�Ĵ�����
	int gpr_pointer;//��ǰ����ָ��
	string *regs_pool;//��ǰ����ļĴ��������Ӧ����ʱ����

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
