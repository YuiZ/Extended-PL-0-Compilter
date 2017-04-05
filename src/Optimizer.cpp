/*�����Ż�_Optimization*/

#include "../lib/pass_first.h"
#include "../lib/pass_second.h"
#include "../lib/yui_tools.h"

int Block_sign[MidCode_Max_Size];//�����컮�ֱ��
SubProgram* SubProg[32];
int P_sp;

BaseBlock::BaseBlock(int new_begin, int new_end)
{
	begin = new_begin;
	end = new_end;

	memset(Def_set, 0, sizeof(Def_set));
	memset(Use_set, 0, sizeof(Use_set));
	memset(In_set, 0, sizeof(In_set));
	memset(Out_set, 0, sizeof(Out_set));
}
void BaseBlock::print_sets(int P_vn)
{
	cout << "\nDEF SET :" << endl;
	for (int i = 0; i < P_vn; ++i)
		printf("%d\t", Def_set[i]);

	cout << "\nUSE SET :" << endl;
	for (int i = 0; i < P_vn; ++i)
		printf("%d\t", Use_set[i]);

	cout << "\nIN SET :" << endl;
	for (int i = 0; i < P_vn; ++i)
		printf("%d\t", In_set[i]);

	cout << "\nOUT SET :" << endl;
	for (int i = 0; i < P_vn; ++i)
		printf("%d\t", Out_set[i]);
	cout << "\n-----------------" << endl;
}

bool BaseBlock::temp_isSaved(string tar, string *temp_regs, int p_tr)
{
	for (int i = p_tr - 1; i >= 0; --i)
		if (temp_regs[i] == tar)
			return true;
	return false;
}
void BaseBlock::temp_fill_table(Sym_table *tar_table, string t_name)
{
	string new_name = "M" + t_name;
	Quatriple *q;

	for (int i = begin; i <= end; ++i)
	{
		q = QuatriCode[i];
		if (q->n1 == t_name)
			q->n1 = new_name;
		if (q->n2 == t_name)
			q->n2 = new_name;
		if (q->r == t_name)
			q->r = new_name;
	}
	tar_table->Sym_fill(new_name, "integer");
}
void BaseBlock::temp_count(Sym_table *owner_table)
{
	string temp_regs[Temp_Reg_Size];
	int p_tr = 0;
	Quatriple *q;

	for (int i = begin; i <= end; ++i)
	{
		q = QuatriCode[i];
		if (q->n1 != "" && q->n1[0] == '_')
		{
			if (!temp_isSaved(q->n1, temp_regs, p_tr))
			{
				if (p_tr < Temp_Reg_Size)
					temp_regs[p_tr++] = q->n1;
				else
					temp_fill_table(owner_table, q->n1);
			}
			if (q->n2 != "" && q->n2[0] == '_')
			{
				if (!temp_isSaved(q->n2, temp_regs, p_tr))
				{
					if (p_tr < Temp_Reg_Size)
						temp_regs[p_tr++] = q->n2;
					else
						temp_fill_table(owner_table, q->n2);
				}

			}
			if (q->r != "" && q->r != q->n1 && q->r != q->n2 && q->r[0] == '_')
			{
				if (!temp_isSaved(q->r, temp_regs, p_tr))
				{
					if (p_tr < Temp_Reg_Size)
						temp_regs[p_tr++] = q->r;
					else
						temp_fill_table(owner_table, q->r);
				}
			}
		}
	}

	owner_table->Sym_GetAddr();
}

SubProgram::SubProgram(Sym_table *sp_table)
{
	this->table = sp_table;
	begin = end = 0;

	P_vn = 0;
	for (int i = 0; i < sp_table->P_st_param; ++i)
	{
		if (sp_table->params[i]->type == "char" || sp_table->params[i]->type == "integer")
			Var_name[P_vn++] = sp_table->params[i]->id;
	}
	for (int i = 0; i < sp_table->P_st_line; ++i)
	{
		if ((sp_table->lines[i]->type == "char" || sp_table->lines[i]->type == "integer") && sp_table->lines[i]->length == 1)
			Var_name[P_vn++] = sp_table->lines[i]->id;
	}
	for (int i = 0; i < 3; ++i)
		Var_glb[i] = "";

	Blocks_Size = 0;
	Blocks = NULL;
	Conflict = NULL;
}
int SubProgram::var_search(string tar)
{
	for (int i = 0; i < P_vn; ++i)
		if (Var_name[i] == tar)
			return i;
	return -1;
}
int SubProgram::block_search(int tar_begin)
{
	for (int i = 0; i < Blocks_Size; ++i)
		if (Blocks[i]->begin == tar_begin)
			return i;
	return -1;
}
void SubProgram::set_DEF_USE()
{
	Quatriple *q;
	BaseBlock *b;
	int loc;

	for (int j = 0; j < Blocks_Size; ++j)
	{
		b = Blocks[j];

		for (int i = b->begin; i <= b->end; ++i)
		{
			q = QuatriCode[i];
			if (q->op == "ADD" || q->op == "SUB" || q->op == "MUL" || q->op == "DIV" 
				|| q->op == "ASSIGN" || q->op == "[]" || q->op == "Sys_Write" || q->op == "Sys_Read")
			{
				if (q->n1 != "" && q->n1[0] != '_' && (q->n1[0] > '9' || q->n1[0] < '0'))
				{
					loc = var_search(q->n1);
					if (loc >= 0 && b->Def_set[loc] == 0)
						b->Use_set[loc] = 1;
				}
				if (q->n2 != "" && q->n2[0] != '_' && (q->n2[0] > '9' || q->n2[0] < '0'))
				{
					loc = var_search(q->n2);
					if (loc >= 0 && b->Def_set[loc] == 0)
						b->Use_set[loc] = 1;
				}

				if (q->r[0] != '_')
				{
					loc = var_search(q->r);
					if (loc >= 0 && b->Use_set[loc] == 0)
						b->Def_set[loc] = 1;
				}
			}
		}
	}
}
void SubProgram::set_IN_OUT()
{
	Quatriple *q;
	BaseBlock *b, *b2;
	int loc;
	bool temp_victor[Proc_Var_Size];
	bool isSame = false;
	
	memset(temp_victor, 0, sizeof(temp_victor));
	
	while (!isSame)
	{
		isSame = true;
		for (int i = Blocks_Size - 1; i >= 0; --i)
		{
			b = Blocks[i];
			q = QuatriCode[b->end];

			if (i + 1 < Blocks_Size)
			{
				b2 = Blocks[i + 1];

				set_or(b->Out_set, b2->In_set, temp_victor, P_vn);
				isSame = set_isEqual(temp_victor, b->Out_set, P_vn);
				set_copy(b->Out_set, temp_victor, P_vn);
			}

			if (q->op == "goto")
			{
				loc = block_search(atoi(q->n1.c_str()));
				b2 = Blocks[loc];

				set_or(b->Out_set, b2->In_set, temp_victor, P_vn);
				isSame = set_isEqual(temp_victor, b->Out_set, P_vn);
				set_copy(b->Out_set, temp_victor, P_vn);
			}

			set_sub(b->Out_set, b->Def_set, b->In_set, P_vn);
			set_or(b->Use_set, b->In_set, b->In_set, P_vn);
		}
	}
}
void SubProgram::build_blocks()
{
	int i, j, p;

	Blocks_Size = 0;
	for (i = begin; i <= end; ++i)
		if (Block_sign[i])
			Blocks_Size++;

	Blocks = (BaseBlock**)malloc(Blocks_Size * sizeof(BaseBlock*));

	i = begin;
	j = begin + 1;
	p = 0;
	while (i <= end)
	{
		while (!Block_sign[j] && j <= end)
			j++;

		Blocks[p++] = new BaseBlock(i, j - 1);

		i = j;
		j++;
	}
}
void SubProgram::print_blocks()
{
	cout << "GlbVar: [" << Var_glb[0] << ", " << Var_glb[1] << ", " << Var_glb[2] << "]" << endl;
	cout << "-----------SubProc------------" << endl;
	cout << begin << " - " << end << endl;

	cout << "Vars:" << endl;
	for (int i = 0; i < P_vn; ++i)
		cout << Var_name[i] << "\t";

	cout << "\n-----" << endl;

	for (int i = 0; i < Blocks_Size; ++i)
	{
		cout << Blocks[i]->begin << " - " << Blocks[i]->end << endl;
		//Blocks[i]->print_sets(P_vn);//��ӡUSE DEF IN OUT���ϼ�����
	}

	cout << "------------------------------" << endl;
}

void SubProgram::tmp_reg_pre_distribute()
{
	for (int i = 0; i < Blocks_Size; ++i)
	{
		Blocks[i]->temp_count(table);
	}
}
void SubProgram::glb_reg_distribute()
{
	int Var_stat[Proc_Var_Size] = { 0 };

	for (int i = 0; i < Blocks_Size; ++i)
	{
		for (int j = 0; j < P_vn; ++j)
		{
			if (Blocks[i]->Out_set[j])
				Var_stat[j]++;
		}
	}

	Var_glb[0] = Var_glb[1] = Var_glb[2] = "";
	P_vg = 0;

	for (int i = Blocks_Size; i >= 1; --i)
	{
		for (int j = 0; j < P_vn; ++j)
		{
			if (Var_stat[j] == i && P_vg < 3)
				Var_glb[P_vg++] = Var_name[j];
		}
		if (P_vg >= 3)
			break;
	}

	
}

void opt_active_var()
{
	for (int i = 0; i < P_sp; ++i)
		SubProg[i]->set_DEF_USE();

	for (int i = 0; i < P_sp; ++i)
	{
		SubProg[i]->set_IN_OUT();
		SubProg[i]->glb_reg_distribute();
	}

	//for (int i = 0; i < P_sp; ++i)
	//	SubProg[i]->print_blocks();
}

void opt_divide2block()//���ֻ�����
{
	Block_sign[0] = 1;
	for (int i = 0; i < P_qc; ++i)
	{
		if (QuatriCode[i]->op == "goto")
		{
			Block_sign[i + 1] = 1;//��ת��ĵ�һ��
			Block_sign[atoi(QuatriCode[i]->n1.c_str())] = 1;//��תĿ��
		}
		else if (QuatriCode[i]->op == "ENTER")//�������
			Block_sign[i] = 1;
		else if (QuatriCode[i]->op == "CALL")//���̵��÷���λ��
		{
			string proc_name = QuatriCode[i]->n2;
			int counter = 0;
			int N_param = QuatriCode[i]->table->Sym_query(QuatriCode[i]->n2)->sub_table->P_st_param;

			int j = 0;
			while (counter < N_param)
			{
				if ((QuatriCode[i + j]->op == "PASS_A" || QuatriCode[i + j]->op == "PASS_V") && QuatriCode[i + j]->r == proc_name)
					counter++;

				j++;
			}
			if (QuatriCode[i+j]->op != "RETURN_TO")//����RETURN_TO���Ǻ������ã��������ó����ڱ��ʽ�м䣬��Ӧ���
				Block_sign[i+j] = 1;
		}
	}

	/*cout << "-----------BLOCK--------------" << endl;
	for (int i = 0; i < P_qc; ++i)
	{
		if (Block_sign[i])
		{
			if (i>0)
				cout << " - " << i - 1 << endl;
			cout << i;
		}
	}
	cout << " - " << P_qc - 1 << endl;
	cout << "------------------------------" << endl;*/
}
void opt_divede2proc()
{
	P_sp = 0;

	int i = 0;
	while (i < P_qc)
	{
		if (QuatriCode[i]->op == "ENTER")
		{
			SubProg[P_sp] = new SubProgram(QuatriCode[i]->table);

			SubProg[P_sp]->begin = i;
			for (int j = i+1; j < P_qc; ++j)
			{
				if (QuatriCode[j]->op == "RETURN")
				{
					SubProg[P_sp]->end = j;
					break;
				}
			}

			SubProg[P_sp]->build_blocks();//��ÿ���ֳ��򣬽�����������¼

			i = SubProg[P_sp]->end + 1;
			P_sp++;
		}
	}
}
void opt_temp_reg_distribute()
{
	for (int i = 0; i < P_sp; ++i)
		SubProg[i]->tmp_reg_pre_distribute();
}

void Opt_process()
{
	opt_divide2block();//���ֻ�����
	opt_divede2proc();//���ֳַ���

	mips_reg = new GPR(8, Temp_Reg_Size);//�趨�Ĵ�����
	opt_temp_reg_distribute();//��ʱ�Ĵ���Ԥ���䴦��

	string option = "Y";
	//cout << "==>> Try to optimize? Enter \"y\" for YES, any other for NO." << endl;
	//cin >> option;
	//cout << "======" << endl;
	if (option == "Y" || option == "y")
	{
		opt_active_var();//��Ծ��������
	}
}
