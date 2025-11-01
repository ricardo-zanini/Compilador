#ifndef _TABELA_H_
#define _TABELA_H_

#include "parser.tab.h"
#include "tipos.h"
#include "uthash.h" // Biblioteca que facilita a implementação da hash table

// Estrutura para armazenar argumentos de função
typedef struct arg_lista {
    TipoDados tipo;
    char* nome;
    struct arg_lista *next;
} ArgLista;

// Entrada da Tabela de Símbolos
typedef struct simbolo {
    char* chave;                // Chave única que identifica o símbolo
    NaturezaSimbolo natureza;   // Natureza (literal, identificador ou função)
    TipoDados tipo_dado;        // Tipo do dado do símbolo (int ou float)
    ValorLexico *valor_lexico;  // Dados do valor do token
    ArgLista *argumentos;       // Argumentos e seus tipos (no caso de funções)
    int num_argumentos;
    UT_hash_handle hh;          // Torna a struct "hasheável" pelo uthash
} Simbolo;

// Estrutura da Tabela de Símbolos
typedef struct tabela_simbolos {
    Simbolo *hash_table; // A hash table (uthash)
} TabelaSimbolos;

// Estrutura para a Pilha de Escopos
typedef struct escopo_pilha {
    TabelaSimbolos *tabela_atual;
    struct escopo_pilha *escopo_pai; // Link para o escopo anterior (a base da pilha)
    TipoDados tipo_retorno; // Qual o tipo de retorno compatível com o do escopo (função) atual
} EscopoPilha;


// Gerenciamento da Pilha
void stack_push(EscopoPilha **pilha);
void stack_pop(EscopoPilha **pilha);

// Inserção e Busca
void symbol_insert(EscopoPilha *pilha, char *chave, Simbolo *entrada);
Simbolo* symbol_lookup(EscopoPilha *pilha, char *chave);

/* Função auxiliar para duplicar o ValorLexico */
ValorLexico* duplicar_valor_lexico(ValorLexico *lex);

// Funções de criação de entradas (helpers)
Simbolo* create_entry_var(char *chave, TipoDados tipo, ValorLexico *lex);
Simbolo* create_entry_fun(char *chave, TipoDados tipo_retorno, ValorLexico *lex);

#endif //_TABELA_H_
