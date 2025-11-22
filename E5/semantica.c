#include "semantica.h"
#include "iloc.h"
#include <string.h>
#include <stdlib.h>

/* A pilha global de escopo */
EscopoPilha *g_pilha_escopo = NULL;

// Variáveis de endereçamento global e local
static int offset_global = 0; 
static int offset_local = 0;

/* ==================================================================== */
/* =============== FUNÇÕES PARA CÓDIGO INTERMEDIARIO ================== */
/* ==================================================================== */

/* Função auxiliar para verificar se o escopo atual é global */
int is_scope_global() {
    /* Se o escopo atual não tem pai, ele é a base (global) */
    return (g_pilha_escopo->escopo_pai == NULL);
}

void processar_declaracao_var(Simbolo *entrada) {
    if (is_scope_global()) {
        entrada->is_global = 1;
        entrada->deslocamento = offset_global;
        offset_global += 4; // Incrementa 4 bytes (tamanho int/float padrão)
    } else {
        entrada->is_global = 0;
        entrada->deslocamento = offset_local;
        offset_local += 4;
    }
}

/* ==================================================================== */
/* =================== FUNÇÕES AUXILIARES PARA A AST ================== */
/* ==================================================================== */

/* Cria um nó folha da AST a partir de um valor léxico */
asd_tree_t* criar_no_folha(ValorLexico* token, TipoDados tipo, NaturezaSimbolo natureza)
{
    if (!token) return NULL;

    // =======================================================
    // ============== TRECHO CÓDIGO INTERMED =================

    // Variaveis necessárias para geração de código
    ListaILOC* lista_codigo = NULL;
    char* temporario_retorno = NULL;

    // Geração de código para literais inteiros
    if(tipo == TIPO_INT && natureza == NAT_LITERAL){

        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();

        adicionar_operacao(lista_codigo, criar_loadI(atoi(token->valor_token), temporario_retorno));
    }

    // =======================================================
    // =======================================================

    asd_tree_t* no = asd_new(token->valor_token, tipo, token->num_linha, temporario_retorno, lista_codigo);

    free(temporario_retorno);
    free_token(token);
    return no;
}

/* Cria um nó de operador unário */
asd_tree_t* criar_no_unario(const char* op_label, TipoDados tipo, asd_tree_t* filho)
{

    // Ordem de concatenação do código pai -> filho
    OrdemConcatenacao ordem = ORD_NORM;

    // Variaveis necessárias para geração de código
    ListaILOC* lista_codigo = NULL;
    char* temporario_retorno = NULL;
    
    // Operação para indicar número positivo
    if(strcmp(op_label, "+") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_i2i(filho->temp, temporario_retorno)); // Acho que não seria necessário copiar o valor do registrador, mas vou fazer na dúvida
        
    // Operação para trocar sinal de número
    }else if(strcmp(op_label, "-") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_rsubI(0, filho->temp, temporario_retorno));

    // Operação de negação
    }else if(strcmp(op_label, "!") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        char* temporario_zero = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_loadI(0, temporario_zero));
        adicionar_operacao(lista_codigo, criar_comparacao("cmp_EQ", temporario_zero, filho->temp, temporario_retorno));
        free(temporario_zero);
    }

    asd_tree_t* no = asd_new(op_label, tipo, filho->num_linha, temporario_retorno, lista_codigo);
    asd_add_child(no, filho, ordem);

    free(temporario_retorno);
    return no;
}

/* Cria um nó de operador binário */
asd_tree_t* criar_no_binario(const char* op_label, TipoDados tipo, asd_tree_t* filho1, asd_tree_t* filho2)
{

    // Ordem de concatenação do código pai -> filho
    OrdemConcatenacao ordem = ORD_NORM;

    // Variaveis necessárias para geração de código
    ListaILOC* lista_codigo = NULL;
    char* temporario_retorno = NULL;
    
    if(strcmp(op_label, "+") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_aritmetica("add", filho1->temp, filho2->temp, temporario_retorno));
        
    }else if(strcmp(op_label, "-") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_aritmetica("sub", filho1->temp, filho2->temp, temporario_retorno));

    }else if(strcmp(op_label, "*") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_aritmetica("mult", filho1->temp, filho2->temp, temporario_retorno));

    }else if(strcmp(op_label, "/") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_aritmetica("div", filho1->temp, filho2->temp, temporario_retorno));

    }else if(strcmp(op_label, "<") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_comparacao("cmp_LT", filho1->temp, filho2->temp, temporario_retorno));

    }else if(strcmp(op_label, ">") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_comparacao("cmp_GT", filho1->temp, filho2->temp, temporario_retorno));

    }else if(strcmp(op_label, "<=") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_comparacao("cmp_LE", filho1->temp, filho2->temp, temporario_retorno));

    }else if(strcmp(op_label, ">=") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_comparacao("cmp_GE", filho1->temp, filho2->temp, temporario_retorno));

    }else if(strcmp(op_label, "==") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_comparacao("cmp_EQ", filho1->temp, filho2->temp, temporario_retorno));

    }else if(strcmp(op_label, "!=") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_comparacao("cmp_NE", filho1->temp, filho2->temp, temporario_retorno));

    }else if(strcmp(op_label, "&") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_comparacao("and", filho1->temp, filho2->temp, temporario_retorno));

    }else if(strcmp(op_label, "|") == 0){
        ordem = ORD_INV;
        lista_codigo = criar_lista_iloc();
        temporario_retorno = gerar_temporario();
        adicionar_operacao(lista_codigo, criar_comparacao("or", filho1->temp, filho2->temp, temporario_retorno));

    }

    asd_tree_t* no = asd_new(op_label, tipo, filho1->num_linha, temporario_retorno, lista_codigo);
    asd_add_child(no, filho1, ordem);
    asd_add_child(no, filho2, ordem);

    free(temporario_retorno);
    return no;
}

/* Libera um token */
void free_token(ValorLexico* token)
{
    if (!token) return;
    free(token->valor_token);
    free(token);
}

/* ========================================================= */
/* =================== FUNÇÕES SEMÂNTICAS ================== */
/* ========================================================= */

/* Gerenciamento de escopo (push) */
void semantica_push_scope() {
    stack_push(&g_pilha_escopo);
}

/* Gerenciamento de escopo (pop)*/
void semantica_pop_scope() {
    stack_pop(&g_pilha_escopo);
}

/* Gerenciamento de escopo (pop) em caso de erro */
void semantica_pop_scope_error() {
    /* Se ocorreu um erro e a pilha de escopo ainda existe, ela é destruída para evitar vazamento de memória. */
    while (g_pilha_escopo != NULL) {
        stack_pop(&g_pilha_escopo);
    }
}

/* Função para tratar da semântica da declaração das funções */
void semantica_funcao_declaracao(ValorLexico* ident, TipoDados tipo) {
    /* Declara a função no escopo ATUAL (externo) */
    Simbolo *entrada_fun = create_entry_fun(ident->valor_token, tipo, ident);
    symbol_insert(g_pilha_escopo, ident->valor_token, entrada_fun);

    /* Cria o novo escopo para os parâmetros e corpo */
    semantica_push_scope();
    g_pilha_escopo->tipo_retorno = tipo;
    g_pilha_escopo->funcao_atual = entrada_fun;
}

/* Função para tratar da semântica de atualização da lista de parâmetros de uma função */
void semantica_funcao_atualizar_args(ArgLista* lista_param) {
    /* Verifica se estamos em um escopo de função */
    if (g_pilha_escopo && g_pilha_escopo->funcao_atual) {
        
        /* Atualiza o símbolo da função (que está no escopo pai) */
        g_pilha_escopo->funcao_atual->argumentos = lista_param;
        
        /* Conta os parâmetros */
        int count = 0;
        ArgLista* node = lista_param;
        while(node) {
            count++;
            node = node->next;
        }
        g_pilha_escopo->funcao_atual->num_argumentos = count;
    }
}

/* Função para tratar da semântica da definição das funções */
asd_tree_t* semantica_funcao_definicao(ValorLexico* ident, TipoDados tipo, asd_tree_t* corpo) {
    
    /* Destrói o escopo da função */
    semantica_pop_scope(); 

    /* Cria o nó da função*/
    asd_tree_t* no = criar_no_folha(ident, tipo, NAT_FUNCAO);

    if (corpo) asd_add_child(no, corpo, ORD_NORM);

    return no;
}

/* Função para tratar da semântica dos parâmetros de funções */
ArgLista* semantica_param(ValorLexico* ident, TipoDados tipo) {
    /* Declara o parâmetro como variável no escopo atual */
    Simbolo *entrada_param = create_entry_var(ident->valor_token, tipo, ident);
    symbol_insert(g_pilha_escopo, ident->valor_token, entrada_param);
    free_token(ident);

    /* Cria o nó da lista de parâmetros para retornar */
    ArgLista* param = (ArgLista*) malloc(sizeof(ArgLista));
    param->nome = strdup(entrada_param->chave); /* Usa a chave já copiada */
    param->tipo = tipo;
    param->next = NULL;

    return param;
}

/* Função para tratar da semântica dos comandos de atribuição */
asd_tree_t* semantica_comando_atrib(ValorLexico* ident, asd_tree_t* exp) {
    /* Verifica se o identificador já foi declarado */
    Simbolo *entrada = symbol_lookup(g_pilha_escopo, ident->valor_token);
    if (entrada == NULL) report_error_undeclared(ident);
    
    /* Caso seja uma função, indica erro */
    if (entrada->natureza == NAT_FUNCAO) report_error_function_as_variable(ident);
    
    /* Caso o tipo do indentificador não seja o mesmo da expressão, indica erro */
    if(entrada->tipo_dado != exp->data_type) report_error_wrong_type_assignment(ident, entrada->tipo_dado, exp->data_type);

    // Variaveis necessárias para geração de código
    ListaILOC* lista_codigo = criar_lista_iloc();

    adicionar_operacao(lista_codigo, criar_storeAI(
        exp->temp,
        (entrada->is_global == 1) ? "rbss" : "rfp", 
        entrada->deslocamento
    ));
    
    asd_tree_t* no_identificador = criar_no_folha(ident, entrada->tipo_dado, entrada->natureza);
    asd_tree_t* no = asd_new(":=", entrada->tipo_dado, no_identificador->num_linha, exp->temp, lista_codigo); /* label é o lexema de TK_ATRIB */

    asd_add_child(no, no_identificador, ORD_NORM);
    asd_add_child(no, exp, ORD_INV);

    return no;
}

/* Função para tratar da semântica das chamadas de função */
asd_tree_t* semantica_chamada_func(ValorLexico* ident, asd_tree_t* lista_arg) {
    /* Verifica se o identificador já foi declarado */
    Simbolo *entrada = symbol_lookup(g_pilha_escopo, ident->valor_token);
    if (entrada == NULL) report_error_undeclared(ident);

    /* Caso seja uma variável, indica erro */
    if (entrada->natureza == NAT_IDENTIFICADOR) report_error_variable_as_function(ident);

    /* Cria o nó da chamada de função */
    char* label = malloc(strlen("call ") + strlen(ident->valor_token) + 1); /* label é 'call' seguido do nome da função */
    sprintf(label, "call %s", ident->valor_token);
    asd_tree_t* no_call = asd_new(label, entrada->tipo_dado, ident->num_linha, NULL, NULL);
    free(label);

    /* Verifica se há argumentos e pega a contagem */
    int provided_count = 0;
    asd_tree_t* arg_list_node = lista_arg;
    if (arg_list_node) {
        provided_count = arg_list_node->number_of_children;
    }

    /* Verifica a contagem */
    int expected_count = entrada->num_argumentos;

    if (provided_count < expected_count) {
        report_error_missing_args(ident, expected_count, provided_count); /* Se tem menos argumentos do que deveria, indica erro */
    }

    if (provided_count > expected_count) {
        report_error_excess_args(ident, expected_count, provided_count); /* Se tem mais argumentos do que deveria, indica erro */
    }

    /* Verifica os tipos dos argumentos (iterando a lista plana) */
    ArgLista* param_iter = entrada->argumentos;
    int arg_index = 0;

    if (arg_list_node) {
        for (arg_index = 0; arg_index < provided_count; arg_index++) {

            if (param_iter == NULL) break; 

            asd_tree_t* arg_node = arg_list_node->children[arg_index];

            /* Se o argumento e seu respectivo parâmetro são de tipos diferentes, indica erro */
            if (param_iter->tipo != arg_node->data_type) {
                report_error_wrong_type_args(ident, arg_index, param_iter->tipo, arg_node->data_type);
            }
            
            param_iter = param_iter->next;
        }
    }

    /* ================================================================== */
    /* ============ FASE 2: REESTRUTURAÇÃO PARA FORMATO E3 ============ */
    /* ================================================================== */

    /* Reestrutura os nós dos argumentos na árvore para montar uma lista encadeada (ao invés de todos os argumentos serem filhos do nó 'call') */
    if (arg_list_node && provided_count > 0) {
        
        /* Pega o primeiro argumento. Ele será o único filho do 'call' */
        asd_tree_t* top_of_chain = arg_list_node->children[0];
        asd_tree_t* current_node = top_of_chain;

        /* Loop para encadear os restantes (recriando a lista encadeada) */
        for (int i = 1; i < provided_count; i++) {
            asd_add_child(current_node, arg_list_node->children[i], ORD_NORM);
            current_node = arg_list_node->children[i];
        }

        /* Anexa apenas o topo da cadeia ao nó 'call' */
        asd_add_child(no_call, top_of_chain, ORD_NORM);

        /* Desconecta os filhos (que agora estão na nova lista) */
        asd_detach_children(arg_list_node);
        
        /* Libera o nó 'arg_list_temp', que agora está vazio */
        asd_free(arg_list_node);

    } else if (arg_list_node) {
        /* Libera o nó 'arg_list_temp', que está vazio */
        asd_free(arg_list_node);
    }

    free_token(ident);

    return no_call;
}

/* Função para tratar da semântica dos comandos de retorno de função */
asd_tree_t* semantica_comando_ret(asd_tree_t* exp, TipoDados tipo) {
    /* Caso o tipo da expressão não seja o mesmo que o informado, indica erro */
    if (exp->data_type != tipo) report_error_wrong_type_return_expr(exp->num_linha, exp->data_type, tipo);

    /* Se o tipo de retorno não for o mesmo da função, indica erro */
    if (tipo != g_pilha_escopo->tipo_retorno) report_error_wrong_type_return_func(exp->num_linha, tipo, g_pilha_escopo->tipo_retorno);

    return criar_no_unario("retorna", exp->data_type, exp); /* label é o lexema de TK_RETORNA */
}

/* Função para tratar da semântica dos comandos condicionais */
asd_tree_t* semantica_condicional(asd_tree_t* exp, asd_tree_t* bloco_if, asd_tree_t* bloco_else) {

    // Variáveis necessárias para criação de código
    ListaILOC* lista_codigo = criar_lista_iloc();
    char *rotulo_if, *rotulo_else, *rotulo_fim_else;

    // Geração dos rótulos de salto
    rotulo_if = gerar_rotulo();
    rotulo_else = gerar_rotulo();
    rotulo_fim_else = gerar_rotulo();

    // Criação de pulo condicional IF-ELSE
    adicionar_operacao(lista_codigo, criar_cbr(exp->temp, rotulo_if, rotulo_else));
    
    // Caso não exista um bloco de if, criar um campo código artificial
    ListaILOC* lista_if = NULL;
    if (bloco_if) {
        if (bloco_if->codigo == NULL) bloco_if->codigo = criar_lista_iloc();
        lista_if = bloco_if->codigo;
    } else {
        lista_if = criar_lista_iloc();
    }
    
    // Adicionar operações no bloco IF, salto e rótulo
    adicionar_operacao_ini(lista_if, criar_nop_com_rotulo(rotulo_if));
    adicionar_operacao(lista_if, criar_jumpI(rotulo_fim_else));

    // Caso não exista um bloco de else, criar um campo código artificial
    ListaILOC* lista_else = NULL;
    if (bloco_else) {
        if (bloco_else->codigo == NULL) bloco_else->codigo = criar_lista_iloc();
        lista_else = bloco_else->codigo;
    } else {
        lista_else = criar_lista_iloc();
    }

    // Adiciona rótulos na lista do ELSE
    adicionar_operacao_ini(lista_else, criar_nop_com_rotulo(rotulo_else));
    adicionar_operacao(lista_else, criar_nop_com_rotulo(rotulo_fim_else));
    
    // Cria o nó principal 'se'
    asd_tree_t* no = asd_new("se", exp->data_type, exp->num_linha, NULL, lista_codigo);
    asd_add_child(no, exp, ORD_INV);

    // Se não existir bloco if, concatenamos nó com lista artificial
    if (bloco_if){
        asd_add_child(no, bloco_if, ORD_NORM);
    } else {
        concatenar_listas(no->codigo, lista_if, ORD_NORM);
    }

    // Se não existir bloco else, concatenamos nó com lista artificial
    if (bloco_else) {
        /* Se os tipos de dados do bloco do if e do bloco do else não forem compatíveis, indica erro */
        if (bloco_if && bloco_if->data_type != bloco_else->data_type) 
            report_error_wrong_type_if_else(exp->num_linha, bloco_if->data_type, bloco_else->data_type);

        asd_add_child(no, bloco_else, ORD_NORM);
    } else {
        concatenar_listas(no->codigo, lista_else, ORD_NORM);
    }

    free(rotulo_if);
    free(rotulo_else);
    free(rotulo_fim_else);

    return no;
}

asd_tree_t* semantica_enquanto(asd_tree_t* exp, asd_tree_t* bloco){ 

    // Variáveis necessárias para criação de código
    ListaILOC* lista_codigo = criar_lista_iloc();
    char *rotulo_test, *rotulo_loop, *rotulo_fim_loop;

    // Geração dos rótulos de salto
    rotulo_test = gerar_rotulo();
    rotulo_loop = gerar_rotulo();
    rotulo_fim_loop = gerar_rotulo();

    // Criação de pulo condicional do loop
    adicionar_operacao_ini(exp->codigo, criar_nop_com_rotulo(rotulo_test));
    adicionar_operacao(lista_codigo, criar_cbr(exp->temp, rotulo_loop, rotulo_fim_loop));

    // Caso não exista um bloco, criar um campo código artificial
    ListaILOC* lista_bloco = NULL;
    if (bloco) {
        if (bloco->codigo == NULL) bloco->codigo = criar_lista_iloc();
        lista_bloco = bloco->codigo;
    } else {
        lista_bloco = criar_lista_iloc();
    }

    // Adição dos rótulos
    adicionar_operacao_ini(lista_bloco, criar_nop_com_rotulo(rotulo_loop));
    adicionar_operacao(lista_bloco, criar_jumpI(rotulo_test));
    adicionar_operacao(lista_bloco, criar_nop_com_rotulo(rotulo_fim_loop));

    asd_tree_t* no = asd_new("enquanto", exp->data_type, exp->num_linha, NULL, lista_codigo);
    asd_add_child(no, exp, ORD_INV);
        
    if (bloco){
        asd_add_child(no, bloco, ORD_NORM);
    }else{
        concatenar_listas(no->codigo, lista_bloco, ORD_NORM);
    }

    free(rotulo_test);
    free(rotulo_loop);
    free(rotulo_fim_loop);

    return no;
}

/* Função para tratar da semântica das declarações de variáveis globais */
asd_tree_t* semantica_declaracao_variavel_no_ini(ValorLexico* ident, TipoDados tipo) {
    Simbolo *entrada = create_entry_var(ident->valor_token, tipo, ident);
    symbol_insert(g_pilha_escopo, ident->valor_token, entrada);

    processar_declaracao_var(entrada);

    free_token(ident);
    return NULL;
}

/* Função para tratar da semântica das declarações de variáveis */
asd_tree_t* semantica_declaracao_variavel(ValorLexico* ident, TipoDados tipo, asd_tree_t* atrib) {
    Simbolo *entrada = create_entry_var(ident->valor_token, tipo, ident);
    symbol_insert(g_pilha_escopo, ident->valor_token, entrada);

    if (atrib) {
        /* Se os tipo da vairável é diferente do tipo da atribuição, indica erro */
        if (atrib->data_type != tipo) report_error_wrong_type_initialization(ident, tipo, atrib->data_type);

        processar_declaracao_var(entrada);

        // Variaveis necessárias para geração de código
        ListaILOC* lista_codigo = criar_lista_iloc();
        char* temporario_retorno;


        temporario_retorno = gerar_temporario();

        adicionar_operacao(lista_codigo, criar_storeAI(
            atrib->temp,
            (entrada->is_global == 1) ? "rbss" : "rfp", 
            entrada->deslocamento
        ));

        asd_tree_t* no_identificador = criar_no_folha(ident, tipo, NAT_IDENTIFICADOR);
        asd_tree_t* no = asd_new("com", tipo, no_identificador->num_linha, temporario_retorno, lista_codigo);

        
        asd_add_child(no, no_identificador, ORD_NORM);
        asd_add_child(no, atrib, ORD_INV);
        
        free(temporario_retorno);
        return no;

    } else {
        free_token(ident);
        return NULL; /* Mesmo comportamento de declaracao_variavel_no_ini */
    }
}

/* Função para tratar da semântica dos identificadores/variáveis */
asd_tree_t* semantica_identificador_variavel(ValorLexico* ident) {
    /* Verifica se o identificador já foi declarado */
    Simbolo *entrada = symbol_lookup(g_pilha_escopo, ident->valor_token);
    if (entrada == NULL) report_error_undeclared(ident);

    /* Caso seja uma função, indica erro */
    if (entrada->natureza == NAT_FUNCAO) report_error_function_as_variable(ident);

    if (!ident) return NULL;

    // Variáveis para tratamento de código
    ListaILOC* lista_codigo = criar_lista_iloc();
    char* temporario;

    // loadAI ry(base do escopo local ou global), cz(deslocamento) => rx(temporario)
    temporario = gerar_temporario();
    adicionar_operacao(lista_codigo, criar_loadAI(
        (entrada->is_global == 1) ? "rbss" : "rfp", 
        entrada->deslocamento,
        temporario
    ));

    asd_tree_t* no = asd_new(ident->valor_token, entrada->tipo_dado, ident->num_linha, temporario, lista_codigo);

    free(temporario);
    free_token(ident);

    return no;
}

/* Função para tratar da semântica das expressões binárias, verificando tipos e criando nó na árvore */
asd_tree_t* semantica_expressoes_binarias(char* operador, asd_tree_t* filho1, asd_tree_t* filho2) {

    /* Se os tipos dos dois lados da expressão não forem compatíveis, indica erro */
    if (filho1->data_type != filho2->data_type) report_error_wrong_type_binary_op(filho1->num_linha, filho1->data_type, filho2->data_type, operador);
    
    return criar_no_binario(operador, filho1->data_type, filho1, filho2);
}