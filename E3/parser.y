/*
* ============ GRUPO R ==================
* Bernardo Cobalchini Zietolie - 00550164
* Ricardo Zanini de Costa - 00344523
 */

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #include "asd.h"    /* Cabeçalho para a construção da Árvore de Sintaxe Abstrata (AST) */

    int yylex(void);
    void yyerror (char const *mensagem);
    int get_line_number();

    extern asd_tree_t *arvore; /* Variável externa (de "main.c") que serve como ponteiro para a raiz da AST a ser construída */
%}

%code requires {
    /* Estrutura para armazenar o valor léxico dos tokens */
    typedef struct valor_lexico_struct {
        int num_linha;
        int tipo_token;
        char* valor_token;
    } ValorLexico;
}

/* Diretiva que configura a variável global yylval. */
%union {
    struct valor_lexico_struct* valor_lexico;
    struct asd_tree* ast_node;
}

%define parse.error verbose

/* Somente tokens identificadores e literais devem possuir um valor léxico a ser empregado na AST. */
%token <valor_lexico> TK_ID
%token <valor_lexico> TK_LI_INTEIRO
%token <valor_lexico> TK_LI_DECIMAL

/* Tokens que não carregam valor */
%token TK_TIPO
%token TK_VAR
%token TK_SENAO
%token TK_DECIMAL
%token TK_SE
%token TK_INTEIRO
%token TK_ATRIB
%token TK_RETORNA
%token TK_SETA
%token TK_ENQUANTO
%token TK_COM
%token TK_OC_LE
%token TK_OC_GE
%token TK_OC_EQ
%token TK_OC_NE
%token TK_ER

/* Define o tipo de retorno para as regras da gramática como um nó da AST */
%type <ast_node> programa lista elemento
%type <ast_node> funcao lista_parametros_ini lista_parametros parametro
%type <ast_node> comando_simples bloco_comandos bloco_comandos_conteudo bloco_comandos_conteudo_lista
%type <ast_node> comando_atribuicao chamada_funcao lista_argumentos_ini lista_argumentos comando_retorno
%type <ast_node> comando_controle_fluxo condicional condicional_else repeticao
%type <ast_node> declaracao_variavel_no_ini declaracao_variavel tipo tipo_valor
%type <ast_node> expressao nivel_6 nivel_5 nivel_4 nivel_3 nivel_2 nivel_1 nivel_0

%%

/* ==================================================================== */
/* ========================== INICIALIZAÇÃO =========================== */
/* ==================================================================== */

/* INICIALIZAÇÃO: Inicialização da gramática, pode ser lista, ou pode ser vazia */
programa
    : lista ';' { arvore = $1; }
    | %empty   { arvore = NULL; }
;

/* LISTA: Definição de "lista", que pode ser um único elemento ou uma lista de elementos separados por vírgula */
lista
    : elemento              { $$ = $1; }
    | elemento ',' lista    { asd_add_child($1, $3); $$ = $1; }
;

/* ELEMENTO: Definição de "elemento", que pode ser uma função ou uma variável */
elemento
    : funcao                        { $$ = $1; }
    | declaracao_variavel_no_ini    { $$ = $1; }
;

/* ==================================================================== */
/* ============================= FUNCOES ============================== */
/* ==================================================================== */

/* FUNÇÃO: Definição de função, que possui um cabeçalho com zero ou mais parâmetros, seguido por um corpo */
funcao
    : TK_ID TK_SETA tipo lista_parametros_ini TK_ATRIB bloco_comandos
    { 
        $$ = asd_new($1->valor_token);
        free($1->valor_token);
        free($1);

        asd_free($3);
        if ($4) asd_free($4);
        if ($6) asd_add_child($$, $6);
    }
;

/* CABEÇALHO DA FUNÇÃO: Pode ter zero ou mais parâmetros (O TOKEN TK_COM É OPCIONAL!!) */

lista_parametros_ini
    : TK_COM lista_parametros   { $$ = $2; }
    | lista_parametros          { $$ = $1; }
    | %empty                    { $$ = NULL; }
;

lista_parametros
    : parametro                         { $$ = $1; }
    | parametro ',' lista_parametros    { asd_add_child($1, $3); $$ = $1; }
;

parametro
    : TK_ID TK_ATRIB tipo
    {
        $$ = asd_new("param");

        asd_add_child($$, asd_new($1->valor_token));
        free($1->valor_token);
        free($1);

        asd_free($3);
    }
;

/* ==================================================================== */
/* ========================= COMANDOS SIMPLES ========================= */
/* ==================================================================== */

/* COMANDO SIMPLES: Um comando simples pode ser uma série de comandos diferentes, como listados */
comando_simples
    : bloco_comandos            { $$ = $1 ? $1 : NULL; }
    | declaracao_variavel       { $$ = $1; }
    | comando_atribuicao        { $$ = $1; }
    | chamada_funcao            { $$ = $1; }
    | comando_retorno           { $$ = $1; }
    | comando_controle_fluxo    { $$ = $1; }
;

/* BLOCO DE COMANDOS: Um bloco de comandos fica em colchetes, e pode ter 0 ou mais comandos simples*/
bloco_comandos
    : '[' bloco_comandos_conteudo ']'   { $$ = $2 ? $2 : NULL; }
;

bloco_comandos_conteudo
    : %empty                            { $$ = NULL; }
    | bloco_comandos_conteudo_lista     { $$ = $1; }
;

bloco_comandos_conteudo_lista
    : comando_simples                                   { $$ = $1; }
    | comando_simples bloco_comandos_conteudo_lista     { asd_add_child($1, $2); $$ = $1; }
; 

/* COMANDO DE ATRIBUIÇÃO: Identificador, seguido de atribuição e expressão*/
comando_atribuicao
    : TK_ID TK_ATRIB expressao
    {
        $$ = asd_new(":=");

        asd_add_child($$, asd_new($1->valor_token));
        free($1->valor_token);
        free($1);

        asd_add_child($$, $3);
    }
;

/* CHAMADA DE FUNÇÃO: Chamada de função pode ter 0 ou mais argumentos listados entre parênteses*/
chamada_funcao
    : TK_ID '(' lista_argumentos_ini ')'
    {
        char* label = malloc(strlen("call ") + strlen($1->valor_token) + 1);

        sprintf(label, "call %s", $1->valor_token);
        free($1->valor_token);
        free($1);

        $$ = asd_new(label);
        free(label);

        if ($3) asd_add_child($$, $3);
    }
;

lista_argumentos_ini
    : lista_argumentos { $$ = $1; }
    | %empty           { $$ = NULL; }
;

lista_argumentos
    : expressao                      { $$ = $1; }
    | expressao ',' lista_argumentos { asd_add_child($1, $3); $$ = $1; }
;

/* COMANDO DE RETORNO: Retorna uma expressão e seu tipo*/
comando_retorno
    : TK_RETORNA expressao TK_ATRIB tipo
    {
        $$ = asd_new("retorna");
        asd_add_child($$, $2);
        asd_free($4);
    }
;

/* COMANDO DE CONTROLE DE FLUXO: O controle de fluxo pode ser uma condição ou uma repetição, uma condição pode ou não ter o comando else */
comando_controle_fluxo
    : condicional { $$ = $1; }
    | repeticao   { $$ = $1; }
;

condicional
    : TK_SE '(' expressao ')' bloco_comandos condicional_else
    {
        $$ = asd_new("se");
        asd_add_child($$, $3);
        asd_add_child($$, $5);
        if ($6) asd_add_child($$, $6);
    }
;

condicional_else
    : TK_SENAO bloco_comandos { $$ = $2; }
    | %empty                  { $$ = NULL; }
;

repeticao
    : TK_ENQUANTO '(' expressao ')' bloco_comandos
    {
        $$ = asd_new("enquanto");
        asd_add_child($$, $3);
        asd_add_child($$, $5);
    }
;

/* ==================================================================== */
/* ============================ VARIÁVEIS ============================= */
/* ==================================================================== */

/* VARIÁVEL INI: Uma variavel que faz parte da linguagem, não pode ter atribuição */
declaracao_variavel_no_ini
    : TK_VAR TK_ID TK_ATRIB tipo
    {
        $$ = asd_new("var");

        asd_add_child($$, asd_new($2->valor_token));
        free($2->valor_token);
        free($2);

        asd_free($4);
    }
;

/* VARIÁVEL: Uma variavel que pode ou não ter uma atribuição*/
declaracao_variavel
    : TK_VAR TK_ID TK_ATRIB tipo TK_COM tipo_valor /* Declaração de variável com inicialização */
    {
        $$ = asd_new("com");

        asd_add_child($$, asd_new($2->valor_token));
        free($2->valor_token);
        free($2);

        asd_free($4);                      
        asd_add_child($$, $6);                      
    }
    | TK_VAR TK_ID TK_ATRIB tipo /* Declaração de variável sem inicialização */
    {
        $$ = asd_new("var");
        
        asd_add_child($$, asd_new($2->valor_token));
        free($2->valor_token);
        free($2);

        asd_free($4);                   
    }
;

/* TIPO: "Tipo" pode ser ou um inteiro ou um decimal, como especificado na descrição da linguagem */
tipo
    : TK_INTEIRO    { $$ = asd_new("inteiro"); }
    | TK_DECIMAL    { $$ = asd_new("decimal"); }
;

tipo_valor
    : TK_LI_DECIMAL
    {
        $$ = asd_new($1->valor_token);
        free($1->valor_token);
        free($1);
    }
    | TK_LI_INTEIRO
    {
        $$ = asd_new($1->valor_token);
        free($1->valor_token);
        free($1);
    }
;

/* ==================================================================== */
/* ============================ EXPRESSÃO ============================= */
/* ==================================================================== */

/* EXPRESSÃO: Expressões possuem vários níveis de acordo com sua precedência, quanto maior o nível menor o grau de precedência. Podem haver operadores unários e binários */
expressao 
    : expressao '|' nivel_6 { $$ = asd_new("|"); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_6               { $$ = $1; }
;

nivel_6
    : nivel_6 '&' nivel_5   { $$ = asd_new("&"); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_5               { $$ = $1; }
;

nivel_5
    : nivel_5 TK_OC_EQ nivel_4 { $$ = asd_new("=="); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_5 TK_OC_NE nivel_4 { $$ = asd_new("!="); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_4                  { $$ = $1; }
;

nivel_4
    : nivel_4 '<' nivel_3      { $$ = asd_new("<"); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_4 '>' nivel_3      { $$ = asd_new(">"); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_4 TK_OC_LE nivel_3 { $$ = asd_new("<="); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_4 TK_OC_GE nivel_3 { $$ = asd_new(">="); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_3                  { $$ = $1; }
;

nivel_3
    : nivel_3 '+' nivel_2   { $$ = asd_new("+"); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_3 '-' nivel_2   { $$ = asd_new("-"); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_2               { $$ = $1; }
;

nivel_2
    : nivel_2 '*' nivel_1   { $$ = asd_new("*"); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_2 '/' nivel_1   { $$ = asd_new("/"); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_2 '%' nivel_1   { $$ = asd_new("%"); asd_add_child($$, $1); asd_add_child($$, $3); }
    | nivel_1               { $$ = $1; }
;

nivel_1
    : '+' nivel_1 { $$ = asd_new("+"); asd_add_child($$, $2); }
    | '-' nivel_1 { $$ = asd_new("-"); asd_add_child($$, $2); }
    | '!' nivel_1 { $$ = asd_new("!"); asd_add_child($$, $2); }
    | nivel_0     { $$ = $1; }
;

nivel_0
    : chamada_funcao  { $$ = $1; }
    | TK_ID
    {
        $$ = asd_new($1->valor_token);
        free($1->valor_token);
        free($1);
    }
    | TK_LI_INTEIRO
    {
        $$ = asd_new($1->valor_token);
        free($1->valor_token);
        free($1);
    }
    | TK_LI_DECIMAL
    {
        $$ = asd_new($1->valor_token);
        free($1->valor_token);
        free($1);
    }
    | '(' expressao ')' { $$ = $2; }
;

%%

void yyerror (char const *mensagem) {
    printf("==================================================================\n");
    printf ("ERRO: Linha %i - [%s]\n", get_line_number(), mensagem);
    printf("==================================================================\n");
}