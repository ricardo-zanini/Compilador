#ifndef _TIPOS_H_
#define _TIPOS_H_

// Tipos de dados da linguagem
typedef enum {
    TIPO_INDEF = 0,
    TIPO_INT,
    TIPO_DEC
} TipoDados;

// Natureza do símbolo na tabela
typedef enum {
    NAT_INDEF = 0,
    NAT_LITERAL,
    NAT_IDENTIFICADOR,
    NAT_FUNCAO
} NaturezaSimbolo;

#endif // _TIPOS_H_