#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembly.h"
#include "iloc.h"
#include "semantica.h"

/* Mapeamento para Registradores */
#define QTD_REG_FISICOS 4

/* Lista de registradores "Callee-Saved" (Seguros entre chamadas) */
const char *reg_fisicos[QTD_REG_FISICOS] = {
    "%ebx", "%r12d", "%r13d", "%r14d"
};

/* Variável global para rastrear o maior registro temporário usado (para alocar stack) */
int max_reg_idx = 0;

/* Variáveis de Estado para Peephole */
int cache_offset = -9999;
char *cache_base = NULL;
int cache_valido = 0; // 0 = inválido, 1 = válido

/* ================================================================= */
/* ==================== FUNÇÕES AUXILIARES ========================= */
/* ================================================================= */

void invalidar_cache() {
    cache_valido = 0;
    cache_offset = -9999;
    cache_base = NULL;
}

/* Imprimir o local de uma variável (Reg ou Pilha) */
void imprimir_local_var(int offset) {

    int indice = offset / 4;

    /* Tenta alocar registrador. Se acabar, usa pilha. */
    if (indice < QTD_REG_FISICOS) {
        printf("%s", reg_fisicos[indice]);
    } else {
        /* Se acabou os registradores, usa a pilha (Spill) */
        printf("-%d(%%rbp)", offset + 4);
    }
}

/* Extrai o ID numérico de um registrador. Ex: "r5" -> 5 */
int obter_id_reg(char *reg) {
    if (!reg || reg[0] != 'r') return -1;
    /* Registradores especiais */
    if (strcmp(reg, "rfp") == 0) return -2;
    if (strcmp(reg, "rbss") == 0) return -3;
    if (strcmp(reg, "rsp") == 0) return -4;
    return atoi(reg + 1); /* Pula o 'r' */
}

/* Calcula quanto espaço de pilha é necessário varrendo todo o código */
void calc_tamanho_pilha(ListaILOC *lista) {
    if (!lista) return;
    OperacaoILOC *op = lista->primeira;
    while(op) {
        /* Verifica fontes */
        for(int i=0; i<op->num_fonte; i++) {
            if(op->operandos_fonte[i].tipo == OPERAND_REGISTER) {
                int id = obter_id_reg(op->operandos_fonte[i].valor.reg);
                if(id > max_reg_idx) max_reg_idx = id;
            }
        }
        /* Verifica alvos */
        for(int i=0; i<op->num_alvo; i++) {
            if(op->operandos_alvo[i].tipo == OPERAND_REGISTER) {
                int id = obter_id_reg(op->operandos_alvo[i].valor.reg);
                if(id > max_reg_idx) max_reg_idx = id;
            }
        }
        op = op->proximo;
    }
}

/* Imprime um operando traduzido para x86 */
void imprimir_operando(OperandoILOC op) {
    if (op.tipo == OPERAND_IMMEDIATE) {
        printf("$%d", op.valor.imediato);
    } 
    else if (op.tipo == OPERAND_LABEL) {
        printf(".%s", op.valor.rotulo);
    } 
    else if (op.tipo == OPERAND_REGISTER) {
        int id = obter_id_reg(op.valor.reg);
        if (id == -2) { /* rfp */
            printf("%%rbp");
        } else if (id == -3) { /* rbss - tratado no contexto da instrução */
            printf("%%rip"); 
        } else {
            /* r0 (temp) vira offset 0, r1 vira offset 4... */
            /* obter_id_reg retorna 0, 1, 2... Multiplicamos por 4. */
            imprimir_local_var(id * 4);
        }
    }
}

/* ================================================================= */
/* ==================== TRADUTOR DE INSTRUÇÕES ===================== */
/* ================================================================= */

void traduzir_instrucao(OperacaoILOC *op) {

    /* Imprime Rótulos (Labels) */
    if (op->rotulo) {

        /* Se encontrar um rótulo, o fluxo de execução é incerto.
           Não podemos garantir o valor de %eax vindo da instrução anterior. */
        invalidar_cache();

        /* Se for label de função, precisa ser global */
        if (strcmp(op->rotulo, "main") == 0) {
            printf("\t.globl\tmain\n");
            printf("\t.type\tmain, @function\n");
            printf("%s:\n", op->rotulo);
        } else{
            printf(".%s:\n", op->rotulo);
        }
        
        /* Prólogo da função (se o rótulo não for um L genérico gerado pelo cbr/jump) */
        /* Simplificação: assumindo que labels de função são nomes, e labels de fluxo são L*/
        if (op->rotulo[0] != 'L' || strcmp(op->rotulo, "main") == 0) {
            printf("\tpushq\t%%rbp\n");
            printf("\tmovq\t%%rsp, %%rbp\n");

            /* Salvar registradores Callee-Saved */
            printf("\tpushq\t%%rbx\n");
            printf("\tpushq\t%%r12\n");
            printf("\tpushq\t%%r13\n");
            printf("\tpushq\t%%r14\n");

            /* Aloca espaço na pilha (alinhado a 16 bytes) */
            int size = (max_reg_idx + 1) * 4;
            if (size % 16 != 0) size += (16 - (size % 16));
            if (size > 0) printf("\tsubq\t$%d, %%rsp\n", size);
        }
    }

    /* Flag para saber se a instrução atual mantem o cache válido */
    int preserva_cache = 0;

    switch (op->opcode) {

        /* Movimentação de Dados */
        case OP_LOADI:
            /* loadI C => rX  --> movl $C, MEM */
            printf("\tmovl\t"); imprimir_operando(op->operandos_fonte[0]); printf(", ");
            imprimir_operando(op->operandos_alvo[0]); printf("\n");
            break;

        case OP_I2I:
            /* i2i rA => rB --> movl rA, %eax; movl %eax, rB */
            printf("\tmovl\t"); imprimir_operando(op->operandos_fonte[0]); printf(", %%eax\n");
            printf("\tmovl\t%%eax, "); imprimir_operando(op->operandos_alvo[0]); printf("\n");
            break;
        
        case OP_LOADAI:
            {
                /* loadAI base, offset => dst */
                char *base = op->operandos_fonte[0].valor.reg;
                int offset = op->operandos_fonte[1].valor.imediato;
                char *nome_global = op->operandos_fonte[1].nome_aux;

                /* Otimização Peephole */
                int pular_load = 0;
                if (cache_valido && cache_base && 
                    strcmp(base, cache_base) == 0 && offset == cache_offset) {
                    pular_load = 1;
                }

                if (!pular_load) {
                    if (strcmp(base, "rbss") == 0) {
                        /* Zero lookup: Usa o nome direto da struct */
                        if (nome_global) {
                            printf("\tmovl\t%s(%%rip), %%eax\n", nome_global);
                        } else {
                            /* Fallback: usa offset numérico se necessário */
                            printf("\tmovl\tdados_globais+%d(%%rip), %%eax\n", offset);
                        }
                    } else {
                        printf("\tmovl\t");
                        imprimir_local_var(offset);
                        printf(", %%eax\n");
                    }
                }

                printf("\tmovl\t%%eax, ");
                imprimir_operando(op->operandos_alvo[0]);
                printf("\n");

                /* Atualiza o cache (após um Load, %eax == memória) */
                cache_base = base;
                cache_offset = offset;
                cache_valido = 1;
                preserva_cache = 1;
            }
            break;

        case OP_STOREAI: 
            {
                char *base = op->operandos_alvo[0].valor.reg;
                int offset = op->operandos_alvo[1].valor.imediato;
                char *nome_global = op->operandos_alvo[1].nome_aux;

                printf("\tmovl\t");
                imprimir_operando(op->operandos_fonte[0]);
                printf(", %%eax\n");
                
                if (strcmp(base, "rbss") == 0) {
                    /* Zero lookup: Usa o nome direto da struct */
                    if (nome_global) {
                        printf("\tmovl\t%%eax, %s(%%rip)\n", nome_global);
                    } else {
                        /* Fallback: usa offset numérico se necessário */
                        printf("\tmovl\t%%eax, dados_globais+%d(%%rip)\n", offset);
                    }
                } else {
                    printf("\tmovl\t%%eax, ");
                    imprimir_local_var(offset);
                    printf("\n");
                }

                /* Atualiza o cache (após um Store, %eax == memória) */
                cache_base = base;
                cache_offset = offset;
                cache_valido = 1;
                preserva_cache = 1;
            }
            break;


        /* Aritmética e Lógica */
        case OP_ADD:
        case OP_SUB:
        case OP_MULT:
        case OP_AND:
        case OP_OR:
        case OP_XOR:
            {
                const char *mnemonico;
                switch(op->opcode) {
                    case OP_ADD: mnemonico = "addl"; break;
                    case OP_SUB: mnemonico = "subl"; break;
                    case OP_MULT: mnemonico = "imull"; break;
                    case OP_AND: mnemonico = "andl"; break;
                    case OP_OR:  mnemonico = "orl"; break;
                    case OP_XOR: mnemonico = "xorl"; break;
                    default: mnemonico = "nop"; break;
                }

                printf("\tmovl\t"); imprimir_operando(op->operandos_fonte[0]); printf(", %%eax\n");
                printf("\t%s\t", mnemonico); imprimir_operando(op->operandos_fonte[1]); printf(", %%eax\n");
                printf("\tmovl\t%%eax, "); imprimir_operando(op->operandos_alvo[0]); printf("\n");
            }
            break;
        
        case OP_DIV:
            printf("\tmovl\t"); imprimir_operando(op->operandos_fonte[0]); printf(", %%eax\n");
            printf("\tcltd\n"); /* EAX -> EDX:EAX */
            printf("\tidivl\t"); imprimir_operando(op->operandos_fonte[1]); printf("\n");
            printf("\tmovl\t%%eax, "); imprimir_operando(op->operandos_alvo[0]); printf("\n");
            break;

        case OP_RSUBI: /* rsubI rA, c => rB (rB = c - rA) */
            printf("\tmovl\t"); imprimir_operando(op->operandos_fonte[1]); printf(", %%eax\n"); /* Carrega Imediato */
            printf("\tsubl\t"); imprimir_operando(op->operandos_fonte[0]); printf(", %%eax\n"); /* Subtrai Reg */
            printf("\tmovl\t%%eax, "); imprimir_operando(op->operandos_alvo[0]); printf("\n");
            break;


        /* Comparações */
        case OP_CMP_LT:
        case OP_CMP_LE:
        case OP_CMP_EQ:
        case OP_CMP_GE:
        case OP_CMP_GT:
        case OP_CMP_NE:
            {
                char *set_cc;
                switch(op->opcode) {
                    case OP_CMP_LT: set_cc = "setl"; break;
                    case OP_CMP_LE: set_cc = "setle"; break;
                    case OP_CMP_EQ: set_cc = "sete"; break;
                    case OP_CMP_GE: set_cc = "setge"; break;
                    case OP_CMP_GT: set_cc = "setg"; break;
                    case OP_CMP_NE: set_cc = "setne"; break;
                    default: set_cc = "sete"; break;
                }

                printf("\tmovl\t"); imprimir_operando(op->operandos_fonte[0]); printf(", %%eax\n");
                printf("\tcmpl\t"); imprimir_operando(op->operandos_fonte[1]); printf(", %%eax\n");
                printf("\t%s\t%%al\n", set_cc);
                printf("\tmovzbl\t%%al, %%eax\n");
                printf("\tmovl\t%%eax, "); imprimir_operando(op->operandos_alvo[0]); printf("\n");
            }
            break;


        /* Fluxo de Controle */
        case OP_JUMPI:
            printf("\tjmp\t.%s\n", op->operandos_alvo[0].valor.rotulo);
            break;

        case OP_CBR: /* cbr rCond -> Ltrue, Lfalse */
            printf("\tmovl\t"); imprimir_operando(op->operandos_fonte[0]); printf(", %%eax\n");
            printf("\tcmpl\t$0, %%eax\n");
            printf("\tjne\t.%s\n", op->operandos_alvo[0].valor.rotulo);
            printf("\tjmp\t.%s\n", op->operandos_alvo[1].valor.rotulo);
            break;


        /* Funções */
        case OP_RET:
            if (op->num_fonte > 0) {
                printf("\tmovl\t"); imprimir_operando(op->operandos_fonte[0]); printf(", %%eax\n");
            }

            /* Restaurar registradores (Ordem Inversa do Push) */
            printf("\tpopq\t%%r14\n");
            printf("\tpopq\t%%r13\n");
            printf("\tpopq\t%%r12\n");
            printf("\tpopq\t%%rbx\n");

            printf("\tleave\n");
            printf("\tret\n");
            break;

        default:
            /* Opcode não implementado ou ignorado */
            break;

    }

    /* Se a instrução altera %eax e não é um dos casos acima, invalida */
    if (!preserva_cache) {
        invalidar_cache();
    }
}

/* ================================================================= */
/* ======================== FUNÇÃO PRINCIPAL ======================= */
/* ================================================================= */

void gerar_assembly(asd_tree_t *arvore) {
    if (!arvore || !arvore->codigo) return;

    /* Cabeçalho Assembly */
    printf("\t.file\t\"programa.c\"\n");

    /* Gera as variáveis globais a partir da tabela de símbolos */
    printf("\t.data\n");
    gerar_segmento_dados();
    printf("\t.text\n");

    /* Calcula tamanho da pilha baseado no maior registrador usado em todo o programa */
    calc_tamanho_pilha(arvore->codigo);

    /* Itera sobre as instruções */
    OperacaoILOC *op = arvore->codigo->primeira;
    while(op) {
        traduzir_instrucao(op);
        op = op->proximo;
    }

    printf("\t.ident\t\"Compilador E6\"\n");
    printf("\t.section\t.note.GNU-stack,\"\",@progbits\n");
}