%{
    #include <stdio.h>
    int yylex(void);
    void yyerror (char const *mensagem);
    int get_line_number();
%}

%define parse.error verbose
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
%token TK_ID
%token TK_LI_INTEIRO
%token TK_LI_DECIMAL
%token TK_ER

%%

/* ==================================================================== */
/* ========================== INICIALIZAÇÃO =========================== */
/* ==================================================================== */

/* INICIALIZAÇÃO: Inicialização da gramática, pode ser lista, ou pode ser vazia */
programa
    : lista ';';
programa
    : %empty;

/* LISTA: Definição de "lista", que pode ser um único elemento ou uma lista de elementos separados por vírgula */
lista
    : elemento;
lista
    : lista ',' elemento;

/* ELEMENTO: Definição de "elemento", que pode ser uma função ou uma variável */
elemento
    : funcao
    | declaracao_variavel_no_ini;

/* ==================================================================== */
/* ============================= FUNCOES ============================== */
/* ==================================================================== */

/* FUNÇÃO: Definição de função, que possui um cabeçalho com zero ou mais parâmetros, seguido por um corpo */
funcao
    : TK_ID TK_SETA tipo lista_parametros_ini_com TK_ATRIB bloco_comandos;

/* CABEÇALHO DA FUNÇÃO: Pode ter zero ou mais parâmetros (O TOKEN TK_COM É OPCIONAL!!) */
lista_parametros_ini_com
    : TK_COM lista_parametros_ini
    | lista_parametros_ini;

lista_parametros_ini
    : lista_parametros
    | %empty;

lista_parametros
    : parametro;
lista_parametros
    : lista_parametros ',' parametro;

parametro
    : TK_ID TK_ATRIB tipo;

/* ==================================================================== */
/* ========================= COMANDOS SIMPLES ========================= */
/* ==================================================================== */

/* COMANDO SIMPLES: Um comando simples pode ser uma série de comandos diferentes, como listados */
comando_simples
    : bloco_comandos
    | declaracao_variavel
    | comando_atribuicao
    | chamada_funcao
    | comando_retorno
    | comando_controle_fluxo;

/* BLOCO DE COMANDOS: Um bloco de comandos fica em colchetes, e pode ter 0 ou mais comandos simples*/
bloco_comandos
    : '[' bloco_comandos_conteudo ']';

bloco_comandos_conteudo
    : %empty
    | bloco_comandos_conteudo_lista;

bloco_comandos_conteudo_lista
    : comando_simples
    | bloco_comandos_conteudo_lista comando_simples; 

/* COMANDO DE ATRIBUIÇÃO: Identificador, seguido de atribuição e expressão*/
comando_atribuicao
    : TK_ID TK_ATRIB expressao;

/* CHAMADA DE FUNÇÃO: Chamada de função pode ter 0 ou mais argumentos listados entre parênteses*/
chamada_funcao
    : TK_ID '(' lista_argumentos_ini ')';

lista_argumentos_ini
    : lista_argumentos
    | %empty;

lista_argumentos
    : expressao
    | lista_argumentos ',' expressao;

/* COMANDO DE RETORNO: Retorna uma expressão e seu tipo*/
comando_retorno
    : TK_RETORNA expressao TK_ATRIB tipo

/* COMANDO DE CONTROLE DE FLUXO: O controle de fluxo pode ser uma condição ou uma repetição, uma condição pode ou não ter o comando else */
comando_controle_fluxo
    : condicional
    | repeticao;

condicional
    : TK_SE '(' expressao ')' bloco_comandos condicional_else;

condicional_else
    : TK_SENAO bloco_comandos
    | %empty;

repeticao
    : TK_ENQUANTO '(' expressao ')' bloco_comandos;

/* ==================================================================== */
/* ============================ VARIÁVEIS ============================= */
/* ==================================================================== */

/* VARIÁVEL INI: Uma variavel que faz part da linguagem, não pode ter atribuição */
declaracao_variavel_no_ini
    : TK_VAR TK_ID TK_ATRIB tipo;

/* VARIÁVEL: Uma variavel que pode ou não ter uma atribuição*/
declaracao_variavel
    : TK_VAR TK_ID TK_ATRIB tipo atribuicao;

/* TIPO: "Tipo" pode ser ou um inteiro ou um decimal, como especificado na descrição da linguagem */
tipo
    : TK_INTEIRO
    | TK_DECIMAL;

/*  ATRIBUIÇÃO: Uma atribuição pode pode ser de um inteiro ou de um decimal, ou não existe e é apenas uma declaração */
atribuicao
    : conteudo_atribuicao
    | %empty;

conteudo_atribuicao
    : TK_COM tipo_valor;

tipo_valor
    : TK_LI_DECIMAL 
    | TK_LI_INTEIRO;

/* ==================================================================== */
/* ============================ EXPRESSÃO ============================= */
/* ==================================================================== */

/* EXPRESSÃO: Expressões possuem vários níveis de acordo com sua precedência, quanto maior o nível menor o grau de precedência. Podem haver operadores unários e binários */
expressao 
    : expressao '|' nivel_6
    | nivel_6;

nivel_6
    : nivel_6 '&' nivel_5
    | nivel_5;

nivel_5
    : nivel_5 TK_OC_EQ nivel_4
    | nivel_5 TK_OC_NE nivel_4
    | nivel_4;

nivel_4
    : nivel_4 '<' nivel_3
    | nivel_4 '>' nivel_3
    | nivel_4 TK_OC_LE nivel_3
    | nivel_4 TK_OC_GE nivel_3
    | nivel_3;

nivel_3
    : nivel_3 '+' nivel_2
    | nivel_3 '-' nivel_2
    | nivel_2;

nivel_2
    : nivel_2 '*' nivel_1
    | nivel_2 '/' nivel_1
    | nivel_2 '%' nivel_1
    | nivel_1;

nivel_1
    : '+' nivel_1
    | '-' nivel_1
    | '!' nivel_1
    | nivel_0;

nivel_0
    : chamada_funcao
    | TK_ID
    | TK_LI_INTEIRO
    | TK_LI_DECIMAL
    | '(' expressao ')';

%%

void yyerror (char const *mensagem) {
    printf("==================================================================\n");
    printf ("ERRO: Linha %i - [%s]\n", get_line_number(), mensagem);
    printf("==================================================================\n");
}