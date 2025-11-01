#ifndef _ERROS_H_
#define _ERROS_H_

#include "tipos.h"
#include "parser.tab.h" // Para ValorLexico
#include "tabelaSimbolos.h" // Para Simbolo*

#define ERR_UNDECLARED       10 // Identificador não declarado
#define ERR_DECLARED         11 // Identificador já declarado
#define ERR_VARIABLE         20 // Identificador de variável sendo usado como função
#define ERR_FUNCTION         21 // Identificador de função sendo usado como variável
#define ERR_WRONG_TYPE       30 // Tipos incompatíveis
#define ERR_MISSING_ARGS     40 // Faltam argumentos na chamada de função
#define ERR_EXCESS_ARGS      41 // Sobram argumentos na chamada de função
#define ERR_WRONG_TYPE_ARGS  42 // Tipos de argumentos incompatíveis

/* Função base yyerror que permite informar número de linha */
void yyerror_base(char const *mensagem, int num_linha);

/* Verificação de Declarações */
void report_error_undeclared(ValorLexico *token);
void report_error_declared(ValorLexico *novo_token, Simbolo *simbolo_existente);

/* Uso Correto de Identificadores */
void report_error_variable_as_function(ValorLexico *token);
void report_error_function_as_variable(ValorLexico *token);

/* Verificação de Tipos */
void report_error_wrong_type_binary_op(int num_linha, TipoDados tipo1, TipoDados tipo2, const char* op);
void report_error_wrong_type_assignment(ValorLexico *id_token, TipoDados id_tipo, TipoDados expr_tipo);
void report_error_wrong_type_initialization(ValorLexico *id_token, TipoDados var_tipo, TipoDados atrib_tipo);
void report_error_wrong_type_return_expr(int num_linha, TipoDados expr_tipo, TipoDados decl_tipo);
void report_error_wrong_type_return_func(int num_linha, TipoDados decl_tipo, TipoDados func_tipo);
void report_error_wrong_type_if_else(int num_linha, TipoDados if_tipo, TipoDados else_tipo);

#endif // _ERROS_H_