/*语法分析_Syntax Analysis*/
/*语义分析_Semantic Analysis*/
/*中间代码生成*/

#include "pass_first.h"
#include "pass_second.h"
#include "yui_tools.h"

string Current_Lex;

string SA_Expression(Sym_table *Current_table);
void SA_Call_Proc(Sym_table *Current_table, bool Has_Params = false);
void SA_Call_Func(Sym_table *Caller_table, Sym_line *Callee_line, bool Has_Params = false);
void SA_Block(Sym_table *Current_table);
void SA_State(Sym_table *Current_table);

int SA_Constant(Sym_line *const_line)//取常量
{
	Current_Lex = LA_getlexicon();
	if (Current_Lex == "SYM_NUM")
	{
		const_line->type = "const_int";
		return atoi(Reg_Lex.c_str());
	}
	else if (Current_Lex == "SYM_ADD" || Current_Lex == "SYM_SUB")
	{
		int sign = (Current_Lex == "SYM_SUB") ? -1 : 1;

		Current_Lex = LA_getlexicon();
		if (Current_Lex == "SYM_NUM")
		{
			const_line->type = "const_int";
			return sign * atoi(Reg_Lex.c_str());
		}
	}
	else if (Current_Lex == "SYM_QUOTA_S")
	{
		LA_getlexicon();
		string Ch = Reg_Lex;
		
		Current_Lex = LA_getlexicon();
		if (Ch.length() == 1 && Current_Lex == "SYM_QUOTA_S" && 
			(Ch[0] >= 'a' && Ch[0] <= 'z' || Ch[0] >= 'A' && Ch[0] <= 'Z' || Ch[0] >= '0' && Ch[0] <= '9'))
		{
			const_line->type = "const_char";
			return (int)(Ch[0]);
		}
	}
	
	Error_report(1, "Const declare : Invalid value.");
	return -1;
}
string SA_Type()//取类型
{
	Current_Lex = LA_getlexicon();
	if (Current_Lex == "SYM_INT" || Current_Lex == "SYM_CHAR")
		return Reg_Lex;
	else if (Current_Lex == "SYM_ARRAY")
	{
		string ArrayType = "";

		if (LA_getlexicon() == "SYM_BRACKET_L")
		{
			Current_Lex = LA_getlexicon();
			if (Current_Lex == "SYM_NUM")
			{
				ArrayType = Reg_Lex;
				if (LA_getlexicon() != "SYM_BRACKET_R")
					Error_report(2, "Array declare : ']' not found.");
			}
			else
				Error_report(2, "Array declare : Invalid <Unsigned Integer>. => \"" + Current_Lex + "\"");
		}
		else
			Error_report(2, "Array declare : '[' expected.");

		if (LA_getlexicon() == "SYM_OF")
		{
			Current_Lex = LA_getlexicon();
			if (Current_Lex == "SYM_INT" || Current_Lex == "SYM_CHAR")
				return ArrayType;
			else
				Error_report(2, "Array declare : Invalid <Base Type>. => \"" + Current_Lex + "\"");
		}
		else
			Error_report(2, "Array declare : \"OF\" not found.");
	}
	else
		Error_report(2, "Var declare : Invalid <Type>. => \"" + Current_Lex + "\"");

	return "";
}

//<表达式>相关，读到表达式后一个token
string SA_ArrayScript(Sym_table *Current_table, string array_name, Quatriple** Exp_Buff, int *P_eb, bool is_assign = false)//数组脚标
{
	string temp_str = SA_Expression(Current_table);

	if (Current_Lex != "SYM_BRACKET_R")//检查']'匹配
		Error_report(2, "Expression array element : ']' not found.");
		
	if (is_assign)
		return temp_str;
	else
	{
		string temp = SeA_GetTempName();
		Exp_Buff[(*P_eb)++] = new Quatriple("[]", temp, array_name, temp_str);
		return temp;
	}
}
string SA_Factor(Sym_table *Current_table, Quatriple** Exp_Buff, int *P_eb)//因子
{
	Sym_line* temp_line;
	string temp_lex, temp_str;

	if (Current_Lex == "SYM_IDENT")
	{
		temp_line = Current_table->Sym_query(Reg_Lex);
		if (temp_line != NULL)
		{
			if (temp_line->type == "func_int" || temp_line->type == "func_char")//函数标识符
			{
				temp_lex = LA_getlexicon();
				if (temp_lex == "SYM_PAREN_L")
					SA_Call_Func(Current_table, temp_line, true);
				else
				{
					SA_Call_Func(Current_table, temp_line);
					Current_Lex = temp_lex;
				}
				temp_str = SeA_GetTempName();
				QuatriCode[P_qc++] = new Quatriple("RETURN_TO", "", temp_str);
				return temp_str;
			}
			else//常量、变量标识符
			{
				temp_str = Reg_Lex;
				temp_lex = LA_getlexicon();
				if (temp_lex == "SYM_BRACKET_L")//数组元素
				{
					temp_str = SA_ArrayScript(Current_table, Reg_Lex, Exp_Buff, P_eb);
					Current_Lex = LA_getlexicon();
				}
				else
					Current_Lex = temp_lex;

				return temp_str;
			}
		}
		else
			Error_report(3, "Expression : Undefine identifier. => \"" + Reg_Lex + "\"");

		return "UNKNOWN FACTOR";
	}
	else if (Current_Lex == "SYM_NUM")
	{
		temp_str = Reg_Lex;
		Current_Lex = LA_getlexicon();
		return temp_str;
	}
	else if (Current_Lex == "SYM_PAREN_L")
	{
		temp_str = SA_Expression(Current_table);
		if (Current_Lex != "SYM_PAREN_R")
			Error_report(2, "Expression : ')' nod found.");
	
		Current_Lex = LA_getlexicon();
		return temp_str;
	}
	else
	{
		Error_report(2, "Expression : Invalid factor. => \"" + Current_Lex + "\"");
		return "";
	}
}
string SA_Term(Sym_table *Current_table, Quatriple** Exp_Buff, int *P_eb)//项
{
	string Factor1, Factor2, op, Result;

	Factor1 = SA_Factor(Current_table, Exp_Buff, P_eb);
	while (Current_Lex == "SYM_MUL" || Current_Lex == "SYM_DIV")
	{
		op = (Current_Lex == "SYM_MUL") ? "MUL" : "DIV";
		Current_Lex = LA_getlexicon();
		Factor2 = SA_Factor(Current_table, Exp_Buff, P_eb);
		
		if (Factor1[0] == '_')
			Result = Factor1;
		else if (Factor2[0] == '_')
			Result = Factor2;
		else
			Result = SeA_GetTempName();
			
		Exp_Buff[(*P_eb)++] = new Quatriple(op, Result, Factor1, Factor2);
		Factor1 = Result;
	}
	return Factor1;
}
string SA_Expression(Sym_table *Current_table)//表达式
{
	Current_Lex = LA_getlexicon();
	if (Current_Lex == "SYM_STRING" || Current_Lex == "SYM_EQUAL" || Current_Lex == "SYM_BIGGER")
		return Current_Lex;
	
	Quatriple* Exp_Buff[64];//将表达式计算所生成的四元式先存入Buff，当整个表达式分析结束后一并送入QutriCode数组
	int P_eb = 0;//目的是让四元式生成时先将表达式内的函数调用计算完成，再计算表达式
	string Factor1,op,Factor2,Result;
	
	if (Current_Lex == "SYM_ADD" || Current_Lex == "SYM_SUB")
	{
		op = Current_Lex;
		Current_Lex = LA_getlexicon();
	}

	Factor1 = SA_Term(Current_table, Exp_Buff, &P_eb);
	if (op == "SYM_SUB")
	{
		Result = SeA_GetTempName();

		Exp_Buff[P_eb++] = new Quatriple("SUB", Result, "", Factor1);
		Factor1 = Result;
	}
	while (Current_Lex == "SYM_ADD" || Current_Lex == "SYM_SUB")
	{
		op = (Current_Lex == "SYM_ADD")?"ADD":"SUB";
		Current_Lex = LA_getlexicon();
		Factor2 = SA_Term(Current_table, Exp_Buff, &P_eb);
		
		if (Factor1[0] == '_')
			Result = Factor1;
		else if (Factor2[0] == '_')
			Result = Factor2;
		else
			Result = SeA_GetTempName();

		Exp_Buff[P_eb++] = new Quatriple(op, Result, Factor1, Factor2);
		Factor1 = Result;
	}

	for (int i = 0; i < P_eb; ++i)
		QuatriCode[P_qc++] = Exp_Buff[i];
		
	return Factor1;
}
string SA_Condition(Sym_table *Current_table)//<条件>，读到后一个token
{
	string left, right, op;

	left = SA_Expression(Current_table);

	if (Current_Lex == "SYM_EQUAL")//=
	{
		right = SA_Expression(Current_table);
		op = "==";
	}
	else if (Current_Lex == "SYM_LESS")//<
	{
		right = SA_Expression(Current_table);
		op = "<";
		if (Current_Lex == "SYM_EQUAL")//<=
		{
			right = SA_Expression(Current_table);
			op += "=";
		}
		else if (Current_Lex == "SYM_BIGGER")//<>
		{
			right = SA_Expression(Current_table);
			op += ">";
		}
	}
	else if (Current_Lex == "SYM_BIGGER")//>
	{
		right = SA_Expression(Current_table);
		op = ">";
		if (Current_Lex == "SYM_EQUAL")//>=
		{
			right = SA_Expression(Current_table);
			op += "=";
		}
	}
	else
		Error_report(2,"Condition : Invalid <operateor>. => \"" + Current_Lex + "\"");

	if (!(Current_Lex == "SYM_THEN" || Current_Lex == "SYM_END" || Current_Lex == "SYM_SEMICOLON"))//<条件>后只有以上情况？
		Error_report(2, "Condition : Unexpected following token. => \"" + Current_Lex + "\"");

	return left + " " + op + " " + right;
}

//<参数表>，读到自身结束位置！(一般是')')
void SA_Formal_Params(Sym_table *Current_table)//形参表，读至右括号
{	
	bool isVar;
	string temp_param[10];

SIG_more_param:
	isVar = false;
	Current_Lex = LA_getlexicon();
	if (Current_Lex == "SYM_VAR")//若有var起始
	{
		isVar = true;
		Current_Lex = LA_getlexicon();
	}

	if (Current_Lex == "SYM_IDENT")
	{
		int params_counter = 0;
		do {
			if (Current_Lex == "SYM_COMMA")
				Current_Lex = LA_getlexicon();

			if (Current_Lex == "SYM_IDENT")
			{
				if (Current_table->Sym_query(Reg_Lex,true) == NULL)//查表
				{
					temp_param[params_counter++] = Reg_Lex;
					Current_Lex = LA_getlexicon();
				}
				else
					Error_report(3, "Formal params : Multiple definition. => \"" + Reg_Lex + "\"");
			}
			else
				Error_report(2, "Formal params : Invalid param. => \"" + Current_Lex + "\"");
		} while (Current_Lex == "SYM_COMMA");

		if (Current_Lex == "SYM_COLON")
		{
			string temp_str = SA_Type();
			if (temp_str == "integer" || temp_str == "char")
			{
				if (isVar)//Var参数为地址
					temp_str = (temp_str == "integer")? "addr_int" : "addr_char";
				
				for (int i = 0; i < params_counter; ++i)
					Current_table->Sym_fill(temp_param[i], temp_str, true);
			}
			else//非法参数类型
				Error_report(2, "Formal params : Invalid param <Type>. => \"" + temp_str + "\"");
						
			Current_Lex = LA_getlexicon();
			if (Current_Lex == "SYM_PAREN_R")
				return;
			else if (Current_Lex == "SYM_SEMICOLON")
				goto SIG_more_param;
			else
				Error_report(2, "Formal params : ')' not found or unexpected token. => \"" + Current_Lex + "\"");
		}
		else
			Error_report(2, "Formal params : ':' expected.");
	}
	else
		Error_report(2, "Formal params : Invalid param <Ident>. => \"" + Current_Lex + "\"");
}
void SA_Actual_Params(Sym_table *Caller_table, Sym_table *Callee_table)//实参表
{
	Quatriple *AP_Buff[Params_Max_Size];
	int P_ap = 0;

	string temp_param,temp_type;
	
	for (int i = 0; i < Callee_table->P_st_param; ++i)
	{
		temp_type = Callee_table->params[i]->type;//取形参规定的参数类型
		temp_param = SA_Expression(Caller_table);

		if (temp_type == "addr_int" || temp_type == "addr_char")//传地址
		{
			if (temp_param[0] == '_')
			{
				if (QuatriCode[P_qc - 1]->op == "[]")
				{
					if (SeA_TypeCheck(Caller_table, temp_type, QuatriCode[P_qc - 1]->n1))
					{
						QuatriCode[P_qc - 1]->r = Callee_table->belong_line->id;
						QuatriCode[P_qc - 1]->op = "PASS_A";
						AP_Buff[P_ap++] = QuatriCode[P_qc - 1];
						P_qc--;
					}
					else
						Error_report(3, "Wrong param type : <" + temp_type + "> expected.");
				}
				else
					Error_report(3, "Wrong param type : <" + temp_type + "> expected.");
			}
			else
			{
				if (SeA_TypeCheck(Caller_table, temp_type, temp_param))
					AP_Buff[P_ap++] = new Quatriple("PASS_A", Callee_table->belong_line->id, temp_param);
				else
					Error_report(3, "Wrong param type : <" + temp_type + "> expected.");
			}
		}
		else//传值
		{ 
			if (SeA_TypeCheck(Caller_table, temp_type, temp_param))
				AP_Buff[P_ap++] = new Quatriple("PASS_V", Callee_table->belong_line->id, temp_param);
			else
				Error_report(3, "Wrong param type : <" + temp_type + "> expected.");
		}
	}
	if (Current_Lex != "SYM_PAREN_R")
		Error_report(2, "Actual params : ')' nod found.");

	for (int i = 0; i < P_ap; ++i)
		QuatriCode[P_qc++] = AP_Buff[i];
}

//<语句>，读到语句后第一个token(通常为';')
void SA_Read(Sym_table *Current_table)//读语句
{
	Current_Lex = LA_getlexicon();
	if (Current_Lex == "SYM_PAREN_L")
	{
		Current_Lex = LA_getlexicon();
		Sym_line *p;
		while (Current_Lex == "SYM_IDENT")
		{
			p = Current_table->Sym_query(Reg_Lex);
			if (p != NULL)
			{
				if (!(p->type == "proc" || p->type == "func_int" || p->type == "func_char"))
					QuatriCode[P_qc++] = new Quatriple("Sys_Read", Reg_Lex,"");
				else
					Error_report(3, "Read : Invalid param type.  => \"" + p->type + "\"");
			}
			else
				Error_report(3,"Read : Undefine identifier. => \"" + Reg_Lex + "\"");

			Current_Lex = LA_getlexicon();
			if (Current_Lex == "SYM_COMMA")
			{
				Current_Lex = LA_getlexicon();
				if (Current_Lex != "SYM_IDENT")
					Error_report(2, "Read : Expect a <Ident>. => \"" + Current_Lex + "\"");
			}
			else
				break;
		}
		if (Current_Lex == "SYM_PAREN_R")
			Current_Lex = LA_getlexicon();
		else
			Error_report(2, "Read : ')' nod found.");
	}
	else
		Error_report(2, "Read : '(' expected.");
}
void SA_Write(Sym_table *Current_table)//写语句
{
	string temp_out;

	if (LA_getlexicon() == "SYM_PAREN_L")
	{
		temp_out = SA_Expression(Current_table);
		if (temp_out == "SYM_STRING")//返回字符串
		{
			QuatriCode[P_qc++] = new Quatriple("Sys_Write","",Reg_Lex);
			
			Current_Lex = LA_getlexicon();
			if (Current_Lex == "SYM_COMMA")
			{
				temp_out = SA_Expression(Current_table);
				QuatriCode[P_qc++] = new Quatriple("Sys_Write", "", temp_out);
			}
		}
		else if (!(temp_out == "SYM_EQUAL" || temp_out == "SYM_BIGGER"))//返回表达式
			QuatriCode[P_qc++] = new Quatriple("Sys_Write", "", temp_out);
		else//Expression的其它返回情况
			Error_report(2, "Write : Invalid param.  => \"" + Current_Lex + "\"");

		if (Current_Lex != "SYM_PAREN_R")
			Error_report(2, "Write : ')' nod found.");
		
		Current_Lex = LA_getlexicon();
	}
	else
		Error_report(2, "Write : '(' expected.");
}
void SA_Assign(Sym_table *Current_table, bool Is_Array = false)//赋值语句
{
	string temp_target = Reg_Lex;
	string temp_value;
	string offset = "";

	Sym_line *p = Current_table->Sym_query(temp_target);
	if (p != NULL)
	{
		if (!(p->type == "const_int" || p->type == "const_char" || p->type == "proc"))
		{
			if (Is_Array)//数组
			{
				offset = SA_ArrayScript(Current_table, temp_target, NULL, NULL, true);
				if (LA_getlexicon() != "SYM_COLON")
					Error_report(2, "Assign : ':' expected.");
			}

			if (LA_getlexicon() == "SYM_EQUAL")
			{
				temp_value = SA_Expression(Current_table);
				QuatriCode[P_qc++] = new Quatriple("ASSIGN", temp_target, temp_value, offset);
			}
			else
				Error_report(2, "Assign : '=' expected.");
		}
		else
			Error_report(3, "Assign : Can't assign to a <" + p->type + ">.");
	}
	else
		Error_report(3, "Assign : Undefine identifier. => \"" + Reg_Lex + "\"");
}
void SA_Call_Proc(Sym_table *Caller_table, bool Has_Params)//过程调用
{
	Sym_line *Callee_line = Caller_table->Sym_query(Reg_Lex);
	
	if (Callee_line == NULL || Callee_line->type != "proc")
		Error_report(3, "Proc Call : Undefine procedure. => \"" + Reg_Lex + "\"");
	
	if ((Has_Params && Callee_line->sub_table->P_st_param == 0) || ((!Has_Params) && Callee_line->sub_table->P_st_param > 0))
		Error_report(3, "Proc Call : Params can't match. => \"" + Callee_line->id + "\"");
	
	if (Callee_line->sub_table->father != NULL)
		QuatriCode[P_qc++] = new Quatriple("CALL", "", Callee_line->sub_table->father->belong_line->id + "_" + Callee_line->id, Callee_line->id, Caller_table);
	else
		QuatriCode[P_qc++] = new Quatriple("CALL", "", "main_" +Callee_line->id, Callee_line->id, Caller_table);//四元式传递调用者符号表指针

	if (Has_Params)
	{
		SA_Actual_Params(Caller_table, Callee_line->sub_table);//处理实际参数表
		Current_Lex = LA_getlexicon();
	}
}
void SA_Call_Func(Sym_table *Caller_table, Sym_line *Callee_line, bool Has_Params)//函数调用
{
	Sym_table *Callee_table = Callee_line->sub_table;

	if ((Has_Params && Callee_table->P_st_param == 0) || ((!Has_Params) && Callee_table->P_st_param > 0))
		Error_report(3, "Func Call : Params can't match. => \"" + Callee_line->id + "\"");
	
	if (Callee_table->father != NULL)
		QuatriCode[P_qc++] = new Quatriple("CALL", "", Callee_table->father->belong_line->id + "_" + Callee_line->id, Callee_line->id, Caller_table);
	else
		QuatriCode[P_qc++] = new Quatriple("CALL", "", "main_" + Callee_line->id, Callee_line->id, Caller_table);
	
	if (Has_Params)
	{
		SA_Actual_Params(Caller_table, Callee_table);//处理实际参数表
		Current_Lex = LA_getlexicon();
	}
	else
		Current_Lex = "SYM_SEMICOLON";
}
void SA_IF(Sym_table *Current_table)
{
	int temp_loc;
	string temp_condit;
	
	temp_condit = SA_Condition(Current_table);
	
	temp_loc = P_qc;
	QuatriCode[P_qc++] = new Quatriple("goto","if not","" ,temp_condit);

	if (Current_Lex == "SYM_THEN")
	{
		Current_Lex = LA_getlexicon();
		SA_State(Current_table);
		QuatriCode[temp_loc]->n1 = yui_itos(P_qc);
		
		if (Current_Lex == "SYM_ELSE")
		{
			QuatriCode[temp_loc]->n1 = yui_itos(P_qc+1);
			temp_loc = P_qc;
			QuatriCode[P_qc++] = new Quatriple("goto", "then", "");
			
			Current_Lex = LA_getlexicon();
			SA_State(Current_table);
			
			QuatriCode[temp_loc]->n1 = yui_itos(P_qc);
		}
	}
	else
		Error_report(2, "IF : \"THEN\" expected. => \"" + Current_Lex + "\"");
}
void SA_DO_WHILE(Sym_table *Current_table)
{
	int temp_start = P_qc;

	Current_Lex = LA_getlexicon();
	SA_State(Current_table);

	if (Current_Lex == "SYM_WHILE")
	{
		string temp_condit = SA_Condition(Current_table);
		
		QuatriCode[P_qc++] = new Quatriple("goto", "if", yui_itos(temp_start), temp_condit);

		if (Current_Lex != "SYM_SEMICOLON" && Current_Lex != "SYM_END")
			Error_report(2, "DO-WHILE : Unexpected following token. => \"" + Current_Lex + "\"");
	}
	else
		Error_report(2, "DO-WHILE : \"WHILE\" expected. => \"" + Current_Lex + "\"");
}
void SA_FOR(Sym_table *Current_table)
{
	Current_Lex = LA_getlexicon();
	if (Current_Lex == "SYM_IDENT")
	{
		string temp_lex = Reg_Lex;
		Sym_line *p = Current_table->Sym_query(Reg_Lex);
		
		if (p != NULL)
		{
			if (p->type == "integer")
			{
				Current_Lex = LA_getlexicon();
				if (Current_Lex == "SYM_COLON")
				{
					SA_Assign(Current_table);
					if (Current_Lex == "SYM_TO" || Current_Lex == "SYM_DOWNTO")
					{
						bool temp_sig = (Current_Lex == "SYM_TO") ? true : false;
						string temp_condit = temp_lex;
											
						temp_condit += (temp_sig)? " <= ": " >= ";
						temp_condit += SA_Expression(Current_table);

						if (Current_Lex == "SYM_DO")
						{
							Current_Lex = LA_getlexicon();
							int temp_start = P_qc;
							SA_State(Current_table);
							QuatriCode[P_qc++] = new Quatriple((temp_sig)?"ADD":"SUB",temp_lex,temp_lex,"1");
							QuatriCode[P_qc++] = new Quatriple("goto", "if", yui_itos(temp_start), temp_condit);
						}
						else
							Error_report(2, "FOR : \"DO\" expected. => \"" + Current_Lex + "\"");
					}
					else
						Error_report(2, "FOR : \"TO\" or \"DOWNTO\" expected. => \"" + Current_Lex + "\"");
				}
				else
					Error_report(2, "FOR : ':' expected. => \"" + Current_Lex + "\"");
			}
			else
				Error_report(3, "FOR : Loop variable should be integer. => \"" + p->id + "\"" + "-" + p->type);
		}
		else
			Error_report(3, "FOR : Undefine identifier. => \"" + Reg_Lex + "\"");
	}
	else
		Error_report(2, "For : Invalid loop variable <Ident>. => \"" + Current_Lex + "\"");
}

void SA_State(Sym_table *Current_table)
{
	string temp_lex = "";

	if (Current_Lex == "SYM_SEMICOLON")//空语句
		;
	else if (Current_Lex == "SYM_READ")
		SA_Read(Current_table);
	else if (Current_Lex == "SYM_WRITE")
		SA_Write(Current_table);
	else if (Current_Lex == "SYM_IDENT")
	{
		temp_lex = LA_getlexicon();
		if (temp_lex == "SYM_COLON")//一般赋值
			SA_Assign(Current_table);
		else if (temp_lex == "SYM_BRACKET_L")//数组元素赋值
			SA_Assign(Current_table, true);
		else if (temp_lex == "SYM_SEMICOLON" || temp_lex == "SYM_END")//无参过程调用
		{
			SA_Call_Proc(Current_table);
			Current_Lex = temp_lex;
		}
		else if (temp_lex == "SYM_PAREN_L")//有参过程调用
			SA_Call_Proc(Current_table, true);
		else
			Error_report(2, "State : Unexpected token after <Ident>. => \"" + Current_Lex + "\"");
	}
	else if (Current_Lex == "SYM_IF")
		SA_IF(Current_table);
	else if (Current_Lex == "SYM_DO")
		SA_DO_WHILE(Current_table);
	else if (Current_Lex == "SYM_FOR")
		SA_FOR(Current_table);
	else if (Current_Lex == "SYM_BEGIN")
		SA_Block(Current_table);
	else if (Current_Lex == "SYM_END")//若end前为空语句，则state会遇到end
		;
	else
		Error_report(2, "State : Unexpected inlet token! => \"" + Current_Lex + "\"");
}

//<分程序>，五个部分，读到下一部分起始的token
void SA_Const(Sym_table *Current_table)
{
	Current_Lex = LA_getlexicon();
	while (Current_Lex != "SYM_SEMICOLON")
	{
		if (Current_Lex == "SYM_IDENT")
		{
			Current_table->Sym_query(Reg_Lex);
			if (Current_table->Sym_query(Reg_Lex) == NULL)
			{
				if (LA_getlexicon() == "SYM_EQUAL")
				{
					Sym_line *p = Current_table->Sym_fill(Reg_Lex, "const");
					int temp_value = SA_Constant(p);
					p->addr = temp_value;
					
					Current_Lex = LA_getlexicon();
					if (Current_Lex == "SYM_SEMICOLON")//分号结束
					{
						Current_Lex = LA_getlexicon();
						return;
					}
					else if (Current_Lex == "SYM_COMMA")
						continue;
					else
						Error_report(2, "Const declare : Unexpected token. => \"" + Current_Lex + "\"");
				}
				
				else
					Error_report(2, "Const declare : '=' expected. => \"" + Current_Lex + "\"");
			}
			else if (Reg_Lex == Current_table->belong_line->id)
				Error_report(3, "Const declare : Multiple definition. Same as Proc/Func. => \"" + Reg_Lex + "\"");
			else
				Error_report(3, "Const declare : Multiple definition. => \"" + Reg_Lex + "\"");
		}
		else if (Current_Lex == "SYM_COMMA")
		{
			Current_Lex = LA_getlexicon();
			if (Current_Lex != "SYM_IDENT")
				Error_report(2, "Const declare : Unexpected token. => \"" + Current_Lex + "\"");
		}
		else
			Error_report(2, "Const declare : Unexpected token. => \"" + Current_Lex + "\"");
	}
}
void SA_Var(Sym_table *Current_table)
{
	string temp_str;
	int var_start;

	Current_Lex = LA_getlexicon();

SIG_more_var:
	var_start = Current_table->P_st_line;
	
	do {
		if (Current_Lex == "SYM_COMMA")
			Current_Lex = LA_getlexicon();

		if (Current_Lex == "SYM_IDENT")
		{
			if (Reg_Lex != Current_table->belong_line->id)
			{
				if (Current_table->Sym_query(Reg_Lex, true) == NULL)//查表
				{
					Current_table->Sym_fill(Reg_Lex, "unknown");
					Current_Lex = LA_getlexicon();
				}
				else
					Error_report(3, "Var declare : Multiple definition. => \"" + Reg_Lex + "\"");
			}
			else
				Error_report(3, "Var declare : Multiple definition. Same as Proc/Func. => \"" + Reg_Lex + "\"");
		}
		else
			Error_report(2, "Var declare : Unexpected token. => \"" + Current_Lex + "\"");
	} while (Current_Lex == "SYM_COMMA");

	if (Current_Lex == "SYM_COLON")
	{
		temp_str = SA_Type();
		if (temp_str == "integer" || temp_str == "char")
		{
			for (int i = var_start; i < Current_table->P_st_line; ++i)
				Current_table->lines[i]->type = temp_str;
		}
		else
		{
			for (int i = var_start; i < Current_table->P_st_line; ++i)
			{
				Current_table->lines[i]->type = Reg_Lex;
				Current_table->lines[i]->length = atoi(temp_str.c_str());
			}
		}
	
		Current_Lex = LA_getlexicon();
		if (Current_Lex == "SYM_SEMICOLON")
		{
			Current_Lex = LA_getlexicon();
			if (Current_Lex == "SYM_IDENT")//可能有下一个变量
				goto SIG_more_var;
		}
		else
			Error_report(2, "Var declare : Unexpected token. => \"" + Current_Lex + "\"");
	}
	else
		Error_report(2, "Var declare : ':' expected. => \"" + Current_Lex + "\"");
}
void SA_Proc(Sym_table *Current_table)
{
	Current_Lex = LA_getlexicon();
	if (Current_Lex == "SYM_IDENT")
	{
		if (Current_table->Sym_query(Reg_Lex,true) == NULL)
		{
			Sym_line* p = Current_table->Sym_fill(Reg_Lex, "proc");
			p->sub_table = new Sym_table(Current_table, p);

			Current_Lex = LA_getlexicon();
			if (Current_Lex == "SYM_PAREN_L")//处理形式参数表
			{
				SA_Formal_Params(p->sub_table);
				Current_Lex = LA_getlexicon();
			}
			if (Current_Lex == "SYM_SEMICOLON")
			{
				SA_process(p->sub_table);
				
				if (Current_Lex == "SYM_SEMICOLON")
					Current_Lex = LA_getlexicon();
				else
					Error_report(2, "Proc declare : ';' expected when block end. => \"" + Current_Lex + "\"");
			}
			else
				Error_report(2, "Proc declare : ';' expected when declared. => \"" + Current_Lex + "\"");
		}
		else
			Error_report(3, "Proc declare : Multiple definition. => \"" + Reg_Lex + "\"");
	}
	else
		Error_report(2, "Proc declare : <Ident> expected. => \"" + Current_Lex + "\"");
}
void SA_Func(Sym_table *Current_table)
{
	Current_Lex = LA_getlexicon();
	if (Current_Lex == "SYM_IDENT")
	{
		if (Current_table->Sym_query(Reg_Lex,true) == NULL)
		{
			Sym_line *p = Current_table->Sym_fill(Reg_Lex,"func");
			p->sub_table = new Sym_table(Current_table, p);

			Current_Lex = LA_getlexicon();
			if (Current_Lex == "SYM_PAREN_L")
			{
				SA_Formal_Params(p->sub_table);
				Current_Lex = LA_getlexicon();
			}
			if (Current_Lex == "SYM_COLON")
			{
				p->type = "func_" + SA_Type();
				if (p->type == "func_integer")//仅为简洁..
					p->type = "func_int";

				if (LA_getlexicon() != "SYM_SEMICOLON")
					Error_report(2, "Func declare : ';' expected when declared. => \"" + Current_Lex + "\"");
				
				SA_process(p->sub_table);

				if (Current_Lex == "SYM_SEMICOLON")
					Current_Lex = LA_getlexicon();
				else
					Error_report(2, "Func declare : ';' expected when block end. => \"" + Current_Lex + "\"");
			}
			else
				Error_report(2, "Func declare : ':' expected.");
		}
		else
			Error_report(3, "Func declare : Multiple definition. => \"" + Reg_Lex + "\"");
	}
	else
		Error_report(2, "Func declare : <Ident> expected. => \"" + Current_Lex + "\"");
}
void SA_Block(Sym_table *Current_table)
{
	Current_Lex = LA_getlexicon();
	SA_State(Current_table);
	
	while (Current_Lex == "SYM_SEMICOLON")
	{
		Current_Lex = LA_getlexicon();
		SA_State(Current_table);
	}

	if (Current_Lex == "SYM_END")
		Current_Lex = LA_getlexicon();
	else if (Current_Lex == "SYM_PERIOD")
		return;
	else
		Error_report(0, "Block : Unexpected situation! => \"" + Reg_Lex + "\" - " + Current_Lex);
}

void SA_process(Sym_table *Current_table)
{
	int Order_sig = 0;//确保 <常量><变量> (<过程>|<函数>) <复合语句> 顺序

	Current_Lex = LA_getlexicon();
	while (Order_sig < 4 && !File_End)
	{
		if (Current_Lex == "SYM_CONST" && Order_sig <= 0)//常量声明
		{
			Order_sig = 1;
			SA_Const(Current_table);
		}
		else if (Current_Lex == "SYM_VAR" && Order_sig <= 1)//变量声明
		{
			Order_sig = 2;
			SA_Var(Current_table);
		}
		else if (Current_Lex == "SYM_PROC" && Order_sig <= 3)//过程声明
		{
			Order_sig = 3;
			SA_Proc(Current_table);
		}
		else if (Current_Lex == "SYM_FUNC" && Order_sig <= 3)//函数声明
		{
			Order_sig = 3;
			SA_Func(Current_table);
		}
		else if (Current_Lex == "SYM_BEGIN")//语句块
		{
			Order_sig = 4;
			if (Current_table->father != NULL)
				QuatriCode[P_qc++] = new Quatriple("ENTER", "", Current_table->father->belong_line->id + "_" + Current_table->belong_line->id, "", Current_table);
			else
				QuatriCode[P_qc++] = new Quatriple("ENTER", "", "main_" + Current_table->belong_line->id, "", Current_table);
			SA_Block(Current_table);
		}
		else if (Current_Lex == "SYM_PERIOD")//终止标记
			cout << "----Single period, the end of this program." << endl;
		else if (Current_Lex == "SYM_CONST" || Current_Lex == "SYM_VAR" || Current_Lex == "SYM_PROC" || Current_Lex == "SYM_FUNC")//出现顺序错乱者
			Error_report(2, "Program process : Unexpected order [CONST -> VAR -> PROC|FUNC -> BLOCK] => " + Current_Lex);
		else//其余情况
			Error_report(2, "Program process : Unexpected inlet token!=> \"" + Current_Lex + "\"");
	}

	if (Order_sig >= 4)
		QuatriCode[P_qc++] = new Quatriple("RETURN", "", "");
	else
		Error_report(2, "Program process : Incomplete program structure of <" + Current_table->belong_line->id + ">.");
}
