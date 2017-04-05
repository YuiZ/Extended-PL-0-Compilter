/*Ŀ���������_Object Code Producer*/

#include "../lib/pass_second.h"
#include "../lib/yui_tools.h"

int P_read;

string ObjectCode[ObjCode_Max_Size];//Ŀ�����
int P_obj;

string Data_str[32];//�ַ���
int P_str;

GPR *mips_reg;

void obj_run(Quatriple *q, Sym_table *t);

Sym_line* RT_query_activelog(Sym_table *t, string tar_id, string* base_reg)//���ұ�����������ֵȡ��base_reg��
{
	int back_level = 0;
	Sym_line* l = t->Sym_query_obj(tar_id, &back_level);

	if (back_level == 0)//�����ڱ���
		*base_reg = "$fp";
	else if (!(l->type == "func_int" || l->type == "func_char"))
	{
		ObjectCode[P_obj++] = "lw " + *base_reg + ",8($fp)";
		for (int i = 1; i < back_level; ++i)
			ObjectCode[P_obj++] = "lw " + *base_reg + ",8(" + *base_reg + ")";
	}
	return l;
}
string RT_query_register(Sym_table *t, string default_reg, string tar)//���Ҳ�����
{
	string reg_r;

	if (tar[0] == '_')//������ʱ����������ҼĴ�����
		reg_r = mips_reg->GPR_get_reg(tar);
	else if (tar[0] >= '0' && tar[0] <= '9')//�������֣�ֱ�Ӽ��ص�default_reg��
	{
		reg_r = default_reg;
		ObjectCode[P_obj++] = "li " + default_reg + "," + tar;
	}
	else//���Ǳ���
	{
		reg_r = mips_reg->GPR_search_glb(tar);//�����Ƿ���ȫ�ּĴ�����
		
		if (reg_r == "")
		{
			string base_reg = "$s7";
			Sym_line *l = RT_query_activelog(t, tar, &base_reg);
			reg_r = default_reg;
			if (l->type == "const_int" || l->type == "const_char")
				ObjectCode[P_obj++] = "li " + reg_r + "," + yui_itos(l->addr);
			else if (l->type == "addr_int" || l->type == "addr_char")
			{
				ObjectCode[P_obj++] = "lw " + reg_r + "," + yui_itos(l->addr) + "(" + base_reg + ")";
				ObjectCode[P_obj++] = "lw " + reg_r + ",0(" + reg_r + ")";
			}
			else
				ObjectCode[P_obj++] = "lw " + reg_r + "," + yui_itos(l->addr) + "(" + base_reg + ")";
		}
	}
	return reg_r;
}

//�Ĵ���״̬���漰�ָ����ڹ��̺ͺ�������֮ǰ/��
void RT_regs_save()//���浱ǰ�Ĵ���״̬
{
	int i;
	for (i = 0; i < mips_reg->gpr_number; ++i)
		ObjectCode[P_obj++] = "sw $" + yui_itos(i + mips_reg->gpr_base) + "," + yui_itos(i * 4) + "($gp)";
	
	ObjectCode[P_obj++] = "addi $gp,$gp," + yui_itos(4*mips_reg->gpr_number);
	ObjectCode[P_obj++] = "";
}
void RT_regs_recover()//��ȡ��һ�α���ļĴ���״̬
{
	ObjectCode[P_obj++] = "subi $gp,$gp," + yui_itos(4 * mips_reg->gpr_number);;
	for (int i = 0; i < mips_reg->gpr_number; ++i)
		ObjectCode[P_obj++] = "lw $" + yui_itos(i + mips_reg->gpr_base) + "," + yui_itos(i * 4) + "($gp)";
	
	ObjectCode[P_obj++] = "";
}

void RT_load_glb(Sym_table *t_callee)
{
	Sym_line *l;

	if (mips_reg->glb_pool[0] != "")
	{
		l = t_callee->Sym_query(mips_reg->glb_pool[0]);
		ObjectCode[P_obj++] = "lw $s4," + yui_itos(l->addr) + "($fp)";
	}
	if (mips_reg->glb_pool[1] != "")
	{
		l = t_callee->Sym_query(mips_reg->glb_pool[1]);
		ObjectCode[P_obj++] = "lw $s5," + yui_itos(l->addr) + "($fp)";

	}
	if (mips_reg->glb_pool[2] != "")
	{
		l = t_callee->Sym_query(mips_reg->glb_pool[2]);
		ObjectCode[P_obj++] = "lw $s6," + yui_itos(l->addr) + "($fp)";
	}
}
void RT_save_glb(Sym_table *t_caller)
{
	Sym_line *l;

	if (mips_reg->glb_pool[0] != "")
	{
		l = t_caller->Sym_query(mips_reg->glb_pool[0]);
		ObjectCode[P_obj++] = "sw $s4," + yui_itos(l->addr) + "($fp)";
	}
	if (mips_reg->glb_pool[1] != "")
	{
		l = t_caller->Sym_query(mips_reg->glb_pool[1]);
		ObjectCode[P_obj++] = "sw $s5," + yui_itos(l->addr) + "($fp)";
	}
	if (mips_reg->glb_pool[2] != "")
	{
		l = t_caller->Sym_query(mips_reg->glb_pool[2]);
		ObjectCode[P_obj++] = "sw $s6," + yui_itos(l->addr) + "($fp)";
	}
}
void RT_refresh_glb(Sym_table *t_callee)
{
	int loc = -1;
	for (int i = 0; i < P_sp; ++i)
	{
		if (SubProg[i]->table->belong_line == t_callee->belong_line)
		{
			loc = i;
			break;
		}
	}

	if (loc != -1)
	{
		SubProgram *sp = SubProg[loc];
		mips_reg->GPR_addto_glb(sp->Var_glb[0], sp->Var_glb[1], sp->Var_glb[2]);
	}
	else
		Error_report(0, "Error in glb_reg => Find me in RT_refresh_glb();");
}

Quatriple* obj_nextQ()//����һ���м����
{
	if (P_read < P_qc)
	{
		if (Block_sign[P_read])//���˾��ǻ��������
		{
			mips_reg->GPR_refresh();//��ʱ�Ĵ�������ָ����0�������������ʱ�Ĵ��������·��䣩
			ObjectCode[P_obj++] = "_b" + yui_itos(P_read) + ":";//��ӻ������ǣ�������ת���
		}
		return  QuatriCode[P_read++];
	}
	else
		return NULL;
}

void obj_caculate_AS(Quatriple *q, Sym_table *t, string op)//���㣬+��-���������
{
	string reg_n1, reg_n2, reg_r;
	
	//�ֱ�ȡ����������
	reg_n1 = (q->n1 == "")?"$0": RT_query_register(t, "$t8", q->n1);//��q->n1Ϊ�գ�����'-'��Ϊ��ʼ�ı��ʽ
	reg_n2 = RT_query_register(t, "$t9", q->n2);

	//ȷ�������������λ��
	if (q->r[0] == '_')//��ʱ����_Tx
	{
		reg_r = mips_reg->GPR_get_reg(q->r);//�ӼĴ����Ѳ�ѯ����δ������������µ���ʱ�Ĵ�����
		ObjectCode[P_obj++] = op + " " + reg_r + "," + reg_n1 + "," + reg_n2;
	}
	else//һ�����
	{
		reg_r = mips_reg->GPR_search_glb(q->r);//����ȫ�ּĴ�����
		if (reg_r == "")
		{
			string base_reg = "$s7";//Ĭ�Ͻ���Ѱ�������ڵĻ���ַ����$s7
			Sym_line *l = RT_query_activelog(t, q->r, &base_reg);//��ѯ��������λ��
			ObjectCode[P_obj++] = op + " $t8," + reg_n1 + "," + reg_n2;//����
			if (l->type == "addr_int" || l->type == "addr_char")//��������һ����ַ������Ѱַ����
			{
				ObjectCode[P_obj++] = "lw $t9," + yui_itos(l->addr) + "(" + base_reg + ")";
				ObjectCode[P_obj++] = "sw $t8,0($t9)";
			}
			else
				ObjectCode[P_obj++] = "sw $t8," + yui_itos(l->addr) + "(" + base_reg + ")";
		}
		else
			ObjectCode[P_obj++] = op + " " + reg_r + "," + reg_n1 + "," + reg_n2;
	}
}
void obj_caculate_MD(Quatriple *q, Sym_table *t, string op)//���㣬*��/���������
{
	string reg_n1, reg_n2, reg_r;

	reg_n1 = RT_query_register(t, "$t8", q->n1);
	reg_n2 = (q->n2 == "") ? "$0" : RT_query_register(t, "$t9", q->n2);

	if (q->r[0] == '_')
	{
		reg_r = mips_reg->GPR_get_reg(q->r);

		ObjectCode[P_obj++] = op + " " + reg_n1 + "," + reg_n2;
		ObjectCode[P_obj++] = "mflo " + reg_r;
	}
	else
	{
		reg_r = mips_reg->GPR_search_glb(q->r);//����ȫ�ּĴ�����
		if (reg_r == "")
		{
			string base_reg = "$s7";
			Sym_line *l = RT_query_activelog(t, q->r, &base_reg);
			ObjectCode[P_obj++] = op + " " + reg_n1 + "," + reg_n2;
			ObjectCode[P_obj++] = "mflo $t8";
			if (l->type == "addr_int" || l->type == "addr_char")
			{
				ObjectCode[P_obj++] = "lw $t9," + yui_itos(l->addr) + "(" + base_reg + ")";
				ObjectCode[P_obj++] = "sw $t8,0($t9)";
			}
			else
				ObjectCode[P_obj++] = "sw $t8," + yui_itos(l->addr) + "(" + base_reg + ")";
		}
		else
		{
			ObjectCode[P_obj++] = op + " " + reg_n1 + "," + reg_n2;
			ObjectCode[P_obj++] = "mflo " + reg_r;
		}
	}
}
void obj_array_load(Quatriple *q, Sym_table *t)//ȡ����Ԫ��ֵ(����Ϊ���ʽ����ʱ)
{
	string base_reg = "$s7";
	Sym_line *l = RT_query_activelog(t, q->n1, &base_reg);//��ѯ��������λ��
	
	if (base_reg == "$fp")//��Ϊ֮����ʹ���������ڲ����ַ���м��㣬�ʵ������ڱ���ʱ��Ҳ�轫��ַȡ��$s7
	{
		ObjectCode[P_obj++] = "ori $s7,$fp,0";
		base_reg = "$s7";
	}
	string reg_offset = RT_query_register(t, "$t8", q->n2);//ȡƫ����
	ObjectCode[P_obj++] = "sll " + reg_offset + "," + reg_offset + ",2";//ʵ���ڴ��ַ = ƫ�� * 4��������2λ��
	ObjectCode[P_obj++] = "add " + base_reg + "," + base_reg + "," + reg_offset;//��ַ+ƫ�� => Ԫ��λ��

	string reg_r = mips_reg->GPR_get_reg(q->r);
	ObjectCode[P_obj++] = "lw " + reg_r + "," + yui_itos(l->addr) + "(" + base_reg + ")";//ȡֵ
}
void obj_assign(Quatriple *q, Sym_table *t)//��ֵ���
{
	string reg_n1, reg_r;

	//���������ֵ֮��p->n1
	reg_n1 = RT_query_register(t, "$t8", q->n1);
	if (reg_n1 != "$t8")//����ֵ����$t8
		ObjectCode[P_obj++] = "ori $t8," + reg_n1 + ",0";

	//������ֵ����
	reg_r = mips_reg->GPR_search_glb(q->r);
	if (reg_r != "")//��������ȫ�ּĴ�����
	{
		ObjectCode[P_obj++] = "ori " + reg_r + ",$t8,0";
	}
	else
	{
		string base_reg = "$s7";
		Sym_line *l = RT_query_activelog(t, q->r, &base_reg);
		if (base_reg == "$fp" && l->length > 1)//����ƫ�Ʋ�Ӧ�ı�$fp
		{
			ObjectCode[P_obj++] = "ori $s7,$fp,0";
			base_reg = "$s7";
		}

		//��ֵ����
		if (l->type == "func_int" || l->type == "func_char")//���˸�ֵ����Ǻ�������ֵ
		{
			ObjectCode[P_obj++] = "ori $v1,$t8,0";//������ֵ���Ĵ���$v1
		}
		else if (l->type == "addr_int" || l->type == "addr_char")
		{
			ObjectCode[P_obj++] = "lw $t9," + yui_itos(l->addr) + "(" + base_reg + ")";
			ObjectCode[P_obj++] = "sw $t8,0($t9)";
		}
		else
		{
			if (q->n2 != "")//��ֵn2�ǿ���Ϊoffset,����ֵ����������Ԫ��
			{
				string reg_n2 = RT_query_register(t, "$t9", q->n2);

				ObjectCode[P_obj++] = "sll " + reg_n2 + "," + reg_n2 + ",2";
				ObjectCode[P_obj++] = "add " + base_reg + "," + base_reg + "," + reg_n2;
			}
			ObjectCode[P_obj++] = "sw $t8," + yui_itos(l->addr) + "(" + base_reg + ")";
		}
	}
}

void obj_jump(Quatriple *q)
{
	ObjectCode[P_obj++] = "j _b" + q->n1;
}
void obj_condit_jump(Quatriple *q, Sym_table *t)//����ת�ƣ�������
{
	string condit_parts[3];
	condit_parts[0] = condit_parts[1] = condit_parts[2] = "";//0-<����ʽ> 1-<�Ƚ������> 2-<�ұ��ʽ>

	yui_split_condit(q->n2, condit_parts);//����ԪʽЯ�����������

	string n1_reg = RT_query_register(t, "$t8", condit_parts[0]);
	string n2_reg = RT_query_register(t, "$t9", condit_parts[2]);

	bool anti = (q->r == "if not") ? true : false;//�Ƿ�Ϊ��������ʱ��ת��

	if (condit_parts[1] == "==")
		ObjectCode[P_obj++] = ((anti)? "bne ":"beq ") + n1_reg + "," + n2_reg + ",_b" + q->n1;
	else if (condit_parts[1] == ">")
	{
		ObjectCode[P_obj++] = "sub $t9," + n1_reg + "," + n2_reg;
		ObjectCode[P_obj++] = ((anti)? "blez $t9,_b" : "bgtz $t9,_b") + q->n1;
	}
	else if (condit_parts[1] == ">=")
	{
		ObjectCode[P_obj++] = "sub $t9," + n1_reg + "," + n2_reg;
		ObjectCode[P_obj++] = ((anti) ? "bltz $t9,_b" : "bgez $t9,_b") + q->n1;
	}
	else if (condit_parts[1] == "<")
	{
		ObjectCode[P_obj++] = "sub $t9," + n1_reg + "," + n2_reg;
		ObjectCode[P_obj++] = ((anti) ? "bgez $t9,_b" : "bltz $t9,_b") + q->n1;
	}
	else if (condit_parts[1] == "<=")
	{
		ObjectCode[P_obj++] = "sub $t9," + n1_reg + "," + n2_reg;
		ObjectCode[P_obj++] = ((anti) ? "bgtz $t9,_b" : "blez $t9,_b") + q->n1;
	}
	else if (condit_parts[1] == "<>")
		ObjectCode[P_obj++] = ((anti) ? "beq " : "bne ") + n1_reg + "," + n2_reg + ",_b" + q->n1;
}

void obj_read(Quatriple *q, Sym_table *t)//ϵͳ����-����
{
	string base_reg = "$t9";
	Sym_line *l = RT_query_activelog(t, q->r, &base_reg);
		
	if (l->type == "char")
	{
		ObjectCode[P_obj++] = "li $v0,12";
		ObjectCode[P_obj++] = "syscall";

		string reg_r = mips_reg->GPR_search_glb(q->r);//����Ŷ�����ȫ�ּĴ�����
		if (reg_r != "")
			ObjectCode[P_obj++] = "ori " + reg_r + ",$v0,0";
		else
			ObjectCode[P_obj++] = "sw $v0," + yui_itos(l->addr) + "(" + base_reg + ")";
	}
	else if (l->type == "integer")
	{
		ObjectCode[P_obj++] = "li $v0,5";
		ObjectCode[P_obj++] = "syscall";

		string reg_r = mips_reg->GPR_search_glb(q->r);
		if (reg_r != "")
			ObjectCode[P_obj++] = "ori " + reg_r + ",$v0,0";
		else
			ObjectCode[P_obj++] = "sw $v0," + yui_itos(l->addr) + "(" + base_reg + ")";
	}
	else if (l->type == "addr_int")
	{
		ObjectCode[P_obj++] = "li $v0,5";
		ObjectCode[P_obj++] = "syscall";
		ObjectCode[P_obj++] = "lw $t9," + yui_itos(l->addr) + "(" + base_reg + ")";
		ObjectCode[P_obj++] = "sw $v0,0($t9)";
	}
	else if (l->type == "addr_char")
	{
		ObjectCode[P_obj++] = "li $v0,12";
		ObjectCode[P_obj++] = "syscall";
		ObjectCode[P_obj++] = "lw $t9," + yui_itos(l->addr) + "(" + base_reg + ")";
		ObjectCode[P_obj++] = "sw $v0,0($t9)";
	}
	else
		Error_report(0, "Unknown read type => Find me in obj_read();");
}
void obj_write(Quatriple *q, Sym_table *t)//ϵͳ����-д
{
	if (q->n1[0] == '"')//���������ַ���
	{
		Data_str[P_str++] = "str" + yui_itos(P_str - 1) + ": .asciiz " + q->n1;
		ObjectCode[P_obj++] = "la $a0,str" + yui_itos(P_str - 1);
		ObjectCode[P_obj++] = "li $v0,4";
	}
	else//���Ǳ��ʽ���
	{
		string temp_reg = RT_query_register(t, "$a0", q->n1);
		if (temp_reg != "$a0")
		{
			ObjectCode[P_obj++] = "ori $a0," + temp_reg + ",0";
			ObjectCode[P_obj++] = "li $v0,1";
		}
		else
		{
			Sym_line *l = t->Sym_query(q->n1);
			if (l != NULL && (l->type == "const_char" || l->type == "char"))
				ObjectCode[P_obj++] = "li $v0,11";
			else
				ObjectCode[P_obj++] = "li $v0,1";
		}
	}
	ObjectCode[P_obj++] = "syscall";
}

void obj_pass(Sym_table *t_callee, Sym_table *t_caller)//��������
{
	int params_counter = 0;//��¼�Ѵ���������
	string base_reg = "$s7";
	Sym_line *temp_line = NULL;
	Quatriple *q_read;
	
	while (params_counter < t_callee->P_st_param)//����ȫ��������֮ǰ�����ܴ��ڼ���������������
	{
		q_read = obj_nextQ();

		if (q_read->op == "PASS_A")//����ַ
		{
			temp_line = RT_query_activelog(t_caller, q_read->n1, &base_reg);//������Դ���ϲ㣬�ʲ�ѯ��t_caller��������ȫ�ּĴ���=>�ս������ڴ�

			if (q_read->n2 != "")//����Ԫ��
			{
				if (base_reg == "$fp")//����ƫ�Ʋ�Ӧ�ı�$fp
				{
					ObjectCode[P_obj++] = "ori $s7,$fp,0";
					base_reg = "$s7";
				}

				string reg_offset = RT_query_register(t_caller, "$t8", q_read->n2);
				ObjectCode[P_obj++] = "sll " + reg_offset + "," + reg_offset + ",2";
				ObjectCode[P_obj++] = "add " + base_reg + "," + base_reg + "," + reg_offset;

				ObjectCode[P_obj++] = "ori $a2," + base_reg + ",0";
			}
			
			if (temp_line->type == "addr_int" || temp_line->type == "addr_char")//�������ǵ�ַ��������ֵ��
				ObjectCode[P_obj++] = "lw $a2," + yui_itos(temp_line->addr) + "(" + base_reg + ")";
			else
				ObjectCode[P_obj++] = "addi $a2," + base_reg + "," + yui_itos(temp_line->addr);
			
			ObjectCode[P_obj++] = "sw $a2," + yui_itos(t_callee->params[params_counter]->addr) + "($sp)";
			params_counter++;
		}
		else if (q_read->op == "PASS_V")//����ֵ
		{
			string temp_reg;
			temp_reg = RT_query_register(t_caller, "$a2", q_read->n1);
			if (temp_reg != "$a2")
				ObjectCode[P_obj++] = "ori $a2," + temp_reg + ",0";

			ObjectCode[P_obj++] = "sw $a2," + yui_itos(t_callee->params[params_counter]->addr) + "($sp)";
			params_counter++;
		}
		else
			obj_run(q_read, t_caller);
	}
}
void obj_call(Quatriple *q)
{
	Sym_table *t_caller = q->table;
	Sym_line *l_callee = t_caller->Sym_query(q->n2);
	Sym_table *t_callee = l_callee->sub_table;
	
	obj_pass(t_callee, t_caller);//���ݲ���

	ObjectCode[P_obj++] = "ori $a0,$fp,0";//�����ϲ��ַ ��̬��

	//���ɾ�̬��
	if (t_caller == Table0 || t_callee->Sym_query(t_caller->belong_line->id) != NULL)//�������ܲ鵽�����ߣ���Ϊ�ݹ���ӹ��̵���
	{
		if (t_callee->father == t_caller->father)//����һ�εݹ��Caller��Callee���ڷ��ű�ͬ��
			ObjectCode[P_obj++] = "lw $a1,8($fp)";//��̬��ͬCaller
		else//���ǵ����ӹ���
			ObjectCode[P_obj++] = "ori $a1,$fp,0";//��̬��ָCaller
	}
	else//���ǵ��÷��ű�֮ǰ��Ĺ��̣��������ݲ�β����ϲ��Ҿ�̬��
	{
		ObjectCode[P_obj++] = "ori $a1,$fp,0";

		Sym_table *t = t_caller;
		string ID = l_callee->id;
		while (t != NULL && t->Sym_query(ID) != NULL)
		{
			ObjectCode[P_obj++] = "lw $a1,8($a1)";
			t = t->father;
		}
		P_obj--;//������ѭ����Ȼ��һ��
	}
	
	RT_regs_save();//��ʱ�Ĵ���״̬����
	RT_save_glb(t_caller);//ȫ�ּĴ���״̬����

	//��ת������/������
	ObjectCode[P_obj++] = "jal " + q->n1;

	//ִ�����
	ObjectCode[P_obj++] = "lw,$fp,4($fp)";//��ǰ��ַ�л�
	ObjectCode[P_obj++] = "subi $sp,$sp," + yui_itos(t_callee->Mem_need);//��ջ
	ObjectCode[P_obj++] = "lw $31,12($fp)";//����$31����λ��--jr�Զ����
	
	RT_regs_recover();//��ʱ�Ĵ���״̬�ָ�
	RT_refresh_glb(t_caller);//ȫ�ּĴ���״̬�ָ�
	RT_load_glb(t_caller);
}
void obj_return_to(Quatriple *q)//��������ֵ���
{
	string tar_reg = mips_reg->GPR_get_reg(q->n1);//��ȡ�µ���ʱ�Ĵ�����Ŵ˴κ������÷���ֵ
	ObjectCode[P_obj++] = "ori " + tar_reg + ",$v1,0";
}

void obj_run(Quatriple *q, Sym_table *t)//�����ദ��
{
	if (q->op == "ADD")
		obj_caculate_AS(q, t, "add");
	else if (q->op == "SUB")
		obj_caculate_AS(q, t, "sub");
	else if (q->op == "MUL")
		obj_caculate_MD(q, t, "mult");
	else if (q->op == "DIV")
		obj_caculate_MD(q, t, "div");
	else if (q->op == "[]")
		obj_array_load(q, t);
	else if (q->op == "goto" && q->r == "then")
		obj_jump(q);
	else if (q->op == "goto")
		obj_condit_jump(q, t);
	else if (q->op == "ASSIGN")
		obj_assign(q, t);
	else if (q->op == "Sys_Read")
		obj_read(q, t);
	else if (q->op == "Sys_Write")
		obj_write(q, t);
	else if (q->op == "CALL")
		obj_call(q);
	else if (q->op == "RETURN_TO")
		obj_return_to(q);
	else
		Error_report(0, "Unknown quatriple. => Find me in obj_run();");
}
void obj_process()
{
	Quatriple *q = obj_nextQ();
	
	while (P_read < P_qc)
	{
		Sym_table *t = q->table;

		ObjectCode[P_obj++] = q->n1 + ":";//������ڱ��

		ObjectCode[P_obj++] = "ori $fp,$sp,0";//$fp�浱ǰ��ַ
		ObjectCode[P_obj++] = "addi $sp,$sp," + yui_itos(t->Mem_need);//ջ������
		ObjectCode[P_obj++] = "sw $fp,0($fp)";//AL-��ַ
		ObjectCode[P_obj++] = "sw $a0,4($fp)";//AL-d_link ��̬��
		ObjectCode[P_obj++] = "sw $a1,8($fp)";//AL-s_link ��̬��
		ObjectCode[P_obj++] = "sw $ra,12($fp)";//AL-����λ�� jal�$31
		ObjectCode[P_obj++] = "";

		RT_refresh_glb(t);//ȫ�ּĴ���ˢ��
		RT_load_glb(t);

		q = obj_nextQ();
		while (q->op != "RETURN")//��RETURN֮ǰ���������Ԫʽ
		{
			obj_run(q, t);
			q = obj_nextQ();
		}

		if (t->father != NULL)//NULL��������
			ObjectCode[P_obj++] = "jr $31";//����
		ObjectCode[P_obj++] = "";

		q = obj_nextQ();
	}
}
void obj_data_str()//.data�ֶ����
{
	for (int i = 0; i < P_str; ++i)
	{
		ObjectCode[P_obj++] = ".data";
		ObjectCode[P_obj++] = Data_str[i];
	}
}

void Obj_generate()
{
	ObjectCode[P_obj++] = "li $sp,0x10040000";//Mars�趨
	ObjectCode[P_obj++] = "li $gp,0x10080000";
	ObjectCode[P_obj++] = "li $a0,0";
	ObjectCode[P_obj++] = "li $a1,0";
	ObjectCode[P_obj++] = "li $ra,0";
	ObjectCode[P_obj++] = "j main_main";
	ObjectCode[P_obj++] = "";
	
	RT_refresh_glb(Table0);
	obj_process();
	obj_data_str();
}

void obj_print()//For Debug
{
	for (int i = 0; i < P_obj; ++i)
		cout << ObjectCode[i] << endl;
}
void obj_print_toFile()
{
	FILE *fout_final = fopen("obj_code.txt","w");

	for (int i = 0; i < P_obj; ++i)
	{
		fputs(ObjectCode[i].c_str(), fout_final);
		fprintf(fout_final, "\n");
	}
	fclose(fout_final);
}
