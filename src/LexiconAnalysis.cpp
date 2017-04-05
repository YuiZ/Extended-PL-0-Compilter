/*???????_Lexicon Analysis*/

#include "../lib/pass_first.h"

FILE *fin;

int Counter_Line, Counter_Lex;
bool File_End = false;

map<string, string> KeyWordTable;//???????
char Buf_Line[Buf_Line_Size];
char Buf_Lex[Buf_Lex_Size];
int P_line, P_lex;
char Char_temp;

string Reg_Lex;//????洢TOKEN????

char getch()//???????
{
	if (Buf_Line[P_line] == '\0' || Buf_Line[P_line] == '\n')
	{
		memset(Buf_Line, '\0', sizeof(Buf_Line));
		P_line = 0;
		if (fgets(Buf_Line, Buf_Line_Size, fin) == NULL)
		{
			//cout << "---------------------------------------------------File End!" << endl;
			File_End = true;
			fclose(fin);
			return '\0';
		}
		else
		{
			Counter_Line++;
			Counter_Lex = 1;
			return '\0';
		}
	}
	return Buf_Line[P_line++];
}
string take_string()//???????????????????????????
{
	char Buf_Str[128];//????????128
	int P_str = 0;

	memset(Buf_Str, '\0', sizeof(Buf_Str));

	while (P_str < 128 && Char_temp != 34 && Char_temp >= 32 && Char_temp <= 126)
	{
		Buf_Str[P_str++] = Char_temp;
		Char_temp = getch();
	}
	if (Char_temp == '"')
	{
		string result = "\"";
		result += string(Buf_Str);
		result += "\"";
		return result;
	}
	
	Error_report(1, "Error when read a string, '\"' expected.");
	return "Error in take_string()!";
}

void LexiconAnalysis_init()
{
	string route;

	cout << "Please enter the route of source file:" << endl;
	cin >> route;
	fin = fopen(route.c_str(), "r");
	if (fin == NULL)
		Error_report(0, "Invalid source file.");
	
	memset(Buf_Line, '\0', sizeof(Buf_Line));
	memset(Buf_Lex, '\0', sizeof(Buf_Lex));
	P_line = 0;
	P_lex = 0;

	Char_temp = '\0';

	Counter_Line = 0;
	Counter_Lex = 1;

	/*????????????*/
	KeyWordTable.insert(pair<string, string>("const", "SYM_CONST"));
	KeyWordTable.insert(pair<string, string>("var", "SYM_VAR"));
	KeyWordTable.insert(pair<string, string>("procedure", "SYM_PROC"));
	KeyWordTable.insert(pair<string, string>("function", "SYM_FUNC"));
	KeyWordTable.insert(pair<string, string>("integer", "SYM_INT"));
	KeyWordTable.insert(pair<string, string>("char", "SYM_CHAR"));
	KeyWordTable.insert(pair<string, string>("array", "SYM_ARRAY"));
	KeyWordTable.insert(pair<string, string>("of", "SYM_OF"));
	KeyWordTable.insert(pair<string, string>(":", "SYM_COLON"));
	KeyWordTable.insert(pair<string, string>(";", "SYM_SEMICOLON"));
	KeyWordTable.insert(pair<string, string>("=", "SYM_EQUAL"));
	KeyWordTable.insert(pair<string, string>("+", "SYM_ADD"));
	KeyWordTable.insert(pair<string, string>("-", "SYM_SUB"));
	KeyWordTable.insert(pair<string, string>("*", "SYM_MUL"));
	KeyWordTable.insert(pair<string, string>("/", "SYM_DIV"));
	KeyWordTable.insert(pair<string, string>("[", "SYM_BRACKET_L"));
	KeyWordTable.insert(pair<string, string>("]", "SYM_BRACKET_R"));
	KeyWordTable.insert(pair<string, string>("(", "SYM_PAREN_L"));
	KeyWordTable.insert(pair<string, string>(")", "SYM_PAREN_R"));
	KeyWordTable.insert(pair<string, string>("begin", "SYM_BEGIN"));
	KeyWordTable.insert(pair<string, string>("end", "SYM_END"));
	KeyWordTable.insert(pair<string, string>(",", "SYM_COMMA"));
	KeyWordTable.insert(pair<string, string>(".", "SYM_PERIOD"));
	KeyWordTable.insert(pair<string, string>("'", "SYM_QUOTA_S"));
	KeyWordTable.insert(pair<string, string>("\"", "SYM_QUOTA_D"));
	KeyWordTable.insert(pair<string, string>("<", "SYM_LESS"));
	KeyWordTable.insert(pair<string, string>(">", "SYM_BIGGER"));
	KeyWordTable.insert(pair<string, string>("do", "SYM_DO"));
	KeyWordTable.insert(pair<string, string>("while", "SYM_WHILE"));
	KeyWordTable.insert(pair<string, string>("for", "SYM_FOR"));
	KeyWordTable.insert(pair<string, string>("to", "SYM_TO"));
	KeyWordTable.insert(pair<string, string>("downto", "SYM_DOWNTO"));
	KeyWordTable.insert(pair<string, string>("if", "SYM_IF"));
	KeyWordTable.insert(pair<string, string>("then", "SYM_THEN"));
	KeyWordTable.insert(pair<string, string>("else", "SYM_ELSE"));
	KeyWordTable.insert(pair<string, string>("read", "SYM_READ"));
	KeyWordTable.insert(pair<string, string>("write", "SYM_WRITE"));
}
string LA_getlexicon()//?????????????Value????Reg_Lex????????Type
{
	Counter_Lex++;
	Char_temp = getch();

	while (Char_temp == ' ' || Char_temp == '\t' || Char_temp == '\r')
		Char_temp = getch();

	P_lex = 0;
	memset(Buf_Lex, '\0', sizeof(Buf_Lex));

	Buf_Lex[P_lex++] = Char_temp;

	int P_read = 0;//??????

	if ((Buf_Lex[P_read] >= 'a' &&  Buf_Lex[P_read] <= 'z') || (Buf_Lex[P_read] >= 'A' &&  Buf_Lex[P_read] <= 'Z'))//??????
	{
		Char_temp = getch();
		while (P_lex < Buf_Lex_Size &&
			((Char_temp >= 'A' && Char_temp <= 'Z') ||
				(Char_temp >= 'a' && Char_temp <= 'z') ||
				(Char_temp >= '0' && Char_temp <= '9')))
		{
			Buf_Lex[P_lex++] = Char_temp;
			Char_temp = getch();
		}
		if (P_line)	
			P_line--;

		string temp = string(Buf_Lex);
		if (temp != "end")//end ???Reg_Lex
			Reg_Lex = temp;

		if (KeyWordTable[string(Buf_Lex)] != "")
			return KeyWordTable[Buf_Lex];
		else
			return "SYM_IDENT";
	}
	else if (Buf_Lex[P_read] >= '0' && Buf_Lex[P_read] <= '9')//????
	{
		Char_temp = getch();
		while (P_lex < Buf_Lex_Size && (Char_temp >= '0' && Char_temp <= '9'))
		{
			Buf_Lex[P_lex++] = Char_temp;
			Char_temp = getch();
		}
		if (P_line)
			P_line--;

		Reg_Lex = string(Buf_Lex);
		return "SYM_NUM";
	}
	else if (Buf_Lex[P_read] == '"')//?????????
	{
		Char_temp = getch();

		Reg_Lex = take_string();
		return "SYM_STRING";
	}
	else if (KeyWordTable[string(Buf_Lex)] != "")
		return KeyWordTable[Buf_Lex];
	else
	{
		if (string(Buf_Lex) == "" && !File_End)
			return LA_getlexicon();
		else
		{
			Reg_Lex = string(Buf_Lex);
			return "";
		}
	}
}
