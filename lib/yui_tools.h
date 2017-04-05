#pragma once

#include <iostream>
#include <String>
using namespace std;

string yui_itos(int num);
void yui_split_condit(string condit, string *result);

void set_or(bool *S1, bool *S2, bool* R, int Size);
void set_sub(bool *S1, bool *S2, bool* R, int Size);
bool set_isEqual(bool *S1, bool *S2, int Size);
void set_copy(bool *S_new, bool *S_origin, int Size);
