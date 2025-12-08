#include "iloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Contadores globais para geração de nomes de rótulos e temporários */
static int count_temp = 1000;
static int count_rotulos = 0;

/* Função auxiliar para debug/impressão */
const char* nome_opcode(Opcode op) {
    switch (op) {
        case OP_NOP: return "nop";
        case OP_ADD: return "add";
        case OP_SUB: return "sub";
        case OP_MULT: return "mult";
        case OP_DIV: return "div";
        case OP_RSUBI: return "rsubi";
        case OP_AND: return "and";
        case OP_OR: return "or";
        case OP_XOR: return "xor";
        case OP_LOAD: return "load";
        case OP_LOADAI: return "loadAI";
        case OP_LOADI: return "loadI";
        case OP_STORE: return "store";
        case OP_STOREAI: return "storeAI";
        case OP_I2I: return "i2i";
        case OP_CMP_LT: return "cmp_LT";
        case OP_CMP_LE: return "cmp_LE";
        case OP_CMP_EQ: return "cmp_EQ";
        case OP_CMP_GE: return "cmp_GE";
        case OP_CMP_GT: return "cmp_GT";
        case OP_CMP_NE: return "cmp_NE";
        case OP_CBR: return "cbr";
        case OP_JUMP: return "jump";
        case OP_JUMPI: return "jumpI";
        case OP_RET: return "retorna";
        default: return "unknown";
    }
}

/* ============================================== */
/* =========== FUNÇÕES DE GERADORES ============= */
/* ============================================== */

char* gerar_temporario() {
    char* temp = malloc(20 * sizeof(char));
    sprintf(temp, "r%d", count_temp++);
    return temp;
}

char* gerar_rotulo() {
    char* rotulo = malloc(20 * sizeof(char));
    sprintf(rotulo, "L%d", count_rotulos++); // nome do temporário é L + número
    return rotulo;
}

/* ============================================== */
/* ======= FUNÇÕES DE CRIAÇÃO DE OPERANDOS ===== */
/* ============================================== */

OperandoILOC* criar_operando_registrador(const char *nome_reg) {
    OperandoILOC *op = (OperandoILOC*)malloc(sizeof(OperandoILOC));
    op->tipo = OPERAND_REGISTER;
    op->valor.reg = strdup(nome_reg);
    op->nome_aux = NULL;
    return op;
}

OperandoILOC* criar_operando_imediato(int valor) {
    OperandoILOC *op = (OperandoILOC*)malloc(sizeof(OperandoILOC));
    op->tipo = OPERAND_IMMEDIATE;
    op->valor.imediato = valor;
    op->nome_aux = NULL;
    return op;
}

OperandoILOC* criar_operando_rotulo(const char *nome_rotulo) {
    OperandoILOC *op = (OperandoILOC*)malloc(sizeof(OperandoILOC));
    op->tipo = OPERAND_LABEL;
    op->valor.rotulo = strdup(nome_rotulo);
    op->nome_aux = NULL;
    return op;
}

void liberar_operando(OperandoILOC *op) {
    if (!op) return;
    
    if (op->tipo == OPERAND_REGISTER) {
        free(op->valor.reg);
    } else if (op->tipo == OPERAND_LABEL) {
        free(op->valor.rotulo);
    }

    if (op->nome_aux != NULL) free(op->nome_aux);
    free(op);
}

/* ============================================== */
/* ======= FUNÇÕES DE CRIAÇÃO DE OPERAÇÕES ===== */
/* ============================================== */

OperacaoILOC* criar_operacao(
    Opcode opcode,
    OperandoILOC **fonte, int num_fonte,
    OperandoILOC **alvo, int num_alvo
) {
    OperacaoILOC *op = (OperacaoILOC*)malloc(sizeof(OperacaoILOC));
    op->opcode = opcode;
    op->rotulo = NULL;
    op->proximo = NULL;
    
    op->num_fonte = num_fonte;
    // Loop copiando operandos fonte
    if (num_fonte > 0) {
        op->operandos_fonte = (OperandoILOC*)malloc(num_fonte * sizeof(OperandoILOC));
        for (int i = 0; i < num_fonte; i++) {
            // Copia o tipo
            op->operandos_fonte[i].tipo = fonte[i]->tipo;
            
            // Copia profundamente o valor baseado no tipo
            if (fonte[i]->tipo == OPERAND_REGISTER) {
                op->operandos_fonte[i].valor.reg = strdup(fonte[i]->valor.reg);
            } else if (fonte[i]->tipo == OPERAND_LABEL) {
                op->operandos_fonte[i].valor.rotulo = strdup(fonte[i]->valor.rotulo);
            } else {
                op->operandos_fonte[i].valor.imediato = fonte[i]->valor.imediato;
            }

            // Cópia do nome_aux se existir
            if (fonte[i]->nome_aux) op->operandos_fonte[i].nome_aux = strdup(fonte[i]->nome_aux);
            else op->operandos_fonte[i].nome_aux = NULL;
        }
    // Se não achou atributos fonte
    } else {
        op->operandos_fonte = NULL;
    }
    
    op->num_alvo = num_alvo;
    // Loop copiando operandos alvo
    if (num_alvo > 0) {
        op->operandos_alvo = (OperandoILOC*)malloc(num_alvo * sizeof(OperandoILOC));
        for (int i = 0; i < num_alvo; i++) {
            // Copia o tipo
            op->operandos_alvo[i].tipo = alvo[i]->tipo;
            
            // Copia profundamente o valor baseado no tipo
            if (alvo[i]->tipo == OPERAND_REGISTER) {
                op->operandos_alvo[i].valor.reg = strdup(alvo[i]->valor.reg);
            } else if (alvo[i]->tipo == OPERAND_LABEL) {
                op->operandos_alvo[i].valor.rotulo = strdup(alvo[i]->valor.rotulo);
            } else {
                op->operandos_alvo[i].valor.imediato = alvo[i]->valor.imediato;
            }
            
            // Cópia do nome_aux se existir
            if (alvo[i]->nome_aux) op->operandos_alvo[i].nome_aux = strdup(alvo[i]->nome_aux);
            else op->operandos_alvo[i].nome_aux = NULL;
        }
    // Se não achou atributos alvo
    } else {
        op->operandos_alvo = NULL;
    }
    
    return op;
}

OperacaoILOC* criar_operacao_com_rotulo(
    const char *rotulo, Opcode opcode,
    OperandoILOC **fonte, int num_fonte,
    OperandoILOC **alvo, int num_alvo
) {
    // Apenas se faz a chamada de criar_operacao com a adicao do rotulo posteriormente
    OperacaoILOC *op = criar_operacao(opcode, fonte, num_fonte, alvo, num_alvo);
    op->rotulo = strdup(rotulo);
    return op;
}

void liberar_operacao(OperacaoILOC *op) {
    // Se não há operação retorna
    if (!op) return;

    if (op->rotulo) free(op->rotulo);
    
    // percorre operandos fonte fazendo limpeza
    for (int i = 0; i < op->num_fonte; i++) {
        if (op->operandos_fonte[i].tipo == OPERAND_REGISTER) {
            free(op->operandos_fonte[i].valor.reg);
        } else if (op->operandos_fonte[i].tipo == OPERAND_LABEL) {
            free(op->operandos_fonte[i].valor.rotulo);
        }

        if (op->operandos_fonte[i].nome_aux != NULL) {
            free(op->operandos_fonte[i].nome_aux);
        }
    }
    if (op->operandos_fonte) free(op->operandos_fonte);
    
    // percorre operandos alvo fazendo a limpeza
    for (int i = 0; i < op->num_alvo; i++) {
        if (op->operandos_alvo[i].tipo == OPERAND_REGISTER) {
            free(op->operandos_alvo[i].valor.reg);
        } else if (op->operandos_alvo[i].tipo == OPERAND_LABEL) {
            free(op->operandos_alvo[i].valor.rotulo);
        }

        if (op->operandos_alvo[i].nome_aux != NULL) {
            free(op->operandos_alvo[i].nome_aux);
        }
    }
    if (op->operandos_alvo) free(op->operandos_alvo);
    
    free(op);
}

/* ============================================== */
/* ======== FUNÇÕES DE LISTA DE OPERAÇÕES ====== */
/* ============================================== */

ListaILOC* criar_lista_iloc() {
    ListaILOC *lista = (ListaILOC*)malloc(sizeof(ListaILOC));
    lista->primeira = NULL;
    lista->ultima = NULL;
    return lista;
}

void adicionar_operacao(ListaILOC *lista, OperacaoILOC *op) {
    if (!lista || !op) return;
    
    // Se for primeira inserção é primeiro e último elemento
    if (lista->primeira == NULL) {
        lista->primeira = op;
        lista->ultima = op;
    // Muda fim da lista e adiciona ponteiro para operação adicionada no ultimo elemento anterior
    } else {
        lista->ultima->proximo = op;
        lista->ultima = op;
    }
}
void adicionar_operacao_ini(ListaILOC *lista, OperacaoILOC *op) {
    if (!lista || !op) return;

    op->proximo = lista->primeira; 

    if (lista->primeira == NULL) {
        lista->ultima = op;
    }
    lista->primeira = op;
}

ListaILOC* concatenar_listas(ListaILOC *lista1, ListaILOC *lista2, OrdemConcatenacao ordem_inv) {
    // Casos de ausência de uma ou ambas as listas
    if (!lista1 && !lista2) return NULL;
    if (!lista1) return lista2;
    if (!lista2) return lista1;
    
    // Se lista1 está vazia, apenas copia os ponteiros de lista2
    if (lista1->primeira == NULL) {
        lista1->primeira = lista2->primeira;
        lista1->ultima = lista2->ultima;
    } 
    // Se lista2 não está vazia, conecta ao final de lista1
    else if (lista2->primeira != NULL) {
        // lista2 -> lista1
        if(ordem_inv == 1){
            lista2->ultima->proximo = lista1->primeira; // A cauda da L2 aponta para a cabeça da L1
            lista1->primeira = lista2->primeira;        // A nova cabeça da L1 passa a ser a cabeça da L2

        //lista1 -> lista2
        }else{
            lista1->ultima->proximo = lista2->primeira;
            lista1->ultima = lista2->ultima;
        }
    }
    
    // Libera apenas a estrutura da lista2
    free(lista2);
    return lista1;
}

void liberar_lista_iloc(ListaILOC *lista) {
    if (!lista) return;
    
    OperacaoILOC *atual = lista->primeira;
    while (atual) {
        OperacaoILOC *prox = atual->proximo;
        liberar_operacao(atual);
        atual = prox;
    }
    
    free(lista);
}

void imprimir_codigo_iloc(ListaILOC *lista) {
    if (!lista) return;
    
    OperacaoILOC *op = lista->primeira;
    while (op) {
        // Imprime o rótulo se existir
        if (op->rotulo) {
            printf("%s: ", op->rotulo);
        }
        
        // Imprime o opcode
        printf("%s", nome_opcode(op->opcode));
        
        // Imprime operandos fonte
        if (op->num_fonte > 0) {
            printf(" ");
            for (int i = 0; i < op->num_fonte; i++) {
                if (i > 0) printf(", ");
                
                switch (op->operandos_fonte[i].tipo) {
                    case OPERAND_REGISTER:
                        printf("%s", op->operandos_fonte[i].valor.reg);
                        break;
                    case OPERAND_IMMEDIATE:
                        printf("%d", op->operandos_fonte[i].valor.imediato);
                        break;
                    case OPERAND_LABEL:
                        printf("%s", op->operandos_fonte[i].valor.rotulo);
                        break;
                }
            }
        }
        
        // Imprime separador e operandos alvo
        if (op->num_alvo > 0) {
            // Verifica se é operação de controle de fluxo (usa ->)
            if (op->opcode == OP_CBR || op->opcode == OP_JUMP || op->opcode == OP_JUMPI){
                printf(" -> ");
            } else {
                printf(" => ");
            }
            
            for (int i = 0; i < op->num_alvo; i++) {
                if (i > 0) printf(", ");
                
                switch (op->operandos_alvo[i].tipo) {
                    case OPERAND_REGISTER:
                        printf("%s", op->operandos_alvo[i].valor.reg);
                        break;
                    case OPERAND_IMMEDIATE:
                        printf("%d", op->operandos_alvo[i].valor.imediato);
                        break;
                    case OPERAND_LABEL:
                        printf("%s", op->operandos_alvo[i].valor.rotulo);
                        break;
                }
            }
        }
        
        printf("\n");
        op = op->proximo;
    }
}

/* ============================================== */
/* ========= FUNÇÕES AUXILIARES DE GERAÇÃO ===== */
/* ============================================== */

OperacaoILOC* criar_loadI(int valor, const char *reg_destino) {
    OperandoILOC *fonte[1] = {criar_operando_imediato(valor)};
    OperandoILOC *alvo[1] = {criar_operando_registrador(reg_destino)};
    
    OperacaoILOC *op = criar_operacao(OP_LOADI, fonte, 1, alvo, 1);
    
    liberar_operando(fonte[0]);
    liberar_operando(alvo[0]);
    
    return op;
}

OperacaoILOC* criar_load(const char *reg_origem, const char *reg_destino) {
    OperandoILOC *fonte[1] = {criar_operando_registrador(reg_origem)};
    OperandoILOC *alvo[1] = {criar_operando_registrador(reg_destino)};
    
    OperacaoILOC *op = criar_operacao(OP_LOAD, fonte, 1, alvo, 1);
    
    liberar_operando(fonte[0]);
    liberar_operando(alvo[0]);
    
    return op;
}

OperacaoILOC* criar_loadAI(const char *reg_base, int offset, const char *reg_destino, const char *nome_global) {
    OperandoILOC *fonte[2] = {
        criar_operando_registrador(reg_base),
        criar_operando_imediato(offset)
    };

    // Anexa o nome ao operando imediato (o offset)
    if (nome_global) fonte[1]->nome_aux = strdup(nome_global);

    OperandoILOC *alvo[1] = {criar_operando_registrador(reg_destino)};
    
    OperacaoILOC *op = criar_operacao(OP_LOADAI, fonte, 2, alvo, 1);
    
    liberar_operando(fonte[0]);
    liberar_operando(fonte[1]);
    liberar_operando(alvo[0]);
    
    return op;
}

OperacaoILOC* criar_store(const char *reg_origem, const char *reg_destino) {
    OperandoILOC *fonte[1] = {criar_operando_registrador(reg_origem)};
    OperandoILOC *alvo[1] = {criar_operando_registrador(reg_destino)};
    
    OperacaoILOC *op = criar_operacao(OP_STORE, fonte, 1, alvo, 1);
    
    liberar_operando(fonte[0]);
    liberar_operando(alvo[0]);
    
    return op;
}

OperacaoILOC* criar_storeAI(const char *reg_origem, const char *reg_base, int offset, const char *nome_global) {
    OperandoILOC *fonte[1] = {criar_operando_registrador(reg_origem)};
    OperandoILOC *alvo[2] = {
        criar_operando_registrador(reg_base),
        criar_operando_imediato(offset)
    };

    // Anexa o nome ao operando imediato (o offset)
    if (nome_global) alvo[1]->nome_aux = strdup(nome_global);
    
    OperacaoILOC *op = criar_operacao(OP_STOREAI, fonte, 1, alvo, 2);
    
    liberar_operando(fonte[0]);
    liberar_operando(alvo[0]);
    liberar_operando(alvo[1]);
    
    return op;
}

OperacaoILOC* criar_aritmetica(Opcode opcode, const char *reg1, const char *reg2, const char *reg_dest) {
    OperandoILOC *fonte[2] = {
        criar_operando_registrador(reg1),
        criar_operando_registrador(reg2)
    };
    OperandoILOC *alvo[1] = {criar_operando_registrador(reg_dest)};
    
    OperacaoILOC *op = criar_operacao(opcode, fonte, 2, alvo, 1);
    
    liberar_operando(fonte[0]);
    liberar_operando(fonte[1]);
    liberar_operando(alvo[0]);
    
    return op;
}

OperacaoILOC* criar_aritmetica_imediata(Opcode opcode, const char *reg, int imediato, const char *reg_dest) {
    OperandoILOC *fonte[2] = {
        criar_operando_registrador(reg),
        criar_operando_imediato(imediato)
    };
    OperandoILOC *alvo[1] = {criar_operando_registrador(reg_dest)};
    
    OperacaoILOC *op = criar_operacao(opcode, fonte, 2, alvo, 1);
    
    liberar_operando(fonte[0]);
    liberar_operando(fonte[1]);
    liberar_operando(alvo[0]);
    
    return op;
}

OperacaoILOC* criar_rsubI(int valor, const char *reg_origem, const char *reg_destino) {
    OperandoILOC *fonte[2] = {
        criar_operando_registrador(reg_origem),
        criar_operando_imediato(valor)
    };
    OperandoILOC *alvo[1] = {criar_operando_registrador(reg_destino)};
    
    OperacaoILOC *op = criar_operacao(OP_RSUBI, fonte, 2, alvo, 1);
    
    liberar_operando(fonte[0]);
    liberar_operando(fonte[1]);
    liberar_operando(alvo[0]);
    
    return op;
}

OperacaoILOC* criar_comparacao(Opcode opcode, const char *reg1, const char *reg2, const char *reg_dest) {
    OperandoILOC *fonte[2] = {
        criar_operando_registrador(reg1),
        criar_operando_registrador(reg2)
    };
    OperandoILOC *alvo[1] = {criar_operando_registrador(reg_dest)};
    
    OperacaoILOC *op = criar_operacao(opcode, fonte, 2, alvo, 1);
    
    liberar_operando(fonte[0]);
    liberar_operando(fonte[1]);
    liberar_operando(alvo[0]);
    
    return op;
}

OperacaoILOC* criar_cbr(const char *reg_condicao, const char *rotulo_true, const char *rotulo_false) {
    OperandoILOC *fonte[1] = {criar_operando_registrador(reg_condicao)};
    OperandoILOC *alvo[2] = {
        criar_operando_rotulo(rotulo_true),
        criar_operando_rotulo(rotulo_false)
    };
    
    OperacaoILOC *op = criar_operacao(OP_CBR, fonte, 1, alvo, 2);
    
    liberar_operando(fonte[0]);
    liberar_operando(alvo[0]);
    liberar_operando(alvo[1]);
    
    return op;
}

OperacaoILOC* criar_jumpI(const char *rotulo) {
    OperandoILOC *alvo[1] = {criar_operando_rotulo(rotulo)};
    
    OperacaoILOC *op = criar_operacao(OP_JUMPI, NULL, 0, alvo, 1);
    
    liberar_operando(alvo[0]);
    
    return op;
}

OperacaoILOC* criar_i2i(const char *reg_origem, const char *reg_destino) {
    OperandoILOC *fonte[1] = {criar_operando_registrador(reg_origem)};
    OperandoILOC *alvo[1] = {criar_operando_registrador(reg_destino)};
    
    OperacaoILOC *op = criar_operacao(OP_I2I, fonte, 1, alvo, 1);
    
    liberar_operando(fonte[0]);
    liberar_operando(alvo[0]);
    
    return op;
}

OperacaoILOC* criar_nop() {
    return criar_operacao(OP_NOP, NULL, 0, NULL, 0);
}

OperacaoILOC* criar_nop_com_rotulo(const char *rotulo) {
    return criar_operacao_com_rotulo(rotulo, OP_NOP, NULL, 0, NULL, 0);
}