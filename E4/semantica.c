#include "semantica.h"

/* A pilha global de escopo */
EscopoPilha *g_pilha_escopo = NULL;

/* ==================================================================== */
/* =================== FUNÇÕES AUXILIARES PARA A AST ================== */
/* ==================================================================== */

/* Cria um nó folha da AST a partir de um valor léxico */
asd_tree_t* criar_no_folha(ValorLexico* token, TipoDados tipo)
{
    if (!token) return NULL;
    asd_tree_t* no = asd_new(token->valor_token, tipo, token->num_linha);
    free_token(token);
    return no;
}

/* Cria um nó de operador unário */
asd_tree_t* criar_no_unario(const char* op_label, TipoDados tipo, asd_tree_t* filho)
{
    asd_tree_t* no = asd_new(op_label, tipo, filho->num_linha);
    asd_add_child(no, filho);
    return no;
}

/* Cria um nó de operador binário */
asd_tree_t* criar_no_binario(const char* op_label, TipoDados tipo, asd_tree_t* filho1, asd_tree_t* filho2)
{
    asd_tree_t* no = asd_new(op_label, tipo, filho1->num_linha);
    asd_add_child(no, filho1);
    asd_add_child(no, filho2);
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

/* Função para tratar da semântica da definição das funções (parâmetros e ações finais) */
asd_tree_t* semantica_funcao_definicao(ValorLexico* ident, TipoDados tipo, ArgLista* lista_param, asd_tree_t* corpo) {
    /* Anexa a lista de parâmetros ao símbolo da função */
    if (g_pilha_escopo->funcao_atual) {
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

    /* Destrói o escopo da função */
    semantica_pop_scope(); 

    /* Cria o nó da função*/
    asd_tree_t* no = criar_no_folha(ident, tipo);

    (void)lista_param; /* Silencia warnings*/
    if (corpo) asd_add_child(no, corpo);

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

    return criar_no_binario(":=", entrada->tipo_dado, criar_no_folha(ident, entrada->tipo_dado), exp); /* label é o lexema de TK_ATRIB */
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
    asd_tree_t* no_call = asd_new(label, entrada->tipo_dado, ident->num_linha);
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
            asd_add_child(current_node, arg_list_node->children[i]);
            current_node = arg_list_node->children[i];
        }

        /* Anexa apenas o topo da cadeia ao nó 'call' */
        asd_add_child(no_call, top_of_chain);

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
    asd_tree_t* no = criar_no_unario("se", exp->data_type, exp); /* label é o lexema de TK_SE */
    if (bloco_if) asd_add_child(no, bloco_if);
    if (bloco_else) {
        /* Se os tipos de dados do bloco do if e do bloco do else não forem compatíveis, indica erro */
        /* Se o bloco if está vazio, assume que é do mesmo tipo do bloco else */
        if (bloco_if && bloco_if->data_type != bloco_else->data_type) report_error_wrong_type_if_else(exp->num_linha, bloco_if->data_type, bloco_else->data_type);

        asd_add_child(no, bloco_else);
    }

    return no;
}

/* Função para tratar da semântica das declarações de variáveis globais */
asd_tree_t* semantica_declaracao_variavel_no_ini(ValorLexico* ident, TipoDados tipo) {
    Simbolo *entrada = create_entry_var(ident->valor_token, tipo, ident);
    symbol_insert(g_pilha_escopo, ident->valor_token, entrada);
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

        return criar_no_binario("com", tipo, criar_no_folha(ident, tipo), atrib); /* label é o lexema de TK_COM */
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

    return criar_no_folha(ident, entrada->tipo_dado);
}

/* Função para tratar da semântica das expressões binárias, verificando tipos e criando nó na árvore */
asd_tree_t* semantica_expressoes_binarias(char* operador, asd_tree_t* filho1, asd_tree_t* filho2) {

    /* Se os tipos dos dois lados da expressão não forem compatíveis, indica erro */
    if (filho1->data_type != filho2->data_type) report_error_wrong_type_binary_op(filho1->num_linha, filho1->data_type, filho2->data_type, operador);
    
    return criar_no_binario(operador, filho1->data_type, filho1, filho2);
}