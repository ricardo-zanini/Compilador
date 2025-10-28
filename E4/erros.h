#ifndef _ERROS_H_
#define _ERROS_H_

#define ERR_UNDECLARED       10 // Identificador não declarado
#define ERR_DECLARED         11 // Identificador já declarado
#define ERR_VARIABLE         20 // Identificador de variável sendo usado como função
#define ERR_FUNCTION         21 // Identificador de função sendo usado como variável
#define ERR_WRONG_TYPE       30 // Tipos incompatíveis
#define ERR_MISSING_ARGS     40 // Faltam argumentos na chamada de função
#define ERR_EXCESS_ARGS      41 // Sobram argumentos na chamada de função
#define ERR_WRONG_TYPE_ARGS  42 // Tipos de argumentos incompatíveis

#endif // _ERROS_H_