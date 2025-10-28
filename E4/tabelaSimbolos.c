#include <stdio.h>
#include <stdlib.h>
#include "tabelaSimbolos.h"
#include "erros.h"

// Função auxiliar (extern) para erros
extern void yyerror (char const *mensagem);

/* Cria e empilha um novo escopo */
void stack_push(EscopoPilha **pilha) {
    EscopoPilha *novo_escopo = (EscopoPilha*) malloc(sizeof(EscopoPilha));
    novo_escopo->tabela_atual = (TabelaSimbolos*) malloc(sizeof(TabelaSimbolos));
    novo_escopo->tabela_atual->hash_table = NULL; // uthash inicializa com NULL
    novo_escopo->escopo_pai = *pilha; // Aponta para o escopo anterior
    *pilha = novo_escopo; // Atualiza o topo da pilha
}

/* Remove (desempilha) o escopo do topo da pilha e libera toda a memória associada a ele (a tabela hash e todas as suas entradas) */
void stack_pop(EscopoPilha **pilha) 
{
    // Verifica se a pilha já está vazia
    if (*pilha == NULL) {
        return; // Nada a fazer
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

    Simbolo *entrada_existente = NULL;
    HASH_FIND_STR(pilha->tabela_atual->hash_table, chave, entrada_existente);

    if (entrada_existente != NULL) {
        // Erro: Dupla declaração 
        char msg_erro[256];
        sprintf(msg_erro, "Identificador '%s' já declarado neste escopo (linha %d)", 
                chave, entrada_existente->valor_lexico->num_linha);
        yyerror(msg_erro);
        exit(ERR_DECLARED);
    } else {
        HASH_ADD_KEYPTR(hh, pilha->tabela_atual->hash_table, 
                        entrada->chave, strlen(entrada->chave), entrada);
    }
}

/* Busca um símbolo em todos os escopos, do topo à base  */
Simbolo* symbol_lookup(EscopoPilha *pilha, char *chave) {
    EscopoPilha *escopo_atual = pilha;
    Simbolo *entrada_encontrada = NULL;

    while (escopo_atual != NULL) {
        HASH_FIND_STR(escopo_atual->tabela_atual->hash_table, chave, entrada_encontrada);
        if (entrada_encontrada != NULL) {
            return entrada_encontrada; // Encontrou!
        }
        escopo_atual = escopo_atual->escopo_pai; // Desce na pilha 
    }

    return NULL; // Não encontrou em nenhum escopo
}

/* Cria uma nova entrada na tabela de símbolos para uma VARIÁVEL. */
Simbolo* create_entry_var(char *chave, TipoDados tipo, ValorLexico *lex) {
    Simbolo *nova_entrada = (Simbolo*) malloc(sizeof(Simbolo));
    nova_entrada->chave = strdup(chave);
    nova_entrada->natureza = NAT_IDENTIFICADOR;
    nova_entrada->tipo_dado = tipo;
    nova_entrada->valor_lexico = lex;
    nova_entrada->argumentos = NULL;
    nova_entrada->num_argumentos = 0;
    return nova_entrada;
}

/* Cria uma nova entrada na tabela de símbolos para uma FUNÇÃO. */
Simbolo* create_entry_fun(char *chave, TipoDados tipo_retorno, ValorLexico *lex) 
{
    // Aloca memória para a nova entrada
    Simbolo *nova_entrada = (Simbolo*) malloc(sizeof(Simbolo));

    // Preenche os campos com os dados da função
    nova_entrada->chave = strdup(chave); // Copia a chave (nome da função)
    nova_entrada->natureza = NAT_FUNCAO;
    nova_entrada->tipo_dado = tipo_retorno; // Tipo de retorno da função
    nova_entrada->valor_lexico = lex;       // Transfere a posse do token
    
    // Inicializa os campos específicos de função
    nova_entrada->argumentos = NULL;    // A lista de argumentos começa vazia
    nova_entrada->num_argumentos = 0;   // Nenhum argumento ainda

    // Limpar o handle do uthash. Boa prática antes de HASH_ADD.
    memset(&(nova_entrada->hh), 0, sizeof(UT_hash_handle));

    return nova_entrada;
}