// common.h
#ifndef COMMON_H
#define COMMON_H

/* Estrutura para armazenar o valor léxico dos tokens */
typedef struct valor_lexico_struct {
    int num_linha;
    int tipo_token;
    char* valor_token;
} ValorLexico;

#endif