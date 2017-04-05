/*������_Main*/

#include "../lib/pass_first.h"
#include "../lib/pass_second.h"
#include "../lib/yui_tools.h"

void Init_all()
{
	LexiconAnalysis_init();
	SemanticAnalysis_init();

	Sym_line *temp_line = new Sym_line("main", "proc");
	Table0 = new Sym_table(NULL, temp_line);
	temp_line->sub_table = Table0;
}

int main()
{
	Init_all();
	cout << "..............................................................Let's begin!" << endl;

	SA_process(Table0);
	cout << "............................PASS ONE: Syntax & Semantic Analysis Finished." << endl;
	
	Opt_process();
	//Table0->Sym_print();//For debug
	SeA_print();
	SeA_print_toFile();
	cout << ".............................PASS TWO: Preprocess & Optimization Finished." << endl;

	Obj_generate();
	obj_print();
	obj_print_toFile();
	cout << ".........................................PASS THREE: Object Code Finished." << endl;
	
	return 0;
}
