/*¥ÌŒÛ¥¶¿Ì_Error Handler*/

#include "pass_first.h"

void Error_report(int Eno, string Info)
{
	string Etype;

	switch (Eno) {
	case 1:
		Etype = "Lexical";
		break;
	case 2:
		Etype = "Syntax Error!";
		break;
	case 3:
		Etype = "Semantic Error!";
		break;
	default:
		Etype = "Untyped Error!";
		break;
	}
	
	cout << "Line: " << Counter_Line;
	cout << " At: " << Counter_Lex << " ---- ";
	cout << Etype << endl;
	cout << Info << endl;
	
	exit(Eno);
}
