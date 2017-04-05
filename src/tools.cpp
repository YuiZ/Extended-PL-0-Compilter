/*工具集_Tools*/

#include "../lib/yui_tools.h"

string yui_itos(int num)
{
	string r = "";
	int t1 = num;
	char t2[10];
	int top = 9;

	if (num < 0)
	{
		r += "-";
		t1 = -num;
	}
	else if (num == 0)
		return "0";

	while (t1 > 0)
	{
		t2[top--] = (char)(t1 % 10 + '0');
		t1 = t1 / 10;
	}

	top++;
	while (top < 10)
		r += t2[top++];

	return r;
}
void yui_split_condit(string condit, string *result)
{
	int k = 0;
	for (int i = 0; i < condit.length(); ++i)
	{
		if (condit[i] != ' ')
			result[k] += condit[i];
		else
			k++;
	}
}

//集合运算
void set_or(bool *S1, bool *S2, bool* R, int Size)
{
	for (int i = 0; i < Size; ++i)
		R[i] = S1[i] | S2[i];
}
void set_sub(bool *S1, bool *S2, bool* R, int Size)
{
	for (int i = 0; i < Size; ++i)
		R[i] = S1[i] & (!S2[i]);
}
bool set_isEqual(bool *S1, bool *S2, int Size)
{
	for (int i = 0; i < Size; ++i)
		if (S1[i] != S2[i])
			return false;
	return true;
}
void set_copy(bool *S_new, bool *S_origin, int Size)
{
	for (int i = 0; i < Size; ++i)
		S_new[i] = S_origin[i];
}
