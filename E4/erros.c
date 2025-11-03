#include <stdio.h>
#include <stdlib.h>
#include "erros.h"

/* Função auxiliar para converter TipoDados em string */
static const char* tipo_para_string(TipoDados tipo) {
    switch (tipo) {
        case TIPO_INT: return "inteiro";
        case TIPO_DEC: return "decimal";
        default: return "indefinido";
    }
}

/* Função base yyerror que permite informar número de linha */
void yyerror_base(char const *mensagem, int num_linha) {
    printf("==================================================================\n");
    printf ("ERRO: Linha %i - [%s]\n", num_linha, mensagem);
    printf("==================================================================\n");
}

/* ====================================================== */
/* ============= ERROS DE DECLARAÇÃO ==================== */
/* ====================================================== */

void report_error_undeclared(ValorLexico *token) {
    char msg_erro[256];
    snprintf(msg_erro, 256, "Identificador '%s' não declarado", token->valor_token);
    yyerror_base(msg_erro, token->num_linha);
    exit(ERR_UNDECLARED);
}

void report_error_declared(ValorLexico *novo_token, Simbolo *simbolo_existente) {
    char msg_erro[256];
    snprintf(msg_erro, 256, "Identificador '%s' já declarado (na linha %d)", 
            novo_token->valor_token, simbolo_existente->valor_lexico->num_linha);
    yyerror_base(msg_erro, novo_token->num_linha);
    exit(ERR_DECLARED);
}

/* ================================================================== */
/* ============= ERROS DE USO DE IDENTIFICADORES ==================== */
/* ================================================================== */

void report_error_variable_as_function(ValorLexico *token) {
    char msg_erro[256];
    snprintf(msg_erro, 256, "Identificador '%s' dito variável usado como uma função", token->valor_token);
    yyerror_base(msg_erro, token->num_linha);
    exit(ERR_VARIABLE);
}

void report_error_function_as_variable(ValorLexico *token) {
    char msg_erro[256];
    snprintf(msg_erro, 256, "Identificador '%s' dito função usado como uma variável", token->valor_token);
    yyerror_base(msg_erro, token->num_linha);
    exit(ERR_FUNCTION);
}

/* ================================================ */
/* ============= ERROS DE TIPO ==================== */
/* ================================================ */

void report_error_wrong_type_binary_op(int num_linha, TipoDados tipo1, TipoDados tipo2, const char* op) {
     char msg_erro[256];
     snprintf(msg_erro, 256, "TIPOS NÃO COMPATÍVEIS: expressão '%s' %s '%s'",
             tipo_para_string(tipo1), op, tipo_para_string(tipo2));
     yyerror_base(msg_erro, num_linha);
     exit(ERR_WRONG_TYPE);
}

void report_error_wrong_type_assignment(ValorLexico *id_token, TipoDados id_tipo, TipoDados expr_tipo) {
    char msg_erro[256];
    snprintf(msg_erro, 256, "TIPOS NÃO COMPATÍVEIS: Atribuindo expressão do tipo '%s' ao identificador '%s' de tipo '%s'",
            tipo_para_string(expr_tipo),
            id_token->valor_token,
            tipo_para_string(id_tipo));
    yyerror_base(msg_erro, id_token->num_linha);
    exit(ERR_WRONG_TYPE);
}

void report_error_wrong_type_initialization(ValorLexico *id_token, TipoDados var_tipo, TipoDados atrib_tipo) {
    char msg_erro[256];
    snprintf(msg_erro, 256, "TIPOS NÃO COMPATÍVEIS: Variável '%s' de tipo '%s' inicializada com valor de tipo '%s'",
            id_token->valor_token, 
            tipo_para_string(var_tipo),
            tipo_para_string(atrib_tipo));
    yyerror_base(msg_erro, id_token->num_linha);
    exit(ERR_WRONG_TYPE);
}

void report_error_wrong_type_return_expr(int num_linha, TipoDados expr_tipo, TipoDados decl_tipo) {
    char msg_erro[256];
    snprintf(msg_erro, 256, "TIPOS NÃO COMPATÍVEIS: Tipo do valor de retorno ('%s') é diferente do tipo especificado ('%s')",
            tipo_para_string(expr_tipo),
            tipo_para_string(decl_tipo));
    yyerror_base(msg_erro, num_linha);
    exit(ERR_WRONG_TYPE);
}

void report_error_wrong_type_return_func(int num_linha, TipoDados decl_tipo, TipoDados func_tipo) {
    char msg_erro[256];
    snprintf(msg_erro, 256, "TIPOS NÃO COMPATÍVEIS: Tipo do retorno ('%s') não é compatível com o tipo de retorno da função ('%s')",
            tipo_para_string(decl_tipo),
            tipo_para_string(func_tipo));
    yyerror_base(msg_erro, num_linha);
    exit(ERR_WRONG_TYPE);
}

void report_error_wrong_type_if_else(int num_linha, TipoDados if_tipo, TipoDados else_tipo) {
    char msg_erro[256];
    snprintf(msg_erro, 256, "TIPOS NÃO COMPATÍVEIS: Bloco 'if' com tipo '%s' e bloco 'else' com tipo '%s'",
            tipo_para_string(if_tipo),
            tipo_para_string(else_tipo));
    yyerror_base(msg_erro, num_linha);
    exit(ERR_WRONG_TYPE);
}

/* ====================================================================== */
/* ============= ERROS DE ARGUMENTOS/PARÂMETROS DE FUNÇÃO ============= */
/* ====================================================================== */

void report_error_missing_args(ValorLexico *fun_token, int esperado, int recebido) {
    char msg_erro[256];
    snprintf(msg_erro, 256, "CHAMADA DE FUNÇÃO '%s': Faltam argumentos. Esperado: %d, Recebido: %d",
            fun_token->valor_token, esperado, recebido);
    yyerror_base(msg_erro, fun_token->num_linha);
    exit(ERR_MISSING_ARGS);
}

void report_error_excess_args(ValorLexico *fun_token, int esperado, int recebido) {
    char msg_erro[256];
    snprintf(msg_erro, 256, "CHAMADA DE FUNÇÃO '%s': Excesso de argumentos. Esperado: %d, Recebido: %d",
            fun_token->valor_token, esperado, recebido);
    yyerror_base(msg_erro, fun_token->num_linha);
    exit(ERR_EXCESS_ARGS);
}

void report_error_wrong_type_args(ValorLexico *fun_token, int arg_num, TipoDados esperado, TipoDados recebido) {
    char msg_erro[256];
    snprintf(msg_erro, 256, "CHAMADA DE FUNÇÃO '%s': Tipo incorreto para o argumento %d. Esperado: '%s', Recebido: '%s'",
            fun_token->valor_token,
            arg_num + 1, // Começa os indíces por 1 (ao invés de 0) para a melhor visualização do usuário
            tipo_para_string(esperado),
            tipo_para_string(recebido));
    yyerror_base(msg_erro, fun_token->num_linha);
    exit(ERR_WRONG_TYPE_ARGS);
}