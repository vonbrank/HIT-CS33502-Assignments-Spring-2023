#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "node.h"

NodePtr emptyNodePtrList[0];

NodePtr newNode(char *name, int lineNumber, int sonNumber, NodePtr sonList[])
{
    NodePtr nodePtr = (NodePtr)malloc(sizeof(Node));

    nodePtr->name = (char *)malloc(sizeof(strlen(name) + 1));
    strcpy(nodePtr->name, name);

    nodePtr->lineNumber = lineNumber;

    // printf("fa = %s, son = ", name);

    nodePtr->sonNumber = sonNumber;
    nodePtr->sonList = malloc(sizeof(NodePtr) * sonNumber);
    for (int i = 0; i < sonNumber; i++)
    {
        nodePtr->sonList[i] = sonList[i];
        // if(sonList[i] != NULL) {
        //     printf("%s ", sonList[i]->name);
        // } else {
        //     printf("NULL");
        // }
    }
    // printf("\n");

    nodePtr->valueText = "\0";

    nodePtr->nodeType = NOT_TOKEN;

    return nodePtr;
}

NodePtr newTokenNode(char *name, int lineNumber, char *valueText)
{
    NodePtr nodePtr = (NodePtr)malloc(sizeof(Node));

    nodePtr->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(nodePtr->name, name);

    nodePtr->lineNumber = lineNumber;

    nodePtr->sonNumber = 0;
    nodePtr->sonList = emptyNodePtrList;

    nodePtr->valueText = (char *)malloc(sizeof(char) * (strlen(valueText) + 1));
    strcpy(nodePtr->valueText, valueText);

    nodePtr->nodeType = TOKEN;

    return nodePtr;
}

void printTree(NodePtr nodePtr, int tabNumber)
{
    if (nodePtr == NULL)
        return;
    for (int i = 0; i < tabNumber; i++)
    {
        printf("%s", "  ");
    }

    if (nodePtr->nodeType == TOKEN)
    {
        if (strcmp(nodePtr->name, "TYPE") == 0)
        {
            printf("TYPE: %s\n", nodePtr->valueText);
        }
        else if (strcmp(nodePtr->name, "ID") == 0)
        {
            printf("ID: %s\n", nodePtr->valueText);
        }
        else if (strcmp(nodePtr->name, "INT") == 0)
        {
            printf("INT: %d\n", atoi(nodePtr->valueText));
        }
        else if (strcmp(nodePtr->name, "FLOAT") == 0)
        {
            printf("FLOAT: %f\n", (float)(atof(nodePtr->valueText)));
        }
        else
        {
            printf("%s\n", nodePtr->name);
        }
    }
    else
    {
        printf("%s (%d)\n", nodePtr->name, nodePtr->lineNumber);
    }

    for (int i = 0; i < nodePtr->sonNumber; i++)
    // for (int i = nodePtr->sonNumber - 1; i >= 0; i--)
    {
        printTree(nodePtr->sonList[i], tabNumber + 1);
    }
}