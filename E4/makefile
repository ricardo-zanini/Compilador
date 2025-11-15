# ============ GRUPO R ==================
# Bernardo Cobalchini Zietolie - 00550164
# Ricardo Zanini de Costa - 00344523

# ============ VARIÁVEIS ===========

BISON = bison
FLEX = flex
CC = gcc
# Adicionamos -MMD -MP para gerar dependências (.d) automaticamente
CFLAGS = -Wall -Werror -fsanitize=address -MMD -MP

# --- Arquivos de Código Gerados ---
BISON_FILE = parser.y
FLEX_FILE = scanner.l
BISON_OUTPUT = parser.tab.c
BISON_HEADER = parser.tab.h
FLEX_OUTPUT = lex.yy.c

# --- Arquivos de Código Manuais ---
ASD_SOURCE = asd.c
TABELA_SOURCE = tabelaSimbolos.c
ERROS_SOURCE = erros.c
SEMANTICA_SOURCE = semantica.c
MAIN_SOURCE = main.c

# --- Cabeçalhos ---
ASD_HEADER = asd.h
TABELA_HEADER = tabelaSimbolos.h
ERROS_HEADER = erros.h
SEMANTICA_HEADER = semantica.h
HEADERS = uthash.h tipos.h

# --- Agrupamento de Objetos ---
MANUAL_OBJECTS = $(ASD_SOURCE:.c=.o) $(TABELA_SOURCE:.c=.o) $(ERROS_SOURCE:.c=.o) $(SEMANTICA_SOURCE:.c=.o)
GENERATED_OBJECTS = $(BISON_FILE:.y=.tab.o) $(FLEX_OUTPUT:.c=.o)
OBJECTS = $(GENERATED_OBJECTS) $(MANUAL_OBJECTS) $(MAIN_SOURCE:.c=.o)

# --- Arquivos de Dependência (.d) ---
DEPS = $(OBJECTS:.o=.d)

# --- Executável ---
EXEC = etapa4
SELF = makefile

# Alvos que não são arquivos
.PHONY: all clean tgz

# Alvo Padrão
all: $(EXEC)
	@echo "Compilação concluída ✅"

# ============== BISON ==============
# Regra para gerar parser.tab.c e parser.tab.h
$(BISON_OUTPUT) $(BISON_HEADER): $(BISON_FILE)
	@$(BISON) -d $(BISON_FILE)

# =============== FLEX ==============
# Regra para gerar lex.yy.c (depende do header do Bison)
$(FLEX_OUTPUT): $(FLEX_FILE) $(BISON_HEADER)
	@$(FLEX) $(FLEX_FILE)

# ============ COMPILAÇÃO ============

$(GENERATED_OBJECTS): $(BISON_HEADER)

# REGRA UNIVERSAL DE COMPILAÇÃO
%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

# --- Regra de Linkagem Final ---
$(EXEC): $(OBJECTS)
	@$(CC) $(CFLAGS) -o $@ $(OBJECTS)

# ============== CLEAN ==============
clean:
	@rm -f $(EXEC) $(BISON_OUTPUT) $(BISON_HEADER) $(FLEX_OUTPUT) $(OBJECTS) $(DEPS)
	@echo "Limpeza concluída 🗑️"

# ============== COMPACTAR ==============
tgz: clean
	@tar -czvf $(EXEC).tgz $(SEMANTICA_SOURCE) $(SEMANTICA_HEADER) $(ERROS_SOURCE) $(ERROS_HEADER) $(TABELA_SOURCE) $(TABELA_HEADER) $(ASD_SOURCE) $(ASD_HEADER) $(HEADERS) $(MAIN_SOURCE) $(SELF) $(BISON_FILE) $(FLEX_FILE)
	@echo "Compactação concluída 📦"

# Inclui os arquivos de dependência .d gerados
-include $(DEPS)

# --> "It makes your life easier" <--