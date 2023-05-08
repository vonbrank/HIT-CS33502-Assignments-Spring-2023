%{
#include "node.h"
#include <stdio.h>

extern NodePtr root;
extern int yylex (void);
extern int yyerror(char *msg);
extern int bisonError;
%}

%union {
    NodePtr node;
}

%token <node> SEMI
%token <node> COMMA
%token <node> ASSIGNOP
%token <node> RELOP
%token <node> PLUS
%token <node> MINUS
%token <node> STAR
%token <node> DIV
%token <node> AND
%token <node> OR
%token <node> DOT
%token <node> NOT
%token <node> LP
%token <node> RP
%token <node> LB
%token <node> RB
%token <node> LC
%token <node> RC
%token <node> STRUCT
%token <node> RETURN
%token <node> IF
%token <node> ELSE
%token <node> WHILE
%token <node> ID
%token <node> INT
%token <node> FLOAT
%token <node> TYPE

/* High-level Definitions type */
%type <node> Program ExtDefList ExtDef ExtDecList

/* Specifiers type */
%type <node> Specifier StructSpecifier OptTag Tag

/* Declarators type */
%type <node> VarDec FunDec VarList ParamDec

/* Statements type */
%type <node> CompSt StmtList Stmt

/* Local Definitions type */
%type <node> DefList Def DecList Dec

/* Expressions type */
%type <node> Exp Args

%%

/* High-level Definitions */
Program : ExtDefList { { NodePtr sonList[1] = {$1}; root = $$ = newNode("Program", @$.first_line, 1, sonList); } }
ExtDefList : { $$ = NULL; }
    | ExtDef ExtDefList { { NodePtr sonList[2] = {$1, $2}; $$ = newNode("ExtDefList", @$.first_line, 2, sonList); } }
ExtDef : Specifier ExtDecList SEMI { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("ExtDef", @$.first_line, 3, sonList); } }
    | Specifier SEMI { { NodePtr sonList[2] = {$1, $2}; $$ = newNode("ExtDef", @$.first_line, 2, sonList); } }
    | Specifier FunDec CompSt { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("ExtDef", @$.first_line, 3, sonList); } }
    | error SEMI { bisonError = 1; }
ExtDecList : VarDec { { NodePtr sonList[1] = {$1}; $$ = newNode("ExtDecList", @$.first_line, 1, sonList); } }
    | VarDec COMMA ExtDecList { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("ExtDecList", @$.first_line, 3, sonList); } }

/* Specifiers */
Specifier : TYPE { { NodePtr sonList[1] = {$1}; $$ = newNode("Specifier", @$.first_line, 1, sonList); } }
    | StructSpecifier { { NodePtr sonList[1] = {$1}; $$ = newNode("Specifier", @$.first_line, 1, sonList); } }
StructSpecifier : STRUCT OptTag LC DefList RC { { NodePtr sonList[5] = {$1, $2, $3, $4, $5}; $$ = newNode("StructSpecifier", @$.first_line, 5, sonList); } }
    | STRUCT Tag { { NodePtr sonList[2] = {$1, $2}; $$ = newNode("StructSpecifier", @$.first_line, 2, sonList); } }
OptTag : { $$ = NULL; }
    | ID { { NodePtr sonList[1] = {$1}; $$ = newNode("OptTag", @$.first_line, 1, sonList); } }
Tag : ID { { NodePtr sonList[1] = {$1}; $$ = newNode("Tag", @$.first_line, 1, sonList); } }

/* Declarators */
VarDec : ID { { NodePtr sonList[1] = {$1}; $$ = newNode("VarDec", @$.first_line, 1, sonList); } }
    | VarDec LB INT RB { { NodePtr sonList[4] = {$1, $2, $3, $4}; $$ = newNode("VarDec", @$.first_line, 4, sonList); } }
FunDec : ID LP VarList RP { { NodePtr sonList[4] = {$1, $2, $3, $4}; $$ = newNode("FunDec", @$.first_line, 4, sonList); } }
    | ID LP RP { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("FunDec", @$.first_line, 3, sonList); } }
    | error RP { bisonError = 1; }
VarList : ParamDec COMMA VarList { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("VarList", @$.first_line, 3, sonList); } }
    | ParamDec { { NodePtr sonList[1] = {$1}; $$ = newNode("VarList", @$.first_line, 1, sonList); } }
ParamDec : Specifier VarDec { { NodePtr sonList[2] = {$1, $2}; $$ = newNode("ParamDec", @$.first_line, 2, sonList); } }

/* Statements */
CompSt: LC DefList StmtList RC { { NodePtr sonList[4] = {$1, $2, $3, $4}; $$ = newNode("CompSt", @$.first_line, 4, sonList); } }
    | error RC { bisonError = 1; }
StmtList : { $$ = NULL; }
    | Stmt StmtList { { NodePtr sonList[2] = {$1, $2}; $$ = newNode("StmtList", @$.first_line, 2, sonList); } }
Stmt : Exp SEMI { { NodePtr sonList[2] = {$1, $2}; $$ = newNode("Stmt", @$.first_line, 2, sonList); } }
    | CompSt { { NodePtr sonList[1] = {$1}; $$ = newNode("Stmt", @$.first_line, 1, sonList); } }
    | RETURN Exp SEMI { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Stmt", @$.first_line, 3, sonList); } }
    | IF LP Exp RP Stmt { { NodePtr sonList[5] = {$1, $2, $3, $4, $5}; $$ = newNode("Stmt", @$.first_line, 5, sonList); } }
    | IF LP Exp RP Stmt ELSE Stmt { { NodePtr sonList[7] = {$1, $2, $3, $4, $5, $6, $7}; $$ = newNode("Stmt", @$.first_line, 7, sonList); } }
    | WHILE LP Exp RP Stmt { { NodePtr sonList[5] = {$1, $2, $3, $4, $5}; $$ = newNode("Stmt", @$.first_line, 5, sonList); } }
    | error SEMI { bisonError = 1; }

/* Local Definitions */
DefList : { $$ = NULL; }
    | Def DefList { { NodePtr sonList[2] = {$1, $2}; $$ = newNode("DefList", @$.first_line, 2, sonList); } }
Def : Specifier DecList SEMI { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Def", @$.first_line, 3, sonList); } }
    | error SEMI { bisonError = 1; }
DecList: Dec { { NodePtr sonList[1] = {$1}; $$ = newNode("DecList", @$.first_line, 1, sonList); } }
    | Dec COMMA DecList { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("DecList", @$.first_line, 3, sonList); } }
Dec : VarDec { { NodePtr sonList[1] = {$1}; $$ = newNode("Dec", @$.first_line, 1, sonList); } }
    | VarDec ASSIGNOP Exp { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Dec", @$.first_line, 3, sonList); } }

/* Expressions */
Exp : Exp ASSIGNOP Exp { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Exp", @$.first_line, 3, sonList); } }
    | Exp AND Exp { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Exp", @$.first_line, 3, sonList); } }
    | Exp OR Exp { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Exp", @$.first_line, 3, sonList); } }
    | Exp RELOP Exp { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Exp", @$.first_line, 3, sonList); } }
    | Exp PLUS Exp { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Exp", @$.first_line, 3, sonList); } }
    | Exp MINUS Exp { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Exp", @$.first_line, 3, sonList); } }
    | Exp STAR Exp { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Exp", @$.first_line, 3, sonList); } }
    | Exp DIV Exp { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Exp", @$.first_line, 3, sonList); } }
    | LP Exp RP { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Exp", @$.first_line, 3, sonList); } }
    | MINUS Exp { { NodePtr sonList[2] = {$1, $2}; $$ = newNode("Exp", @$.first_line, 2, sonList); } }
    | NOT Exp { { NodePtr sonList[2] = {$1, $2}; $$ = newNode("Exp", @$.first_line, 2, sonList); } }
    | ID LP Args RP { { NodePtr sonList[4] = {$1, $2, $3, $4}; $$ = newNode("Exp", @$.first_line, 4, sonList); } }
    | ID LP RP { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Exp", @$.first_line, 3, sonList); } }
    | Exp LB Exp RB { { NodePtr sonList[4] = {$1, $2, $3, $4}; $$ = newNode("Exp", @$.first_line, 4, sonList); } }
    | Exp DOT ID { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Exp", @$.first_line, 3, sonList); } }
    | ID { { NodePtr sonList[1] = {$1}; $$ = newNode("Exp", @$.first_line, 1, sonList); } }
    | INT { { NodePtr sonList[1] = {$1}; $$ = newNode("Exp", @$.first_line, 1, sonList); } }
    | FLOAT { { NodePtr sonList[1] = {$1}; $$ = newNode("Exp", @$.first_line, 1, sonList); } }
Args : Exp COMMA Args { { NodePtr sonList[3] = {$1, $2, $3}; $$ = newNode("Args", @$.first_line, 3, sonList); } }
    | Exp { { NodePtr sonList[1] = {$1}; $$ = newNode("Args", @$.first_line, 1, sonList); } }
%%