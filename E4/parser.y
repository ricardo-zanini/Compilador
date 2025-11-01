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
    #include "parser.tab.h" /* Para garantir que ValorLexico seja conhecido pelos protótipos mais abaixo */

    /* Declarações de funções que serão utilizadas nas ações semânticas e sintáticas. */
    int yylex(void);
    void yyerror (char const *mensagem);
    int get_line_number();

    extern asd_tree_t *arvore; /* Variável externa (de "main.c") que serve como ponteiro para a raiz da AST a ser construída */
    EscopoPilha *g_pilha_escopo = NULL; /* Pilha global */

    /* ============================================================================================ */
    /* ======================== PROTÓTIPOS DAS FUNÇÕES AUXILIARES PARA A AST ====================== */
    /* ============================================================================================ */
    static asd_tree_t* criar_no_folha(ValorLexico* token, TipoDados tipo);
    static asd_tree_t* criar_no_unario(const char* op_label, TipoDados tipo, asd_tree_t* filho);
    static asd_tree_t* criar_no_binario(const char* op_label, TipoDados tipo, asd_tree_t* filho1, asd_tree_t* filho2);
    static void free_token(ValorLexico* token);

    /* Função para tratar da semântica das expressões binárias, verificando tipos e criando nó na árvore */
    static asd_tree_t* semantica_expressoes_binarias(char* operador, asd_tree_t* filho1, asd_tree_t* filho2, int num_linha);
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
    int tipo_dado;
}

/* Informa ao Bison como liberar os tokens quando eles são descartados */
%destructor { free_token($$); } <valor_lexico>

/* Informa ao Bison como liberar nós da AST que são descartados durante a recuperação de erro. */
%destructor { if ($$) asd_free($$); } <ast_node>

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

/* Define o tipo de retorno para as regras da gramática como um nó da AST */
%type <ast_node> programa lista elemento
%type <ast_node> funcao lista_parametros_ini lista_parametros parametro
%type <ast_node> comando_simples bloco_comandos bloco_comandos_conteudo bloco_comandos_conteudo_lista
%type <ast_node> comando_atribuicao chamada_funcao lista_argumentos_ini lista_argumentos comando_retorno
%type <ast_node> comando_controle_fluxo condicional condicional_else repeticao
%type <ast_node> declaracao_variavel_no_ini declaracao_variavel atribuicao tipo_valor
%type <ast_node> expressao nivel_6 nivel_5 nivel_4 nivel_3 nivel_2 nivel_1 nivel_0
%type <tipo_dado> tipo

// Faz o Bison começar pela regra "wrapper" (permitindo iniciar o gerenciamento da pilha e de escopo)
%start programa_wrapper

%%

/* ==================================================================== */
/* ========================== REGRAS INICIAIS ========================= */
/* ==================================================================== */

/* Regra 'wrapper' gerencia o escopo global. Executada uma única vez. */
programa_wrapper
    : { stack_push(&g_pilha_escopo); } /* Cria o escopo global */
      programa /* Executa o parsing do programa */
      {
        (void)$2; /* Silencia o aviso "unused value" */
        if (g_pilha_escopo != NULL) {
            stack_pop(&g_pilha_escopo);
        }
      } /* Destrói o escopo global */
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
    | elemento ',' lista
    {   
        if ($1) { // Se o elemento atual não for nulo
            if ($3) asd_add_child($1, $3); // Adiciona o resto da lista como filho
            $$ = $1;
        } else { // Se o elemento atual for nulo, a lista começa a partir do próximo
            $$ = $3;
        }
    }
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
    : TK_ID TK_SETA tipo
    {
        // Declara a função no escopo ATUAL (externo)
        Simbolo *entrada_fun = create_entry_fun($1->valor_token, $3, $1);
        symbol_insert(g_pilha_escopo, $1->valor_token, entrada_fun);

        // Cria o novo escopo para os parâmetros e corpo
        stack_push(&g_pilha_escopo);
        g_pilha_escopo->tipo_retorno = $3;

    } lista_parametros_ini TK_ATRIB bloco_comandos {

        // Destrói o escopo da função
        stack_pop(&g_pilha_escopo); 

        $$ = criar_no_folha($1, $3);

        (void)$5;
        if ($7) asd_add_child($$, $7);
    }
;

/* CABEÇALHO DA FUNÇÃO: Pode ter zero ou mais parâmetros (O TOKEN TK_COM É OPCIONAL!!) */
/* Validam a sintaxe, mas retornam NULL pois os parâmetros não entram na AST. */
lista_parametros_ini
    : TK_COM lista_parametros   { (void)$2; $$ = NULL; }
    | lista_parametros          { (void)$1; $$ = NULL; }
    | %empty                    { $$ = NULL; }
;

lista_parametros
    : parametro                         { (void)$1; $$ = NULL; }
    | parametro ',' lista_parametros    { (void)$1; (void)$3; $$ = NULL; }
;

parametro
    : TK_ID TK_ATRIB tipo
    {
        Simbolo *entrada_param = create_entry_var($1->valor_token, $3, $1);
        symbol_insert(g_pilha_escopo, $1->valor_token, entrada_param);
        free_token($1);
        $$ = NULL;
    }
;

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
    : '[' { stack_push(&g_pilha_escopo); } bloco_comandos_conteudo ']'   { $$ = $3; stack_pop(&g_pilha_escopo); }
;

bloco_comandos_conteudo
    : %empty                            { $$ = NULL; }
    | bloco_comandos_conteudo_lista     { $$ = $1; }
;

bloco_comandos_conteudo_lista
    : comando_simples                                   { $$ = $1; }
    | comando_simples bloco_comandos_conteudo_lista
    {
        if ($1) { // Se o comando atual não for nulo
            if ($2) asd_add_child($1, $2); // Adiciona o resto da lista como filho
            $$ = $1;
        } else {
            $$ = $2; // Se o comando atual for nulo, a lista começa a partir do próximo
        }
    }
; 

/* COMANDO DE ATRIBUIÇÃO: Identificador, seguido de atribuição e expressão*/
comando_atribuicao
    : TK_ID TK_ATRIB expressao
    {
        /* Verifica se o identificador já foi declarado */
        Simbolo *entrada = symbol_lookup(g_pilha_escopo, $1->valor_token);
        if (entrada == NULL) report_error_undeclared($1);
        
        /* Caso seja uma função, indica erro */
        if (entrada->natureza == NAT_FUNCAO) report_error_function_as_variable($1);
        
        /* Caso o tipo do indentificador não seja o mesmo da expressão, indica erro */
        if(entrada->tipo_dado != $3->data_type) report_error_wrong_type_assignment($1, entrada->tipo_dado, $3->data_type);

        $$ = criar_no_binario(":=", entrada->tipo_dado, criar_no_folha($1, entrada->tipo_dado), $3); /* label é o lexema de TK_ATRIB */
    }
;

/* CHAMADA DE FUNÇÃO: Chamada de função pode ter 0 ou mais argumentos listados entre parênteses*/
chamada_funcao
    : TK_ID '(' lista_argumentos_ini ')'
    {
        /* Verifica se o identificador já foi declarado */
        Simbolo *entrada = symbol_lookup(g_pilha_escopo, $1->valor_token);
        if (entrada == NULL) report_error_undeclared($1);

        /* Caso seja uma variável, indica erro */
        if (entrada->natureza == NAT_IDENTIFICADOR) report_error_variable_as_function($1);
        
        char* label = malloc(strlen("call ") + strlen($1->valor_token) + 1); // label é 'call' seguido do nome da função
        sprintf(label, "call %s", $1->valor_token);
        free_token($1);

        /* Cria diretamente um nó na árvore */
        $$ = asd_new(label, entrada->tipo_dado);
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
        /* Caso o tipo da expressão não seja o mesmo que o informado, indica erro */
        if ($2->data_type != $4) report_error_wrong_type_return_expr(get_line_number(), $2->data_type, $4);

        /* Se o tipo de retorno não for o mesmo da função, indica erro */
        if ($4 != g_pilha_escopo->tipo_retorno) report_error_wrong_type_return_func(get_line_number(), $4, g_pilha_escopo->tipo_retorno);

        $$ = criar_no_unario("retorna", $2->data_type, $2); /* label é o lexema de TK_RETORNA */
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
        $$ = criar_no_unario("se", $3->data_type, $3); // label é o lexema de TK_SE
        if ($5) asd_add_child($$, $5);
        if ($6) {
            /* Se os tipos de dados do bloco do if e do bloco do else não forem compatíveis, indica erro */
            if ($5->data_type != $6->data_type) report_error_wrong_type_if_else(get_line_number(), $5->data_type, $6->data_type);

            asd_add_child($$, $6);
        }
    }
;

condicional_else
    : TK_SENAO bloco_comandos { $$ = $2; }
    | %empty                  { $$ = NULL; }
;

repeticao
    : TK_ENQUANTO '(' expressao ')' bloco_comandos { $$ = criar_no_unario("enquanto", $3->data_type, $3); /* label é o lexema de TK_ENQUANTO */ if ($5) asd_add_child($$, $5); }
;

/* ==================================================================== */
/* ============================ VARIÁVEIS ============================= */
/* ==================================================================== */

/* VARIÁVEL INI: Uma variavel que faz parte da linguagem, não pode ter atribuição */
/* Retorna NULL, pois não gera nó na AST, e libera a memória do token do identificador */
declaracao_variavel_no_ini
    : TK_VAR TK_ID TK_ATRIB tipo
    {
        Simbolo *entrada = create_entry_var($2->valor_token, $4, $2);
        symbol_insert(g_pilha_escopo, $2->valor_token, entrada);

        free_token($2);

        $$ = NULL;
    }
;

/* VARIÁVEL: Uma variavel que pode ou não ter uma atribuição*/
declaracao_variavel
    : TK_VAR TK_ID TK_ATRIB tipo atribuicao
    {
        Simbolo *entrada = create_entry_var($2->valor_token, $4, $2);
        symbol_insert(g_pilha_escopo, $2->valor_token, entrada);

        if ($5) {
            /* Se os tipo da vairável é diferente do tipo da atribuição, indica erro */
            if ($5->data_type != $4) report_error_wrong_type_initialization($2, $4, $5->data_type);

            $$ = criar_no_binario("com", $4, criar_no_folha($2, $4), $5); /* label é o lexema de TK_COM */
        } else {
            $$ = NULL; /* Mesmo comportamento de declaracao_variavel_no_ini */
            free_token($2);
        }
    }
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
    | %empty { $$ = NULL; }
;

/* Criam nós folha na AST com o valor do literal e liberam a memória do token */
tipo_valor
    : TK_LI_DECIMAL { $$ = criar_no_folha($1, TIPO_DEC); }
    | TK_LI_INTEIRO { $$ = criar_no_folha($1, TIPO_INT); }
;

/* ==================================================================== */
/* ============================ EXPRESSÃO ============================= */
/* ==================================================================== */

/* EXPRESSÃO: Expressões possuem vários níveis de acordo com sua precedência, quanto maior o nível menor o grau de precedência. Podem haver operadores unários e binários */
expressao 
    : expressao '|' nivel_6 { $$ = semantica_expressoes_binarias("|", $1, $3, get_line_number()); }
    | nivel_6               { $$ = $1; }
;

nivel_6
    : nivel_6 '&' nivel_5   { $$ = semantica_expressoes_binarias("&", $1, $3, get_line_number()); }
    | nivel_5               { $$ = $1; }
;

nivel_5
    : nivel_5 TK_OC_EQ nivel_4  { $$ = semantica_expressoes_binarias("==", $1, $3, get_line_number()); }
    | nivel_5 TK_OC_NE nivel_4  { $$ = semantica_expressoes_binarias("!=", $1, $3, get_line_number()); }
    | nivel_4                   { $$ = $1; }
;

nivel_4
    : nivel_4 '<' nivel_3       { $$ = semantica_expressoes_binarias("<", $1, $3, get_line_number()); }
    | nivel_4 '>' nivel_3       { $$ = semantica_expressoes_binarias(">", $1, $3, get_line_number()); }
    | nivel_4 TK_OC_LE nivel_3  { $$ = semantica_expressoes_binarias("<=", $1, $3, get_line_number()); }
    | nivel_4 TK_OC_GE nivel_3  { $$ = semantica_expressoes_binarias(">=", $1, $3, get_line_number()); }
    | nivel_3                   { $$ = $1; }
;

nivel_3
    : nivel_3 '+' nivel_2   { $$ = semantica_expressoes_binarias("+", $1, $3, get_line_number()); }
    | nivel_3 '-' nivel_2   { $$ = semantica_expressoes_binarias("-", $1, $3, get_line_number()); }
    | nivel_2               { $$ = $1; }
;

nivel_2
    : nivel_2 '*' nivel_1   { $$ = semantica_expressoes_binarias("*", $1, $3, get_line_number()); }
    | nivel_2 '/' nivel_1   { $$ = semantica_expressoes_binarias("/", $1, $3, get_line_number()); }
    | nivel_2 '%' nivel_1   { $$ = semantica_expressoes_binarias("%", $1, $3, get_line_number()); }
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
    | TK_ID
    {
        /* Verifica se o identificador já foi declarado */
        Simbolo *entrada = symbol_lookup(g_pilha_escopo, $1->valor_token);
        if (entrada == NULL) report_error_undeclared($1);

        /* Caso seja uma função, indica erro */
        if (entrada->natureza == NAT_FUNCAO) report_error_function_as_variable($1);

        $$ = criar_no_folha($1, entrada->tipo_dado);
    }
    | TK_LI_INTEIRO     { $$ = criar_no_folha($1, TIPO_INT); }
    | TK_LI_DECIMAL     { $$ = criar_no_folha($1, TIPO_DEC); }
    | '(' expressao ')' { $$ = $2; }
;

%%

/* ==================================================================== */
/* =================== FUNÇÕES AUXILIARES PARA A AST ================== */
/* ==================================================================== */

/* Cria um nó folha da AST a partir de um valor léxico */
static asd_tree_t* criar_no_folha(ValorLexico* token, TipoDados tipo)
{
    if (!token) return NULL;
    asd_tree_t* no = asd_new(token->valor_token, tipo);
    free_token(token);
    return no;
}

/* Cria um nó de operador unário */
static asd_tree_t* criar_no_unario(const char* op_label, TipoDados tipo, asd_tree_t* filho)
{
    asd_tree_t* no = asd_new(op_label, tipo);
    asd_add_child(no, filho);
    return no;
}

/* Cria um nó de operador binário */
static asd_tree_t* criar_no_binario(const char* op_label, TipoDados tipo, asd_tree_t* filho1, asd_tree_t* filho2)
{
    asd_tree_t* no = asd_new(op_label, tipo);
    asd_add_child(no, filho1);
    asd_add_child(no, filho2);
    return no;
}

/* Libera um token */
static void free_token(ValorLexico* token)
{
    if (!token) return;
    free(token->valor_token);
    free(token);
}

/* Função para tratar da semântica das expressões binárias, verificando tipos e criando nó na árvore */
static asd_tree_t* semantica_expressoes_binarias(char* operador, asd_tree_t* filho1, asd_tree_t* filho2, int num_linha) {

    /* Se os tipos dos dois lados da expressão não forem compatíveis, indica erro */
    if (filho1->data_type != filho2->data_type) report_error_wrong_type_binary_op(num_linha, filho1->data_type, filho2->data_type, operador);
    
    return criar_no_binario(operador, filho1->data_type, filho1, filho2);
}

/* Função chamada pelo Bison em caso de erro de sintaxe. Imprime uma mensagem de erro formatada, incluindo o número da linha. */
void yyerror (char const *mensagem) {
    printf("==================================================================\n");
    printf ("ERRO: Linha %i - [%s]\n", get_line_number(), mensagem);
    printf("==================================================================\n");

    /* Se ocorreu um erro e a pilha de escopo global ainda existe, ela é destruída para evitar vazamento de memória. */
    if (g_pilha_escopo != NULL) {
        stack_pop(&g_pilha_escopo);
    }
}