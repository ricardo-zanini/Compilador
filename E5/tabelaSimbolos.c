#include <stdio.h>
#include <stdlib.h>
#include "tabelaSimbolos.h"
#include "erros.h"

// Funções auxiliares para erros
extern void yyerror (char const *mensagem);
extern void yyerror_semantico(char const *mensagem, int num_linha);

/* Cria e empilha um novo escopo */
void stack_push(EscopoPilha **pilha) {
    EscopoPilha *novo_escopo = (EscopoPilha*) malloc(sizeof(EscopoPilha));
    novo_escopo->tabela_atual = (TabelaSimbolos*) malloc(sizeof(TabelaSimbolos));
    novo_escopo->tabela_atual->hash_table = NULL; // uthash inicializa com NULL
    novo_escopo->escopo_pai = *pilha; // Aponta para o escopo anterior
    novo_escopo->funcao_atual = NULL;

    if (*pilha != NULL) {
        // Herda o tipo de retorno do escopo pai
        novo_escopo->tipo_retorno = (*pilha)->tipo_retorno;
    } else {
        // Escopo global, não tem tipo de retorno
        novo_escopo->tipo_retorno = TIPO_INDEF;
    }

    *pilha = novo_escopo; // Atualiza o topo da pilha
}

/* Remove (desempilha) o escopo do topo da pilha e libera toda a memória associada a ele (a tabela hash e todas as suas entradas) */
void stack_pop(EscopoPilha **pilha) 
{
    // Verifica se a pilha já está vazia
    if (*pilha == NULL) {
        return;
    }

    // Guarda um ponteiro para o escopo do topo (que será removido)
    EscopoPilha *escopo_a_remover = *pilha;

    // "Desempilha" - faz a cabeça da pilha apontar para o escopo pai
    *pilha = (*pilha)->escopo_pai;

    // Prepara-se para iterar e destruir a tabela hash
    Simbolo *entrada_atual, *tmp;
    TabelaSimbolos *tabela = escopo_a_remover->tabela_atual;

    // Itera sobre CADA entrada na tabela hash
    HASH_ITER(hh, tabela->hash_table, entrada_atual, tmp) {
        
        // Remove a entrada da tabela hash
        HASH_DEL(tabela->hash_table, entrada_atual);

        // Libera a memória da chave (que foi duplicada com strdup)
        free(entrada_atual->chave);

        // Libera a memória do ValorLexico (que veio do scanner)
        free(entrada_atual->valor_lexico->valor_token); // strdup do yytext
        free(entrada_atual->valor_lexico);              // malloc do ValorLexico

        // Se for uma função, libera sua lista de argumentos
        if (entrada_atual->natureza == NAT_FUNCAO) {
            ArgLista *arg_atual = entrada_atual->argumentos;
            while (arg_atual != NULL) {
                ArgLista *arg_temp = arg_atual;
                arg_atual = arg_atual->next;
                free(arg_temp->nome);
                free(arg_temp);
            }
        }

        // Libera a própria estrutura da entrada
        free(entrada_atual);
    }

    // Libera o contêiner da tabela
    free(escopo_a_remover->tabela_atual);

    // Libera o nó da pilha de escopo
    free(escopo_a_remover);
}

/* Insere um símbolo na tabela do topo (escopo atual)  */
void symbol_insert(EscopoPilha *pilha, char *chave, Simbolo *entrada) {
    if (pilha == NULL || pilha->tabela_atual == NULL) {
        yyerror("Erro fatal: Pilha de escopo não inicializada.");
        exit(1); // Erro interno
    }

    /* Verifica se já foi declarado (no escopo atual ou nos escopos pais) */
    Simbolo *entrada_existente = symbol_lookup_local(pilha, chave);

    if (entrada_existente != NULL) {
        // Erro: Dupla declaração 
        report_error_declared(entrada->valor_lexico, entrada_existente);
    } else {
        /* Se não foi declarado, insere no escopo atual */
        HASH_ADD_KEYPTR(hh, pilha->tabela_atual->hash_table, 
                        entrada->chave, strlen(entrada->chave), entrada);
    }
    
}

/* Busca um símbolo em todos os escopos, do topo à base */
Simbolo* symbol_lookup(EscopoPilha *pilha, char *chave) {
    EscopoPilha *escopo_atual = pilha;
    Simbolo *entrada_encontrada = NULL;

    while (escopo_atual != NULL) {
        HASH_FIND_STR(escopo_atual->tabela_atual->hash_table, chave, entrada_encontrada);
        if (entrada_encontrada != NULL) {
            return entrada_encontrada;
        }
        escopo_atual = escopo_atual->escopo_pai; // Desce na pilha 
    }

    return NULL; // Não encontrou em nenhum escopo
}

/* Busca um símbolo apenas no escopo do topo */
Simbolo* symbol_lookup_local(EscopoPilha *pilha, char *chave) {
    if (pilha == NULL || pilha->tabela_atual == NULL) {
        return NULL;
    }

    Simbolo *entrada_encontrada = NULL;
    HASH_FIND_STR(pilha->tabela_atual->hash_table, chave, entrada_encontrada);
    return entrada_encontrada;
}

/* Função auxiliar para duplicar o ValorLexico */
ValorLexico* duplicar_valor_lexico(ValorLexico *lex) {
    if (lex == NULL) return NULL;

    // Aloca memória para a nova estrutura
    ValorLexico *copia = (ValorLexico*) malloc(sizeof(ValorLexico));
    if (copia == NULL) {
        yyerror("Erro fatal: Falha ao alocar memória para cópia do token.");
        exit(1); // Erro crítico
    }

    // Copia os dados
    copia->num_linha = lex->num_linha;
    copia->tipo_token = lex->tipo_token;
    
    // Aloca memória e copia a string do valor do token
    copia->valor_token = strdup(lex->valor_token);
    
    return copia;
}

/* Cria uma nova entrada na tabela de símbolos para uma VARIÁVEL. */
Simbolo* create_entry_var(char *chave, TipoDados tipo, ValorLexico *lex) {
    Simbolo *nova_entrada = (Simbolo*) malloc(sizeof(Simbolo));
    nova_entrada->chave = strdup(chave);
    nova_entrada->natureza = NAT_IDENTIFICADOR;
    nova_entrada->tipo_dado = tipo;
    nova_entrada->valor_lexico = duplicar_valor_lexico(lex); // Armazena uma cópia profunda, não o ponteiro original
    nova_entrada->argumentos = NULL;
    nova_entrada->num_argumentos = 0;
    nova_entrada->is_global = 1;
    nova_entrada->deslocamento = 0;
    return nova_entrada;
}

/* Cria uma nova entrada na tabela de símbolos para uma FUNÇÃO. */
Simbolo* create_entry_fun(char *chave, TipoDados tipo_retorno, ValorLexico *lex) 
{
    // Aloca memória para a nova entrada
    Simbolo *nova_entrada = (Simbolo*) malloc(sizeof(Simbolo));

    // Preenche os campos com os dados da função
    nova_entrada->chave = strdup(chave);
    nova_entrada->natureza = NAT_FUNCAO;
    nova_entrada->tipo_dado = tipo_retorno;
    nova_entrada->valor_lexico = duplicar_valor_lexico(lex); // Armazena uma cópia profunda, não o ponteiro original
    
    // Inicializa os campos específicos de função
    nova_entrada->argumentos = NULL;    // A lista de argumentos começa vazia
    nova_entrada->num_argumentos = 0;   // Nenhum argumento ainda

    // Limpar o handle do uthash. Boa prática antes de HASH_ADD.
    memset(&(nova_entrada->hh), 0, sizeof(UT_hash_handle));

    return nova_entrada;
}

/* Libera uma lista de argumentos (ArgLista) */
void free_arg_list(ArgLista* list) {
    ArgLista* curr = list;
    while(curr) {
        ArgLista* temp = curr;
        curr = curr->next;
        free(temp->nome);
        free(temp);
    }
}