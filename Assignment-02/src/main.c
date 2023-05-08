#include <stdio.h>
#include "node.h"
#include "parser.h"
#include "analyser.c"
#include "utils/string.h"

int printAnalysingLog = 0;
int printParsingTree = 0;

extern NodePtr root;
extern int flexError;
extern int bisonError;

int main(int argc, char **argv)
{
    parse();

    if (flexError == 0 && bisonError == 0 && printParsingTree)
    {
        printTree(root, 0);
    }

    analyse(root);

    return 0;
}