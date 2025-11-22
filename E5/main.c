/*
* ============ GRUPO R ==================
* Bernardo Cobalchini Zietolie - 00550164
* Ricardo Zanini de Costa - 00344523
 */

#include <stdio.h>
#include "asd.h"
#include "iloc.h"

extern int yyparse(void);
extern int yylex_destroy(void);
asd_tree_t *arvore = NULL;

int main (int argc, char **argv)
{
  int ret = yyparse();
  // asd_print_graphviz(arvore); // Descomente para imprimir a árvore em .dot
  imprimir_codigo_iloc(arvore->codigo);
  asd_free(arvore);
  yylex_destroy();
  return ret;
}
