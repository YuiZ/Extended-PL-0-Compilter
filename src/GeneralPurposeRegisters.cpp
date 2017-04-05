/*Í¨ÓÃ¼Ä´æÆ÷¶Ñ_General Purpose Registers*/

#include "pass_second.h"
#include "yui_tools.h"

GPR::GPR(int base, int number)
{
	gpr_base = base;
	gpr_number = number;
	gpr_pointer = 0;

	regs_pool = new string[number];
	for (int i = 0; i < number; ++i)
		regs_pool[i] = "";

	for (int i = 0; i < 3; ++i)
		glb_pool[i] = "";
}

void GPR::GPR_refresh()
{
	for (int i = 0; i < gpr_pointer; ++i)
		regs_pool[i] = "";
	gpr_pointer = 0;
}
string GPR::GPR_get_reg(string temp_name)
{
	for (int i = 0; i < gpr_pointer; ++i)
	{
		if (regs_pool[i] == temp_name)
			return "$" + yui_itos(i + gpr_base);
	}

	if (gpr_pointer < gpr_number)
	{
		regs_pool[gpr_pointer++] = temp_name;
		return "$" + yui_itos(gpr_pointer + gpr_base - 1);
	}
	else
	{
		cout << "STOP------No more Registers!" << endl;
		exit(1);
		return NULL;
	}
}
void GPR::GPR_print()
{
	for (int i = 0; i < gpr_pointer; ++i)
		cout << regs_pool[i] << " - $t" << i + gpr_base << endl;

	for (int i = 0; i < 3; ++i)
		cout << glb_pool[i] << " - $s" << i + 4 << endl;
}

void GPR::GPR_addto_glb(string v1, string v2, string v3)
{
	glb_pool[0] = v1;
	glb_pool[1] = v2;
	glb_pool[2] = v3;
}
string GPR::GPR_get_glb(string tar)
{
	for (int i = 0; i < 3; ++i)
	{
		if (glb_pool[i] == tar)
			return "$s" + yui_itos(i + 4);
	}
	return "";
}
string GPR::GPR_search_glb(string tar)
{
	for (int i = 0; i < 3; ++i)
	{
		if (glb_pool[i] == tar)
			return "$s" + yui_itos(i + 4);
	}
	return "";
}
