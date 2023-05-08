#include <stdio.h>
#include "node.h"
#include "parser.h"
#include "translator.h"
#include "utils/string.h"

int printTranslatingLog = 0;
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

    translate(root);

    return 0;
}