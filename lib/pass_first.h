/*��������*/
#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <String>
#include <map>
using namespace std;

/*ȫ�ֱ���_Global Var*/
extern int Counter_Line;//�С�λ�ü�¼
extern int Counter_Lex;
extern bool File_End;//�����ȡ�������

/*���ű�_Symbol Table*/
#define Params_Max_Size 8

struct Sym_line {
	string id;
	string type;
	
	int length;//���鳤��Ӧ>1��������Ϊ1
	int addr;//��ַorֵ
	struct Sym_table *sub_table;

	Sym_line(string new_id, string new_type);
};
struct Sym_table {
	struct Sym_line* params[Params_Max_Size];
	struct Sym_line* lines[32];
	int P_st_param;
	int P_st_line;
	int Mem_need;
	struct Sym_table* father;
	struct Sym_line * belong_line;

	Sym_table(Sym_table* F = NULL, Sym_line *B_line = NULL);
	int Sym_GetAddr();
	Sym_line* Sym_fill(string id, string type, bool isParam = false);
	Sym_line* Sym_query(string tar_id, bool limited = false);
	Sym_line* Sym_query_obj(string tar_id, int *back_level);
	void Sym_print();
};

extern Sym_table *Table0;

/*������_Error Handler*/
void Error_report(int Eno, string Info);

/*�ʷ�����_Lexicon Analysis*/
#define Buf_Line_Size 256
#define Buf_Lex_Size 32

extern string Reg_Lex;//��ǰToken����

void LexiconAnalysis_init();
string LA_getlexicon();

/*�﷨����_Syntax Analysis*/
extern string Current_Lex;//��ǰToken����

void SA_process(Sym_table *Current_table);

/*�������_Semantic Analysis*/
#define MidCode_Max_Size 512

struct Quatriple {
	string op;
	string n1;
	string n2;
	string r;
	Sym_table *table;

	Quatriple(string new_op, string new_r, string new_n1, string new_n2 = "", Sym_table *new_table = NULL);
	string q_toString();
};

extern Quatriple *QuatriCode[MidCode_Max_Size];
extern int P_qc;

string SeA_GetTempName();
bool SeA_TypeCheck(Sym_table *caller_table, string type_fp, string ap_id);
void SemanticAnalysis_init();

void SeA_print();
void SeA_print_toFile();
