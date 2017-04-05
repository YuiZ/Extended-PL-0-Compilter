/*·ûºÅ±í_Symbol Table*/

#include "pass_first.h"
#include "pass_second.h"
#include "yui_tools.h"

Sym_table *Table0;

Sym_line::Sym_line(string new_id, string new_type)
{
	this->id = new_id;
	this->type = new_type;
	
	this->length = 1;
	this->addr = -1;
	this->sub_table = NULL;
}

Sym_table::Sym_table(Sym_table* F, Sym_line *B_line)
{
	memset(params, NULL, sizeof(params));
	memset(lines, NULL, sizeof(lines));
	P_st_line = P_st_param = 0;
	Mem_need = 0;
	father = F;
	belong_line = B_line;
}
int Sym_table::Sym_GetAddr()
{
	int offset = 16;//base\s-link\d-link\return_loc 4 * 4 = 20
	Sym_line *p = NULL;

	for (int i = 0; i < P_st_param; ++i)
	{
		p = params[i];
		p->addr = offset;
		offset += p->length * 4;
	}
	for (int i = 0; i < P_st_line; ++i)
	{
		p = lines[i];
		if (p->type != "const_int" && p->type != "const_char" && p->type != "proc")
		{
			p->addr = offset;
			offset += p->length * 4;
		}
	}
	Mem_need = offset;
	return offset;
}
Sym_line* Sym_table::Sym_fill(string id, string type, bool isParam)//Ìî±í
{
	Sym_line* temp_line;
	if (isParam)
		temp_line = params[P_st_param++] = new Sym_line(id, type);
	else
		temp_line = lines[P_st_line++] = new Sym_line(id, type);

	return temp_line;
}
Sym_line* Sym_table::Sym_query(string tar_id, bool limited)
{
	if (tar_id != "")
	{
		for (int i = P_st_param - 1; i >= 0; --i)
		{
			if (params[i]->id == tar_id)
				return params[i];
		}
		for (int i = P_st_line - 1; i >= 0; --i)
		{
			if (lines[i]->id == tar_id)
				return lines[i];
		}
		if (!limited && this->father != NULL)
			return this->father->Sym_query(tar_id);
	}
	return NULL;
}
Sym_line* Sym_table::Sym_query_obj(string tar_id, int *back_level)
{
	for (int i = P_st_param - 1; i >= 0; --i)
		if (params[i]->id == tar_id)
			return params[i];
	for (int i = P_st_line - 1; i >= 0; --i)
		if (lines[i]->id == tar_id)
			return lines[i];
		
	if (this->father != NULL)
	{
		(*back_level) = (*back_level) + 1;
		return this->father->Sym_query_obj(tar_id, back_level);
	}
		
	return NULL;
}

void Sym_table::Sym_print()
{
	cout << "|| " + this->belong_line->id + " ||" << endl;
	cout << "memory: " << this->Mem_need << " bytes" << endl;
	cout << "INDEX\t" << "ID\t\t" << "TYPE\t\t" << "LEN\t\t" << "ADDR" << endl;
	cout << "-----------------------" << endl;
	for (int i = 0; i < P_st_param; ++i)
	{
		cout << i << "\t";
		cout << params[i]->id << "\t\t";
		cout << params[i]->type << "\t\t";
		cout << params[i]->length << "\t\t";
		cout << params[i]->addr << endl;
	}
	cout << "---" << endl;
	for (int i = 0; i < P_st_line; ++i)
	{
		cout << i << "\t";
		cout << lines[i]->id << "\t\t";
		cout << lines[i]->type << "\t\t";
		cout << lines[i]->length << "\t\t";
		cout << lines[i]->addr << endl;
		
		if (lines[i]->sub_table != NULL)
			lines[i]->sub_table->Sym_print();
	}
	cout << "-----^-----------^-----" << endl;
}
