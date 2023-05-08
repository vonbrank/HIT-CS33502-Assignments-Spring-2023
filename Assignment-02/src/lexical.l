%{
#include "syntax.tab.h"
#include "node.h"
extern NodePtr root;
extern int printToken;
int yycolumn = 1;

#define YY_USER_ACTION \
    yylloc.first_line=yylloc.last_line=yylineno; \
    yylloc.first_column=yycolumn; \
    yylloc.last_column=yycolumn+yyleng-1; \
    yycolumn+=yyleng;

extern int flexError;
%}


%option yylineno

digit [0-9]
letter [_a-zA-Z]
white_space [ \t]
crlf [\r\n]
oct 0|0[1-9]+{digit}*
dec 0|[1-9]+{digit}*
hex 0[xX][0-9a-fA-F]+
int {oct}|{dec}|{hex}
float {digit}+"."{digit}+|{digit}*"."{digit}+[eE][+-]?{digit}+|{digit}+"."{digit}*[eE][+-]?{digit}+
id {letter}({letter}|{digit})*
semi ";"
comma ","
assignop "="
relop ">"|"<"|">="|"<="|"=="|"!="
plus "+"
minus "-"
star "*"
div "/"
and "&&"
or "||"
dot "."
not "!"
type "int"|"float"
lp "("
rp ")"
lb "["
rb "]"
lc "{"
rc "}"
struct "struct"
return "return"
if "if"
else "else"
while "while"
%%
{semi} { 
    if(printToken) printf("<SEMI>\n"); 
    yylval.node = newTokenNode("SEMI", yylineno, yytext);
    return SEMI;
}
{comma} { 
    if(printToken) printf("<COMMA>\n"); 
    yylval.node = newTokenNode("COMMA", yylineno, yytext);
    return COMMA;
}
{assignop} { 
    if(printToken) printf("<ASSIGNOP>\n"); 
    yylval.node = newTokenNode("ASSIGNOP", yylineno, yytext);
    return ASSIGNOP; 
}
{relop} { 
    if(printToken) printf("<RELOP, \"%s\">\n", yytext); 
    yylval.node = newTokenNode("RELOP", yylineno, yytext);
    return RELOP; 
}
{plus} { 
    if(printToken) printf("<PLUS>\n");
    yylval.node = newTokenNode("PLUS", yylineno, yytext);
    return PLUS;
}
{minus} {     
    if(printToken) printf("<MINUS>\n"); 
    yylval.node = newTokenNode("MINUS", yylineno, yytext);    
    return MINUS;
}
{star} { 
    if(printToken) printf("<STAR>\n");
    yylval.node = newTokenNode("STAR", yylineno, yytext);
    return STAR;
}
{div} { 
    if(printToken) printf("<DIV>\n");
    yylval.node = newTokenNode("DIV", yylineno, yytext);
    return DIV;
}
{and} { 
    if(printToken) printf("<AND>\n");
    yylval.node = newTokenNode("AND", yylineno, yytext); 
    return AND; 
}
{or} { 
    if(printToken) printf("<OR>\n"); 
    yylval.node = newTokenNode("OR", yylineno, yytext);
    return OR; 
}
{dot} { 
    if(printToken) printf("<DOT>\n"); 
    yylval.node = newTokenNode("DOT", yylineno, yytext);
    return DOT; 
}
{not} { 
    if(printToken) printf("<NOT>\n");
    yylval.node = newTokenNode("NOT", yylineno, yytext); 
    return NOT; 
}
{lp} { 
    if(printToken) printf("<LP>\n"); 
    yylval.node = newTokenNode("LP", yylineno, yytext);
    return LP; 
}
{rp} { 
    if(printToken) printf("<RP>\n"); 
    yylval.node = newTokenNode("RP", yylineno, yytext);
    return RP; 
}
{lb} { 
    if(printToken) printf("<LB>\n"); 
    yylval.node = newTokenNode("LB", yylineno, yytext);
    return LB; 
}
{rb} { 
    if(printToken) printf("<RB>\n"); 
    yylval.node = newTokenNode("RB", yylineno, yytext);
    return RB; 
}
{lc} { 
    if(printToken) printf("<LC>\n");
    yylval.node = newTokenNode("LC", yylineno, yytext); 
    return LC; 
}
{rc} { 
    if(printToken) printf("<RC>\n");
    yylval.node = newTokenNode("RC", yylineno, yytext);
    return RC; 
}
{struct} { 
    if(printToken) printf("<STRUCT>\n");
    yylval.node = newTokenNode("STRUCT", yylineno, yytext); 
    return STRUCT; 
}
{return} { 
    if(printToken) printf("<RETURN>\n"); 
    yylval.node = newTokenNode("RETURN", yylineno, yytext);
    return RETURN; 
}
{if} { 
    if(printToken) printf("<IF>\n"); 
    yylval.node = newTokenNode("IF", yylineno, yytext);
    return IF; 
}
{else} { 
    if(printToken) printf("<ELSE>\n"); 
    yylval.node = newTokenNode("ELSE", yylineno, yytext);
    return ELSE; 
}
{while} { 
    if(printToken) printf("<WHILE>\n"); 
    yylval.node = newTokenNode("WHILE", yylineno, yytext);
    return WHILE; 
}
{type} { 
    if(printToken) printf("<TYPE, \"%s\">\n", yytext); 
    yylval.node = newTokenNode("TYPE", yylineno, yytext);
    return TYPE;
}
{id} { 
    if(printToken) printf("<ID, \"%s\">\n", yytext);
    yylval.node = newTokenNode("ID", yylineno, yytext);
    return ID;
}
{int} { 
    if(printToken) printf("<INT, \"%s\">\n", yytext);
    yylval.node = newTokenNode("INT", yylineno, yytext);
    return INT;    
}
{float} { 
    if(printToken) printf("<FLOAT, \"%s\">\n", yytext); 
    yylval.node = newTokenNode("FLOAT", yylineno, yytext);
    return FLOAT;
    }
{white_space} { }
{crlf} { yycolumn = 1; }

{digit}{id} { flexError = 1; printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); }
. { flexError = 1; printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); }
%%