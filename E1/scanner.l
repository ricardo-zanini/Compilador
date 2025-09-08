%{
    #include "tokens.h"
%}

%option noyywrap
%option yylineno

%%

tipo                            { return TK_TIPO; }
var                             { return TK_VAR; }
senao                           { return TK_SENAO; }
decimal                         { return TK_DECIMAL; }
se                              { return TK_SE; }
inteiro                         { return TK_INTEIRO; }
retorna                         { return TK_RETORNA; }
enquanto                        { return TK_ENQUANTO; }
com                             { return TK_COM; }
[!\*\/%\+\-<>\[\]\(\)=,;\&\|]   { return yytext[0]; }
"<="                            { return TK_OC_LE; }
">="                            { return TK_OC_GE; }
"=="                            { return TK_OC_EQ; }
"!="                            { return TK_OC_NE; }
":="                            { return TK_ATRIB; }
"->"                            { return TK_SETA; }
[a-z_][a-z0-9_]*                { return TK_ID; }
[0-9]+                          { return TK_LI_INTEIRO; }
[0-9]*\.[0-9]+                  { return TK_LI_DECIMAL; }
"//".*[\n]*
\n
[ \t\r]+
.                               { return TK_ER; }
%%

int get_line_number(){
    return yylineno;
}