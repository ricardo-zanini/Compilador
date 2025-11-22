#!/bin/bash

# --- CONFIGURAÇÃO ---
COMPILER="./etapa4"
TEST_DIR="testes"
PREFIX="qwe"
ERROR_HEADER="erros.h" # Caminho para seu arquivo de cabeçalho de erros

# --- 🎨 CORES ---
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
NC="\033[0m" # Sem Cor

# --- VERIFICAÇÃO INICIAL ---
if [ ! -f "$COMPILER" ]; then
    echo -e "${RED}Erro: Executável '$COMPILER' não encontrado.${NC}"
    exit 1
fi
if [ ! -d "$TEST_DIR" ]; then
    echo -e "${RED}Erro: Diretório de testes '$TEST_DIR' não encontrado.${NC}"
    exit 1
fi
if [ ! -f "$ERROR_HEADER" ]; then
    echo -e "${RED}Erro: Arquivo de cabeçalho '$ERROR_HEADER' não encontrado.${NC}"
    exit 1
fi

# --- 1. MAPA DE ERROS (Lido dinamicamente de erros.h) ---
declare -A ERROR_MAP
echo "Lendo códigos de erro de $ERROR_HEADER..."

# Lê o arquivo .h, procura por linhas "#define ERR_", remove \r (Windows)
# e extrai o NOME ($2) e o VALOR ($3)
while read -r error_name error_code; do
    if [[ -n "$error_name" && -n "$error_code" ]]; then
        # Remove \r do código de erro, caso exista
        error_code_cleaned=$(echo "$error_code" | tr -d '\r')
        ERROR_MAP[$error_name]=$error_code_cleaned
        echo "  -> Mapeando $error_name para $error_code_cleaned"
    fi
done < <(grep "^#define ERR_" "$ERROR_HEADER" | awk '{print $2, $3}')

# --- 2. PRÉ-PROCESSAMENTO DOS TESTES ---
declare -a EXPECTED_RESULTS
echo "Pré-processando arquivos de teste..."

for i in {0..82}
do
    num=$(printf "%02d" $i)
    test_file_path="${TEST_DIR}/${PREFIX}${num}"

    if [ ! -f "$test_file_path" ]; then
        EXPECTED_RESULTS[$i]=-1 # Código para "AUSENTE"
    else
        first_line=$(head -n 1 "$test_file_path")
        
        if [[ "$first_line" == "//"* ]]; then
            # É um erro esperado
            # Limpa o comentário e remove espaços/quebras de linha
            error_name=$(echo "$first_line" | sed 's#//##' | tr -d '[:space:]\r')
            error_code=${ERROR_MAP[$error_name]}
            
            if [ -z "$error_code" ]; then
                echo -e "${YELLOW}Aviso: Erro desconhecido '$error_name' em ${test_file_path}${NC}"
                EXPECTED_RESULTS[$i]=-2 # Código para "ERRO NO TESTE"
            else
                EXPECTED_RESULTS[$i]=$error_code
            fi
        else
            # Sucesso esperado
            EXPECTED_RESULTS[$i]=0
        fi
    fi
done

# --- 3. EXECUÇÃO E SUMÁRIO ---
passed_count=0
failed_count=0
missing_count=0
total_count=83

echo "Iniciando testes (Total: $total_count)..."
echo "----------------------------------------------------"

for i in {0..82}
do
    num=$(printf "%02d" $i)
    test_file_name="${PREFIX}${num}"
    test_file_path="${TEST_DIR}/${test_file_name}"
    expected_code=${EXPECTED_RESULTS[$i]}

    if [ "$expected_code" -eq -1 ]; then
        echo -e "[${test_file_name}] ${YELLOW}AUSENTE: Arquivo não encontrado${NC}"
        ((missing_count++))
    
    elif [ "$expected_code" -eq -2 ]; then
        echo -e "[${test_file_name}] ${RED}FALHA DE TESTE: Comentário de erro inválido ('$test_file_name')${NC}"
        ((failed_count++))
    
    else
        $COMPILER < "$test_file_path" > /dev/null 2>&1
        actual_code=$?
        
        if [ "$actual_code" -eq "$expected_code" ]; then
            echo -e "[${test_file_name}] ${GREEN}PASSOU${NC} (Esperado: $expected_code, Recebido: $actual_code)"
            ((passed_count++))
        else
            echo -e "[${test_file_name}] ${RED}FALHOU${NC} (Esperado: $expected_code, Recebido: $actual_code)"
            ((failed_count++))
        fi
    fi
done

# --- SUMÁRIO FINAL ---
echo "----------------------------------------------------"
echo "Sumário dos Testes:"
echo -e "  ${GREEN}Passaram: $passed_count${NC}"
echo -e "  ${RED}Falharam: $failed_count${NC}"
echo -e "  ${YELLOW}Ausentes: $missing_count${NC}"
echo "  Total: $total_count"
echo "----------------------------------------------------"

if [ "$failed_count" -gt 0 ] || [ "$missing_count" -gt 0 ]; then
    exit 1
else
    exit 0
fi