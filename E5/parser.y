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
    #include "tipos.h"
    #include "erros.h"
    #include "tabelaSimbolos.h"
    #include "semantica.h"
    #include "iloc.h"

    /* Declarações de funções que serão utilizadas nas ações semânticas e sintáticas. */
    int yylex(void);
    void yyerror (char const *mensagem);
    int get_line_number();

    extern asd_tree_t *arvore; /* Variável externa (de "main.c") que serve como ponteiro para a raiz da AST a ser construída */
%}

/* Seção que define código que é necessário tanto no parser quanto no scanner */
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
    struct valor_lexico_struct* valor_lexico; /* Para tokens que carregam dados do scanner */
    struct asd_tree* ast_node; /* Para as regras da gramática que constroem e retornam nós da AST */
    int tipo_dado; /* Para a regra de tipos (é um TipoDados) */
    struct arg_lista* arg_list; /* Para a lista de parâmetros/argumentos de funções */
}

/* Informa ao Bison como liberar as estruturas utilizadas quando são descartadas*/
%destructor { free_token($$); } <valor_lexico> /* Liberar os tokens quando eles são descartados */
%destructor { if ($$) asd_free($$); } <ast_node> /* Liberar nós da AST que são descartados durante a recuperação de erro */
%destructor { free_arg_list($$); } <arg_list> /* Liberar as listas de parâmetros/argumentos quando são descartadas */

/* Habilita mensagens de erro mais detalhadas. */
%define parse.error verbose

/* ==================================================================== */
/* ======================== DECLARAÇÃO DE TOKENS ====================== */
/* ==================================================================== */

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

/* Define o tipo de retorno para as regras da gramática */
%type <ast_node> programa lista elemento funcao comando_simples bloco_comandos bloco_comandos_funcao bloco_comandos_conteudo bloco_comandos_conteudo_lista
%type <ast_node> comando_atribuicao chamada_funcao lista_argumentos_ini lista_argumentos comando_retorno
%type <ast_node> comando_controle_fluxo condicional condicional_else repeticao
%type <ast_node> declaracao_variavel_no_ini declaracao_variavel atribuicao tipo_valor
%type <ast_node> expressao nivel_6 nivel_5 nivel_4 nivel_3 nivel_2 nivel_1 nivel_0
%type <tipo_dado> tipo
%type <arg_list> lista_parametros_ini lista_parametros parametro

/* Faz o Bison começar pela regra "wrapper" (permitindo iniciar o gerenciamento da pilha e de escopo) */
%start programa_wrapper

%%

/* ==================================================================== */
/* ========================== REGRAS INICIAIS ========================= */
/* ==================================================================== */

/* Regra 'wrapper' gerencia o escopo global. Executada uma única vez. */
programa_wrapper
    : { semantica_push_scope(); } /* Cria o escopo global */
      programa /* Executa o parsing do programa */
      { (void)$2; semantica_pop_scope(); } /* Destrói o escopo global */
;

/* INICIALIZAÇÃO: Inicialização da gramática, pode ser lista, ou pode ser vazia */
/* Ao final, a variável global 'arvore' aponta para a raiz da AST construída */
programa
    : lista ';' { arvore = $1; $$ = $1; }
    | %empty    { arvore = NULL; $$ = NULL; }
;

/* LISTA: Definição de "lista", que pode ser um único elemento ou uma lista de elementos separados por vírgula */
lista
    : elemento              { $$ = $1; }
    | elemento ',' lista    { if ($1) { if ($3) asd_add_child($1, $3, ORD_NORM); $$ = $1; } else { $$ = $3; } }
    /* Se o elemento atual não for nulo, adiciona o resto da lista como filho. Se for, a lista começa a partir do próximo */
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
/* A ação semântica cria um nó na AST com o nome da função. O bloco de comandos é adicionado como filho caso não seja vazio. */
funcao
    : TK_ID TK_SETA tipo                { semantica_funcao_declaracao($1, $3); }
        lista_parametros_ini            { semantica_funcao_atualizar_args($5); }
        TK_ATRIB bloco_comandos_funcao  { $$ = semantica_funcao_definicao($1, $3, $8); }
;

/* CABEÇALHO DA FUNÇÃO: Pode ter zero ou mais parâmetros (O TOKEN TK_COM É OPCIONAL!!) */
/* Validam a sintaxe, mas retornam NULL pois os parâmetros não entram na AST. */
lista_parametros_ini
    : TK_COM lista_parametros   { $$ = $2; }
    | lista_parametros          { $$ = $1; }
    | %empty                    { $$ = NULL; }
;

lista_parametros
    : parametro                         { $$ = $1; }
    | parametro ',' lista_parametros    { $1->next = $3; $$ = $1; }
;

parametro
    : TK_ID TK_ATRIB tipo { $$ = semantica_param($1, $3); }
;

/* BLOCO DE COMANDOS DE FUNÇÃO: mesma semântica do bloco de comandos normal, mas sem push/pop */
bloco_comandos_funcao
    : '[' bloco_comandos_conteudo ']' { $$ = $2; }

/* ==================================================================== */
/* ========================= COMANDOS SIMPLES ========================= */
/* ==================================================================== */

/* COMANDO SIMPLES: Um comando simples pode ser uma série de comandos diferentes, como listados */
comando_simples
    : bloco_comandos            { $$ = $1; }
    | declaracao_variavel       { $$ = $1; }
    | comando_atribuicao        { $$ = $1; }
    | chamada_funcao            { $$ = $1; }
    | comando_retorno           { $$ = $1; }
    | comando_controle_fluxo    { $$ = $1; }
;

/* BLOCO DE COMANDOS: Um bloco de comandos fica em colchetes, e pode ter 0 ou mais comandos simples*/
bloco_comandos
    : '[' { semantica_push_scope(); } bloco_comandos_conteudo ']' { $$ = $3; semantica_pop_scope(); }
;

bloco_comandos_conteudo
    : %empty                            { $$ = NULL; }
    | bloco_comandos_conteudo_lista     { $$ = $1; }
;

bloco_comandos_conteudo_lista
    : comando_simples                               { $$ = $1; }
    | comando_simples bloco_comandos_conteudo_lista { if ($1) { if ($2) asd_add_child($1, $2, ORD_NORM); $$ = $1; } else { $$ = $2; } }
    /* Se o comando atual não for nulo, adiciona o resto da lista como filho. Se for, a lista começa a partir do próximo */
; 

/* COMANDO DE ATRIBUIÇÃO: Identificador, seguido de atribuição e expressão*/
comando_atribuicao
    : TK_ID TK_ATRIB expressao { $$ = semantica_comando_atrib($1, $3); }
;

/* CHAMADA DE FUNÇÃO: Chamada de função pode ter 0 ou mais argumentos listados entre parênteses*/
chamada_funcao
    : TK_ID '(' lista_argumentos_ini ')' { $$ = semantica_chamada_func($1, $3); }
;

lista_argumentos_ini
    : lista_argumentos { $$ = $1; }
    | %empty           { $$ = NULL; }
;

lista_argumentos
    : expressao                         { $$ = asd_new("arg_list_temp", TIPO_INDEF, $1->num_linha, NULL, NULL); asd_add_child($$, $1, ORD_NORM); } /* Cria um novo nó 'arg_list' temporário */
    | lista_argumentos ',' expressao    { asd_add_child($1, $3, ORD_NORM); $$ = $1; } /* Adiciona a nova expressão como filha do 'arg_list' existente */
;

/* COMANDO DE RETORNO: Retorna uma expressão e seu tipo*/
comando_retorno
    : TK_RETORNA expressao TK_ATRIB tipo { $$ = semantica_comando_ret($2, $4); }
;

/* COMANDO DE CONTROLE DE FLUXO: O controle de fluxo pode ser uma condição ou uma repetição, uma condição pode ou não ter o comando else */
comando_controle_fluxo
    : condicional { $$ = $1; }
    | repeticao   { $$ = $1; }
;

condicional
    : TK_SE '(' expressao ')' bloco_comandos condicional_else { $$ = semantica_condicional($3, $5, $6); }
;

condicional_else
    : TK_SENAO bloco_comandos { $$ = $2; }
    | %empty                  { $$ = NULL; }
;

repeticao
    : TK_ENQUANTO '(' expressao ')' bloco_comandos { $$ = semantica_enquanto($3, $5); } /* label é o lexema de TK_ENQUANTO */
;

/* ==================================================================== */
/* ============================ VARIÁVEIS ============================= */
/* ==================================================================== */

/* VARIÁVEL INI: Uma variavel que faz parte da linguagem, não pode ter atribuição */
/* Retorna NULL, pois não gera nó na AST, e libera a memória do token do identificador */
declaracao_variavel_no_ini
    : TK_VAR TK_ID TK_ATRIB tipo { $$ = semantica_declaracao_variavel_no_ini($2, $4); }
;

/* VARIÁVEL: Uma variavel que pode ou não ter uma atribuição*/
declaracao_variavel
    : TK_VAR TK_ID TK_ATRIB tipo atribuicao { $$ = semantica_declaracao_variavel($2, $4, $5); }
;

/* TIPO: "Tipo" pode ser ou um inteiro ou um decimal, como especificado na descrição da linguagem */
/* Não geram nós na AST. */
tipo
    : TK_INTEIRO    { $$ = TIPO_INT; }
    | TK_DECIMAL    { $$ = TIPO_DEC; }
;

/*  ATRIBUIÇÃO: Uma atribuição pode pode ser de um inteiro ou de um decimal, ou não existe e é apenas uma declaração */
atribuicao
    : TK_COM tipo_valor { $$ = $2; }
    | %empty            { $$ = NULL; }
;

/* Criam nós folha na AST com o valor do literal e liberam a memória do token */
tipo_valor
    : TK_LI_DECIMAL { $$ = criar_no_folha($1, TIPO_DEC, NAT_LITERAL); }
    | TK_LI_INTEIRO { $$ = criar_no_folha($1, TIPO_INT, NAT_LITERAL); }
;

/* ==================================================================== */
/* ============================ EXPRESSÃO ============================= */
/* ==================================================================== */

/* EXPRESSÃO: Expressões possuem vários níveis de acordo com sua precedência, quanto maior o nível menor o grau de precedência. Podem haver operadores unários e binários */
expressao 
    : expressao '|' nivel_6 { $$ = semantica_expressoes_binarias("|", $1, $3); }
    | nivel_6               { $$ = $1; }
;

nivel_6
    : nivel_6 '&' nivel_5   { $$ = semantica_expressoes_binarias("&", $1, $3); }
    | nivel_5               { $$ = $1; }
;

nivel_5
    : nivel_5 TK_OC_EQ nivel_4  { $$ = semantica_expressoes_binarias("==", $1, $3); }
    | nivel_5 TK_OC_NE nivel_4  { $$ = semantica_expressoes_binarias("!=", $1, $3); }
    | nivel_4                   { $$ = $1; }
;

nivel_4
    : nivel_4 '<' nivel_3       { $$ = semantica_expressoes_binarias("<", $1, $3); }
    | nivel_4 '>' nivel_3       { $$ = semantica_expressoes_binarias(">", $1, $3); }
    | nivel_4 TK_OC_LE nivel_3  { $$ = semantica_expressoes_binarias("<=", $1, $3); }
    | nivel_4 TK_OC_GE nivel_3  { $$ = semantica_expressoes_binarias(">=", $1, $3); }
    | nivel_3                   { $$ = $1; }
;

nivel_3
    : nivel_3 '+' nivel_2   { $$ = semantica_expressoes_binarias("+", $1, $3); }
    | nivel_3 '-' nivel_2   { $$ = semantica_expressoes_binarias("-", $1, $3); }
    | nivel_2               { $$ = $1; }
;

nivel_2
    : nivel_2 '*' nivel_1   { $$ = semantica_expressoes_binarias("*", $1, $3); }
    | nivel_2 '/' nivel_1   { $$ = semantica_expressoes_binarias("/", $1, $3); }
    | nivel_2 '%' nivel_1   { $$ = semantica_expressoes_binarias("%", $1, $3); }
    | nivel_1               { $$ = $1; }
;

nivel_1
    : '+' nivel_1 { $$ = criar_no_unario("+", $2->data_type, $2); }
    | '-' nivel_1 { $$ = criar_no_unario("-", $2->data_type, $2); }
    | '!' nivel_1 { $$ = criar_no_unario("!", $2->data_type, $2); }
    | nivel_0     { $$ = $1; }
;

nivel_0
    : chamada_funcao    { $$ = $1; }
    | TK_ID             { $$ = semantica_identificador_variavel($1); }
    | TK_LI_INTEIRO     { $$ = criar_no_folha($1, TIPO_INT, NAT_LITERAL); }
    | TK_LI_DECIMAL     { $$ = criar_no_folha($1, TIPO_DEC, NAT_LITERAL); }
    | '(' expressao ')' { $$ = $2; }
;

%%

/* Função chamada pelo Bison em caso de erro de sintaxe. Imprime uma mensagem de erro formatada, incluindo o número da linha. */
void yyerror (char const *mensagem) {
    printf("==================================================================\n");
    printf ("ERRO: Linha %i - [%s]\n", get_line_number(), mensagem);
    printf("==================================================================\n");

    /* Se ocorreu um erro e a pilha de escopo ainda existe, ela é destruída para evitar vazamento de memória. */
    semantica_pop_scope_error();
}