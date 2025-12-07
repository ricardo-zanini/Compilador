/*
* ============ GRUPO R ==================
* Bernardo Cobalchini Zietolie - 00550164
* Ricardo Zanini de Costa - 00344523
 */

#include <stdio.h>
#include "asd.h"
#include "assembly.h"
#include "semantica.h"

extern int yyparse(void);
extern int yylex_destroy(void);
asd_tree_t *arvore = NULL;

int main (int argc, char **argv)
{
  int ret = yyparse();
  // asd_print_graphviz(arvore); // Descomente para imprimir a árvore em .dot

  if (ret == 0 && arvore != NULL) {
      gerar_assembly(arvore);
      asd_free(arvore);
  }

  semantica_pop_scope();

  yylex_destroy();
  return ret;
}
