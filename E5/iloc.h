#ifndef _ILOC_H_
#define _ILOC_H_

/* ============================================== */
/* ============== ESTRUTURAS ILOC =============== */
/* ============================================== */

// Natureza do símbolo na tabela
typedef enum {
    ORD_NORM = 0,
    ORD_INV
} OrdemConcatenacao;

typedef enum {
    OPERAND_REGISTER,   // Registrador (r0, r1, rfp, etc)
    OPERAND_IMMEDIATE,  // Valor imediato (constante numérica)
    OPERAND_LABEL       // Rótulo (L0, L1, etc)
} TipoOperando;

typedef struct operando_iloc {
    TipoOperando tipo;
    union {
        char *reg;      // Nome do registrador
        int imediato;   // Valor imediato
        char *rotulo;   // Nome do rótulo
    } valor;
} OperandoILOC;

typedef struct operacao_iloc {
    char *opcode;                   // Código da operação (add, sub, load, etc)
    OperandoILOC *operandos_fonte;  // Array de operandos fonte
    int num_fonte;                  // Número de operandos fonte
    OperandoILOC *operandos_alvo;   // Array de operandos alvo
    int num_alvo;                   // Número de operandos alvo
    char *rotulo;                   // Rótulo opcional desta instrução
    struct operacao_iloc *proximo;  // Próxima operação na lista
} OperacaoILOC;

/* Estrutura para lista de operações ILOC */
typedef struct lista_iloc {
    OperacaoILOC *primeira;
    OperacaoILOC *ultima;
} ListaILOC;

/* ============================================== */
/* =========== FUNÇÕES DE GERADORES ============= */
/* ============================================== */

/* Gera um novo nome de registrador temporário */
char* gerar_temporario();

/* Gera um novo nome de rótulo */
char* gerar_rotulo();

/* ============================================== */
/* ======== FUNÇÕES DE CRIAÇÃO DE OPERANDOS ===== */
/* ============================================== */

/* Cria um operando do tipo registrador */
OperandoILOC* criar_operando_registrador(const char *nome_reg);

/* Cria um operando do tipo imediato */
OperandoILOC* criar_operando_imediato(int valor);

/* Cria um operando do tipo rótulo */
OperandoILOC* criar_operando_rotulo(const char *nome_rotulo);

/* Libera um operando */
void liberar_operando(OperandoILOC *op);

/* ============================================== */
/* ======= FUNÇÕES DE CRIAÇÃO DE OPERAÇÕES ===== */
/* ============================================== */

/* Cria uma operação ILOC básica */
OperacaoILOC* criar_operacao(const char *opcode, 
                             OperandoILOC **fonte, int num_fonte,
                             OperandoILOC **alvo, int num_alvo);

/* Cria operação com rótulo */
OperacaoILOC* criar_operacao_com_rotulo(const char *rotulo, const char *opcode,
                                        OperandoILOC **fonte, int num_fonte,
                                        OperandoILOC **alvo, int num_alvo);

/* Libera uma operação */
void liberar_operacao(OperacaoILOC *op);

/* ============================================== */
/* ======== FUNÇÕES DE LISTA DE OPERAÇÕES ====== */
/* ============================================== */

/* Cria uma lista vazia */
ListaILOC* criar_lista_iloc();

/* Adiciona operação ao final da lista */
void adicionar_operacao(ListaILOC *lista, OperacaoILOC *op);

/* Adiciona operação ao inicio da lista */
void adicionar_operacao_ini(ListaILOC *lista, OperacaoILOC *op);

/* Concatena duas listas (lista2 é adicionada ao final de lista1) */
/* se ordem_inv == 1, então lista1 é adicionada ao fim de lista2*/
ListaILOC* concatenar_listas(ListaILOC *lista1, ListaILOC *lista2, OrdemConcatenacao ordem_inv);

/* Libera toda a lista */
void liberar_lista_iloc(ListaILOC *lista);

/* Imprime a lista de operações ILOC */
void imprimir_codigo_iloc(ListaILOC *lista);

/* ============================================== */
/* ========= FUNÇÕES AUXILIARES DE GERAÇÃO ===== */
/* ============================================== */

/* Cria operação loadI (carrega imediato em registrador) */
OperacaoILOC* criar_loadI(int valor, const char *reg_destino);

/* Cria operação load (carrega de memória) */
OperacaoILOC* criar_load(const char *reg_origem, const char *reg_destino);

/* Cria operação loadAI (carrega com offset) */
OperacaoILOC* criar_loadAI(const char *reg_base, int offset, const char *reg_destino);

/* Cria operação store (armazena em memória) */
OperacaoILOC* criar_store(const char *reg_origem, const char *reg_destino);

/* Cria operação storeAI (armazena com offset) */
OperacaoILOC* criar_storeAI(const char *reg_origem, const char *reg_base, int offset);

/* Cria operação aritmética binária (add, sub, mult, div) */
OperacaoILOC* criar_aritmetica(const char *opcode, const char *reg1, const char *reg2, const char *reg_dest);

/* Cria operação aritmética com imediato */
OperacaoILOC* criar_aritmetica_imediata(const char *opcode, const char *reg, int imediato, const char *reg_dest);

OperacaoILOC* criar_rsubI(int valor, const char *reg_origem, const char *reg_destino);

/* Cria operação de comparação */
OperacaoILOC* criar_comparacao(const char *opcode, const char *reg1, const char *reg2, const char *reg_dest);

/* Cria operação de salto condicional (cbr) */
OperacaoILOC* criar_cbr(const char *reg_condicao, const char *rotulo_true, const char *rotulo_false);

/* Cria operação de salto incondicional (jumpI) */
OperacaoILOC* criar_jumpI(const char *rotulo);

OperacaoILOC* criar_i2i(const char *reg_origem, const char *reg_destino);

/* Cria operação nop */
OperacaoILOC* criar_nop();

/* Cria operação nop com rótulo */
OperacaoILOC* criar_nop_com_rotulo(const char *rotulo);

#endif // _ILOC_H_