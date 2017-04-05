/*ÓïÒå·ÖÎö_Semantic Analysis*/

#include "pass_first.h"
#include "pass_second.h"
#include "yui_tools.h"

Quatriple *QuatriCode[MidCode_Max_Size];
int P_qc;

int TempName;

Quatriple :: Quatriple(string new_op, string new_r, string new_n1, string new_n2, Sym_table *new_table)
{
	this->r = new_r;
	this->op = new_op;
	this->n1 = new_n1;
	this->n2 = new_n2;
	this->table = new_table;
}
string Quatriple :: q_toString()
{
	return this->r + " " + this->op + " " + this->n1 + " " + this->n2;
}

void SemanticAnalysis_init()
{
	memset(QuatriCode, NULL, sizeof(QuatriCode));
	P_qc = 0;
	TempName = 0;
}
string SeA_GetTempName()
{
	int temp = TempName++;
	return "_T" + yui_itos(temp);
}
bool SeA_TypeCheck(Sym_table *caller_table, string type_fp, string ap_id)
{
	Sym_line *temp_line = caller_table->Sym_query(ap_id);
	string type_ap = (temp_line == NULL)? "integer" : temp_line->type;
	
	if (type_fp == "addr_int" || type_fp == "addr_char")
	{
		if (temp_line == NULL)
			return false;
		else if (type_fp == "addr_int" && type_ap != "integer" && type_ap != "addr_int")
			return false;
		else if (type_fp == "addr_char" && type_ap != "char" && type_ap != "addr_char")
			return false;
		else
			return true;
	}
	else
	{
		if (type_ap == "func_int" || type_ap == "const_int" || type_ap == "addr_int")
			type_ap = "integer";
		else if (type_ap == "func_char" || type_ap == "const_char" || type_ap == "addr_int")
			type_ap = "char";

		return (type_ap == type_fp) ? true : false;
	}
}

void SeA_print_toFile()
{
	FILE *fout_mid = fopen("mid_code.txt", "w");
	
	for (int i = 0; i < P_qc; ++i)
	{
		fprintf(fout_mid, "(%d)", i);
		fputs((QuatriCode[i]->q_toString()).c_str(), fout_mid);
		fprintf(fout_mid, "\n");
	}
	fclose(fout_mid);
}
void SeA_print()//For Debug
{
	cout << "(No.) r op n1 n2" << endl;
	for (int i = 0; i < P_qc; ++i)
		cout << "(" << i << ")" << QuatriCode[i]->q_toString() << endl;
}
