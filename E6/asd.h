#ifndef _ASD_H_
#define _ASD_H_

#include "tipos.h"
#include "iloc.h"

typedef struct asd_tree {
  char *label;
  int number_of_children;
  TipoDados data_type;
  int num_linha;
  struct asd_tree **children;

  char *temp;         // Ex: "r1", "r2". O registrador que guarda o valor deste nó.
  ListaILOC *codigo;  // A lista de instruções ILOC gerada por este nó e seus filhos.
  
  struct asd_tree *filho_1;
  struct asd_tree *filho_2;
} asd_tree_t;

/*
 * Função asd_new, cria um nó sem filhos com o label informado.
 */
asd_tree_t *asd_new(const char *label, TipoDados data_type, int num_linha, char *temp, ListaILOC *codigo);

/*
 * Função asd_tree, libera recursivamente o nó e seus filhos.
 */
void asd_free(asd_tree_t *tree);

/*
 * Função asd_add_child, adiciona child como filho de tree.
 */
void asd_add_child(asd_tree_t *tree, asd_tree_t *child, OrdemConcatenacao ordem_inv);

/*
 * Função asd_detach_children, libera o array de filhos de um nó,
 * mas não os filhos em si. Define o número de filhos como 0.
 */
void asd_detach_children(asd_tree_t *tree);

/*
 * Função asd_print, imprime recursivamente a árvore.
 */
void asd_print(asd_tree_t *tree);

/*
 * Função asd_print_graphviz, idem, em formato DOT
 */
void asd_print_graphviz (asd_tree_t *tree);
#endif //_ASD_H_
