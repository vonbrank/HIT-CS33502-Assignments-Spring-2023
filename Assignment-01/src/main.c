#include <stdio.h>
#include "node.h"
#include "lex.yy.c"

#define MAX_LINE_NUMBER 8192

NodePtr root;
int printToken = 0;

extern int yylineno;

int flexError = 0;
int bisonError = 0;

int main(int argc, char **argv)
{
    // if (argc > 1)
    // {
    //     if (!(yyin = fopen(argv[1], "r")))
    //     {
    //         perror(argv[1]);
    //         return 1;
    //     }
    // }
    root = NULL;
    yyparse();

    if (flexError == 0 && bisonError == 0)
    {
        printTree(root, 0);
    }
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