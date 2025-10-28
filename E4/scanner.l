/*
* ============ GRUPO R ==================
* Bernardo Cobalchini Zietolie - 00550164
* Ricardo Zanini de Costa - 00344523
 */

%{
    #include "parser.tab.h"
    #include <string.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include "erros.h"
    #include "tipos.h"

    /* Função auxiliar para a criação de ValorLexico, retornando o tipo do token */
    int criar_valor_lexico(int tipo_token) {
        yylval.valor_lexico = (ValorLexico*) malloc(sizeof(ValorLexico));
        yylval.valor_lexico->num_linha = yylineno;              /* Número da linha atual */
        yylval.valor_lexico->tipo_token = tipo_token;           /* Tipo do token */
        yylval.valor_lexico->valor_token = strdup(yytext);      /* Valor (o próprio lexema) */
        return tipo_token;
    }
%}

%option noyywrap
%option yylineno
%option noinput nounput

%%

tipo                            { return TK_TIPO; }         /* Ao encontrar a palavra "tipo" retorna TK_TIPO */
var                             { return TK_VAR; }          /* Ao encontar a plavra "var" retorna TK_VAR */
senao                           { return TK_SENAO; }        /* Ao encontrar a palavra "senao" retorna TK_SENAO */
decimal                         { return TK_DECIMAL; }      /* Ao encontrar a palavra "decimal" retorna TK_DECIMAL */
se                              { return TK_SE; }           /* Ao encontrar a palavra "se" retorna TK_SE */
inteiro                         { return TK_INTEIRO; }      /* Ao encontrar a palavra "inteiro" retorna TK_INTEIRO */
retorna                         { return TK_RETORNA; }      /* Ao encontrar a palavra "retorna" retorna TK_RETORNA */
enquanto                        { return TK_ENQUANTO; }     /* Ao encontrar a palavra "enquanto" retorna TK_ENQUANTO */
com                             { return TK_COM; }          /* Ao encontrar a palavra "com" retorna TK_COM */
[!\*\/%\+\-<>\[\]\(\)=,;\&\|]   { return yytext[0]; }       /* Identifica elementos de um certo conjunto de caracteres presentes na linguagem */
"<="                            { return TK_OC_LE; }        /* Ao encontrar o operador relacional "<=" (menor ou igual) retorna TK_OC_LE */
">="                            { return TK_OC_GE; }        /* Ao encontrar o operador relacional ">=" (maior ou igual) retorna TK_OC_GE */
"=="                            { return TK_OC_EQ; }        /* Ao encontrar o operador relacional "==" (igualdade) retorna TK_OC_EQ */
"!="                            { return TK_OC_NE; }        /* Ao encontrar o operador relacional "!=" (diferente) retorna TK_OC_NE */
":="                            { return TK_ATRIB; }        /* Ao encontrar o operador de atribuição ":=" retorna TK_ATRIB */
"->"                            { return TK_SETA; }         /* Ao encontrar o operador "->" retorna TK_SETA */

[a-z_][a-z0-9_]*                {   /* Identificadores que podem conter somente letras minúsculas, números e underlines, mas que não podem começar com números */
                                    return criar_valor_lexico(TK_ID);
                                }

[0-9]+                          {   /* Identifica números inteiros positivos */
                                    return criar_valor_lexico(TK_LI_INTEIRO);
                                }

[0-9]*\.[0-9]+                  {   /* Identifica números com casas decimais, e com dígitos opcionais antes do "." */
                                    return criar_valor_lexico(TK_LI_DECIMAL);
                                }

"//".*[\n]*                                                 /* Ignora-se comentários de uma linha que começam com "//" e vão até o final da linha */
\n                                                          /* Ignora-se quebras de linha */
[ \t\r]+                                                    /* Ignora-se outros caracteres nulos */
.                               { return TK_ER; }           /* Qualquer outro caractere que não seja contemplado nas regras acima é um erro, assim retornando TK_ERRO */
%%

/* Retorna o número da linha atual, usado em caso de erro sintático */
int get_line_number(){
    return yylineno;
}