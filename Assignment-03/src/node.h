#ifndef PARSER_NODE_H
#define PARSER_NODE_H

typedef enum nodeType
{
    TOKEN,
    NOT_TOKEN,
} NodeType;

struct node;

typedef struct node
{
    char *name;

    int lineNumber;

    int sonNumber;
    struct node **sonList;

    char *valueText;

    NodeType nodeType;

} Node;

typedef Node *NodePtr;

extern NodePtr emptyNodePtrList[];

NodePtr newNode(char *name, int lineNumber, int sonNumber, NodePtr sonList[]);

NodePtr newTokenNode(char *name, int lineNumber, char *valueText);

void printTree(NodePtr nodePtr, int tabNumber);

#endif