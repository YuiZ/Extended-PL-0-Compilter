/*目标代码生成_Object Code Producer*/

#include "pass_second.h"
#include "yui_tools.h"

int P_read;

string ObjectCode[ObjCode_Max_Size];//目标代码
int P_obj;

string Data_str[32];//字符串
int P_str;

GPR *mips_reg;

void obj_run(Quatriple *q, Sym_table *t);

Sym_line* RT_query_activelog(Sym_table *t, string tar_id, string* base_reg)//查找变量，并将其值取到base_reg中
{
	int back_level = 0;
	Sym_line* l = t->Sym_query_obj(tar_id, &back_level);

	if (back_level == 0)//若就在本层
		*base_reg = "$fp";
	else if (!(l->type == "func_int" || l->type == "func_char"))
	{
		ObjectCode[P_obj++] = "lw " + *base_reg + ",8($fp)";
		for (int i = 1; i < back_level; ++i)
			ObjectCode[P_obj++] = "lw " + *base_reg + ",8(" + *base_reg + ")";
	}
	return l;
}
string RT_query_register(Sym_table *t, string default_reg, string tar)//查找操作数
{
	string reg_r;

	if (tar[0] == '_')//若是临时变量，则查找寄存器堆
		reg_r = mips_reg->GPR_get_reg(tar);
	else if (tar[0] >= '0' && tar[0] <= '9')//若是数字，直接加载到default_reg中
	{
		reg_r = default_reg;
		ObjectCode[P_obj++] = "li " + default_reg + "," + tar;
	}
	else//若是变量
	{
		reg_r = mips_reg->GPR_search_glb(tar);//先找是否在全局寄存器中
		
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

//寄存器状态保存及恢复；在过程和函数调用之前/后
void RT_regs_save()//保存当前寄存器状态
{
	int i;
	for (i = 0; i < mips_reg->gpr_number; ++i)
		ObjectCode[P_obj++] = "sw $" + yui_itos(i + mips_reg->gpr_base) + "," + yui_itos(i * 4) + "($gp)";
	
	ObjectCode[P_obj++] = "addi $gp,$gp," + yui_itos(4*mips_reg->gpr_number);
	ObjectCode[P_obj++] = "";
}
void RT_regs_recover()//读取上一次保存的寄存器状态
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

Quatriple* obj_nextQ()//读下一条中间代码
{
	if (P_read < P_qc)
	{
		if (Block_sign[P_read])//若此句是基本块入口
		{
			mips_reg->GPR_refresh();//临时寄存器分配指针置0（过基本块后临时寄存器可重新分配）
			ObjectCode[P_obj++] = "_b" + yui_itos(P_read) + ":";//添加基本块标记，用于跳转语句
		}
		return  QuatriCode[P_read++];
	}
	else
		return NULL;
}

void obj_caculate_AS(Quatriple *q, Sym_table *t, string op)//计算，+与-运算符处理
{
	string reg_n1, reg_n2, reg_r;
	
	//分别取两个操作数
	reg_n1 = (q->n1 == "")?"$0": RT_query_register(t, "$t8", q->n1);//若q->n1为空，则是'-'作为起始的表达式
	reg_n2 = RT_query_register(t, "$t9", q->n2);

	//确定计算结果存入的位置
	if (q->r[0] == '_')//临时变量_Tx
	{
		reg_r = mips_reg->GPR_get_reg(q->r);//从寄存器堆查询（如未记入则分配以新的临时寄存器）
		ObjectCode[P_obj++] = op + " " + reg_r + "," + reg_n1 + "," + reg_n2;
	}
	else//一般变量
	{
		reg_r = mips_reg->GPR_search_glb(q->r);//若在全局寄存器中
		if (reg_r == "")
		{
			string base_reg = "$s7";//默认将待寻变量所在的基地址存入$s7
			Sym_line *l = RT_query_activelog(t, q->r, &base_reg);//查询变量所在位置
			ObjectCode[P_obj++] = op + " $t8," + reg_n1 + "," + reg_n2;//计算
			if (l->type == "addr_int" || l->type == "addr_char")//若变量是一个地址，则再寻址存入
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
void obj_caculate_MD(Quatriple *q, Sym_table *t, string op)//计算，*与/运算符处理
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
		reg_r = mips_reg->GPR_search_glb(q->r);//若在全局寄存器中
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
void obj_array_load(Quatriple *q, Sym_table *t)//取数组元素值(仅作为表达式因子时)
{
	string base_reg = "$s7";
	Sym_line *l = RT_query_activelog(t, q->n1, &base_reg);//查询数组所在位置
	
	if (base_reg == "$fp")//因为之后需使用数组所在层基地址进行计算，故当数组在本层时，也需将地址取到$s7
	{
		ObjectCode[P_obj++] = "ori $s7,$fp,0";
		base_reg = "$s7";
	}
	string reg_offset = RT_query_register(t, "$t8", q->n2);//取偏移量
	ObjectCode[P_obj++] = "sll " + reg_offset + "," + reg_offset + ",2";//实际内存地址 = 偏移 * 4（即左移2位）
	ObjectCode[P_obj++] = "add " + base_reg + "," + base_reg + "," + reg_offset;//基址+偏移 => 元素位置

	string reg_r = mips_reg->GPR_get_reg(q->r);
	ObjectCode[P_obj++] = "lw " + reg_r + "," + yui_itos(l->addr) + "(" + base_reg + ")";//取值
}
void obj_assign(Quatriple *q, Sym_table *t)//赋值语句
{
	string reg_n1, reg_r;

	//处理待赋予之值，p->n1
	reg_n1 = RT_query_register(t, "$t8", q->n1);
	if (reg_n1 != "$t8")//待赋值置与$t8
		ObjectCode[P_obj++] = "ori $t8," + reg_n1 + ",0";

	//处理被赋值对象
	reg_r = mips_reg->GPR_search_glb(q->r);
	if (reg_r != "")//若对象在全局寄存器中
	{
		ObjectCode[P_obj++] = "ori " + reg_r + ",$t8,0";
	}
	else
	{
		string base_reg = "$s7";
		Sym_line *l = RT_query_activelog(t, q->r, &base_reg);
		if (base_reg == "$fp" && l->length > 1)//数组偏移不应改变$fp
		{
			ObjectCode[P_obj++] = "ori $s7,$fp,0";
			base_reg = "$s7";
		}

		//赋值过程
		if (l->type == "func_int" || l->type == "func_char")//若此赋值语句是函数返回值
		{
			ObjectCode[P_obj++] = "ori $v1,$t8,0";//将返回值给寄存器$v1
		}
		else if (l->type == "addr_int" || l->type == "addr_char")
		{
			ObjectCode[P_obj++] = "lw $t9," + yui_itos(l->addr) + "(" + base_reg + ")";
			ObjectCode[P_obj++] = "sw $t8,0($t9)";
		}
		else
		{
			if (q->n2 != "")//赋值n2非空则为offset,即赋值对象是数组元素
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
void obj_condit_jump(Quatriple *q, Sym_table *t)//条件转移，控制流
{
	string condit_parts[3];
	condit_parts[0] = condit_parts[1] = condit_parts[2] = "";//0-<左表达式> 1-<比较运算符> 2-<右表达式>

	yui_split_condit(q->n2, condit_parts);//将四元式携带的条件拆分

	string n1_reg = RT_query_register(t, "$t8", condit_parts[0]);
	string n2_reg = RT_query_register(t, "$t9", condit_parts[2]);

	bool anti = (q->r == "if not") ? true : false;//是否为“不成立时跳转”

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

void obj_read(Quatriple *q, Sym_table *t)//系统调用-读入
{
	string base_reg = "$t9";
	Sym_line *l = RT_query_activelog(t, q->r, &base_reg);
		
	if (l->type == "char")
	{
		ObjectCode[P_obj++] = "li $v0,12";
		ObjectCode[P_obj++] = "syscall";

		string reg_r = mips_reg->GPR_search_glb(q->r);//若存放对象在全局寄存器中
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
void obj_write(Quatriple *q, Sym_table *t)//系统调用-写
{
	if (q->n1[0] == '"')//若参数是字符串
	{
		Data_str[P_str++] = "str" + yui_itos(P_str - 1) + ": .asciiz " + q->n1;
		ObjectCode[P_obj++] = "la $a0,str" + yui_itos(P_str - 1);
		ObjectCode[P_obj++] = "li $v0,4";
	}
	else//若是表达式结果
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

void obj_pass(Sym_table *t_callee, Sym_table *t_caller)//参数传递
{
	int params_counter = 0;//记录已传参数个数
	string base_reg = "$s7";
	Sym_line *temp_line = NULL;
	Quatriple *q_read;
	
	while (params_counter < t_callee->P_st_param)//参数全部传递完之前，可能存在计算参数的语句序列
	{
		q_read = obj_nextQ();

		if (q_read->op == "PASS_A")//传地址
		{
			temp_line = RT_query_activelog(t_caller, q_read->n1, &base_reg);//参数来源于上层，故查询用t_caller；不考虑全局寄存器=>终将存入内存

			if (q_read->n2 != "")//数组元素
			{
				if (base_reg == "$fp")//数组偏移不应改变$fp
				{
					ObjectCode[P_obj++] = "ori $s7,$fp,0";
					base_reg = "$s7";
				}

				string reg_offset = RT_query_register(t_caller, "$t8", q_read->n2);
				ObjectCode[P_obj++] = "sll " + reg_offset + "," + reg_offset + ",2";
				ObjectCode[P_obj++] = "add " + base_reg + "," + base_reg + "," + reg_offset;

				ObjectCode[P_obj++] = "ori $a2," + base_reg + ",0";
			}
			
			if (temp_line->type == "addr_int" || temp_line->type == "addr_char")//若本身是地址，即“传值”
				ObjectCode[P_obj++] = "lw $a2," + yui_itos(temp_line->addr) + "(" + base_reg + ")";
			else
				ObjectCode[P_obj++] = "addi $a2," + base_reg + "," + yui_itos(temp_line->addr);
			
			ObjectCode[P_obj++] = "sw $a2," + yui_itos(t_callee->params[params_counter]->addr) + "($sp)";
			params_counter++;
		}
		else if (q_read->op == "PASS_V")//传递值
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
	
	obj_pass(t_callee, t_caller);//传递参数

	ObjectCode[P_obj++] = "ori $a0,$fp,0";//记下上层基址 动态链

	//生成静态链
	if (t_caller == Table0 || t_callee->Sym_query(t_caller->belong_line->id) != NULL)//被调者能查到调用者，即为递归或子过程调用
	{
		if (t_callee->father == t_caller->father)//若是一次递归或Caller与Callee处于符号表同层
			ObjectCode[P_obj++] = "lw $a1,8($fp)";//静态链同Caller
		else//若是调用子过程
			ObjectCode[P_obj++] = "ori $a1,$fp,0";//静态链指Caller
	}
	else//若是调用符号表之前层的过程，则需依据层次差向上查找静态链
	{
		ObjectCode[P_obj++] = "ori $a1,$fp,0";

		Sym_table *t = t_caller;
		string ID = l_callee->id;
		while (t != NULL && t->Sym_query(ID) != NULL)
		{
			ObjectCode[P_obj++] = "lw $a1,8($a1)";
			t = t->father;
		}
		P_obj--;//按上述循环必然多一次
	}
	
	RT_regs_save();//临时寄存器状态保存
	RT_save_glb(t_caller);//全局寄存器状态保存

	//跳转到过程/函数体
	ObjectCode[P_obj++] = "jal " + q->n1;

	//执行完后
	ObjectCode[P_obj++] = "lw,$fp,4($fp)";//当前基址切回
	ObjectCode[P_obj++] = "subi $sp,$sp," + yui_itos(t_callee->Mem_need);//退栈
	ObjectCode[P_obj++] = "lw $31,12($fp)";//重置$31返回位置--jr自动回填？
	
	RT_regs_recover();//临时寄存器状态恢复
	RT_refresh_glb(t_caller);//全局寄存器状态恢复
	RT_load_glb(t_caller);
}
void obj_return_to(Quatriple *q)//函数返回值语句
{
	string tar_reg = mips_reg->GPR_get_reg(q->n1);//获取新的临时寄存器存放此次函数调用返回值
	ObjectCode[P_obj++] = "ori " + tar_reg + ",$v1,0";
}

void obj_run(Quatriple *q, Sym_table *t)//语句分类处理
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

		ObjectCode[P_obj++] = q->n1 + ":";//过程入口标记

		ObjectCode[P_obj++] = "ori $fp,$sp,0";//$fp存当前基址
		ObjectCode[P_obj++] = "addi $sp,$sp," + yui_itos(t->Mem_need);//栈顶上移
		ObjectCode[P_obj++] = "sw $fp,0($fp)";//AL-基址
		ObjectCode[P_obj++] = "sw $a0,4($fp)";//AL-d_link 动态链
		ObjectCode[P_obj++] = "sw $a1,8($fp)";//AL-s_link 静态链
		ObjectCode[P_obj++] = "sw $ra,12($fp)";//AL-返回位置 jal填到$31
		ObjectCode[P_obj++] = "";

		RT_refresh_glb(t);//全局寄存器刷新
		RT_load_glb(t);

		q = obj_nextQ();
		while (q->op != "RETURN")//在RETURN之前逐个处理四元式
		{
			obj_run(q, t);
			q = obj_nextQ();
		}

		if (t->father != NULL)//NULL即主程序
			ObjectCode[P_obj++] = "jr $31";//返回
		ObjectCode[P_obj++] = "";

		q = obj_nextQ();
	}
}
void obj_data_str()//.data字段填充
{
	for (int i = 0; i < P_str; ++i)
	{
		ObjectCode[P_obj++] = ".data";
		ObjectCode[P_obj++] = Data_str[i];
	}
}

void Obj_generate()
{
	ObjectCode[P_obj++] = "li $sp,0x10040000";//Mars设定
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
