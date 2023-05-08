#include <stdio.h>
#include "node.h"
#include "lex.yy.c"

#define MAX_LINE_NUMBER 8192

NodePtr root;
int printToken = 0;

extern int yylineno;

int flexError = 0;
int bisonError = 0;

int parse()
{
    root = NULL;
    yyparse();
    return 0;
}

int yyerror(char *msg)
{
    static int currentLineno = -1;

    if (currentLineno != yylineno)
    {
        currentLineno = yylineno;
    }
    else
    {
        return 0;
    }

    printf("Error type B at line %d: %s.\n", yylineno, msg);
}