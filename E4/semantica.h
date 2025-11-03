#ifndef _SEMANTICA_H_
#define _SEMANTICA_H_

#include "asd.h"
#include "tipos.h"
#include "parser.tab.h"
#include "tabelaSimbolos.h"
#include "erros.h"
#include <stdio.h>

/* ============================================================================================ */
/* ======================== PROTÓTIPOS DAS FUNÇÕES AUXILIARES PARA A AST ====================== */
/* ============================================================================================ */
asd_tree_t* criar_no_folha(ValorLexico* token, TipoDados tipo);
asd_tree_t* criar_no_unario(const char* op_label, TipoDados tipo, asd_tree_t* filho);
asd_tree_t* criar_no_binario(const char* op_label, TipoDados tipo, asd_tree_t* filho1, asd_tree_t* filho2);
void free_token(ValorLexico* token);

/* ================================================================================= */
/* ======================== PROTÓTIPOS DAS FUNÇÕES SEMÂNTICAS ====================== */
/* ================================================================================= */

/* Gerenciamento de escopo */
void semantica_push_scope();
void semantica_pop_scope();
void semantica_pop_scope_error();

/* Funções para tratar da semântica das funções */
void semantica_funcao_declaracao(ValorLexico* ident, TipoDados tipo);
asd_tree_t* semantica_funcao_definicao(ValorLexico* ident, TipoDados tipo, ArgLista* lista_param, asd_tree_t* corpo);

/* Função para tratar da semântica dos parâmetros de funções */
ArgLista* semantica_param(ValorLexico* ident, TipoDados tipo);

/* Função para tratar da semântica dos comandos de atribuição */
asd_tree_t* semantica_comando_atrib(ValorLexico* ident, asd_tree_t* exp);

/* Função para tratar da semântica das chamadas de função */
asd_tree_t* semantica_chamada_func(ValorLexico* ident, asd_tree_t* lista_arg);

/* Função para tratar da semântica dos comandos de retorno de função */
asd_tree_t* semantica_comando_ret(asd_tree_t* exp, TipoDados tipo);

/* Função para tratar da semântica dos comandos condicionais */
asd_tree_t* semantica_condicional(asd_tree_t* exp, asd_tree_t* bloco_if, asd_tree_t* bloco_else);

/* Função para tratar da semântica das declarações de variáveis globais */
asd_tree_t* semantica_declaracao_variavel_no_ini(ValorLexico* ident, TipoDados tipo);

/* Função para tratar da semântica das declarações de variáveis */
asd_tree_t* semantica_declaracao_variavel(ValorLexico* ident, TipoDados tipo, asd_tree_t* atrib);

/* Função para tratar da semântica dos identificadores/variáveis */
asd_tree_t* semantica_identificador_variavel(ValorLexico* ident);

/* Função para tratar da semântica das expressões binárias, verificando tipos e criando nó na árvore */
asd_tree_t* semantica_expressoes_binarias(char* operador, asd_tree_t* filho1, asd_tree_t* filho2, int num_linha);

#endif // _SEMANTICA_H_