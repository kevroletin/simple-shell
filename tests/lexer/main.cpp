#include "../../lexer.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
    std::ifstream fin;
    fin.open(argv[1]);
    CVarTable table;
    CLexer lex(fin, table);
    IToken* tok = NULL;
    do {
        bool b;
        tok = lex.GetToken(b);
        if (tok != NULL ) std::cerr << *tok << "\n";
        else std::cerr << "[NULL]";
    } while (tok != NULL);
    return 0;
}
