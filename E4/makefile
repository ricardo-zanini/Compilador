# ============ GRUPO R ==================
# Bernardo Cobalchini Zietolie - 00550164
# Ricardo Zanini de Costa - 00344523

# ============ VARIÁVEIS ===========

BISON = bison
FLEX = flex
CC = gcc
CFLAGS = -Wall -Werror -fsanitize=address

# --- Arquivos de Código Gerados ---
BISON_FILE = parser.y
FLEX_FILE = scanner.l
BISON_OUTPUT = parser.tab.c
BISON_HEADER = parser.tab.h
FLEX_OUTPUT = lex.yy.c
FLEX_OBJECT = lex.yy.o

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
MAIN_OBJECT = $(MAIN_SOURCE:.c=.o)
MANUAL_OBJECTS = $(ASD_SOURCE:.c=.o) $(TABELA_SOURCE:.c=.o) $(ERROS_SOURCE:.c=.o) $(SEMANTICA_SOURCE:.c=.o)
GENERATED_OBJECTS = $(BISON_FILE:.y=.tab.o) $(FLEX_OBJECT)
OBJECTS = $(GENERATED_OBJECTS) $(MANUAL_OBJECTS) $(MAIN_OBJECT)

# --- Executável ---
EXEC = etapa4
SELF = makefile

# Alvos que não são arquivos
.PHONY: all clean tgz

# Alvo Padrão
all: $(EXEC)

# ============== BISON ==============
$(BISON_OUTPUT) $(BISON_HEADER): $(BISON_FILE)
	$(BISON) -d $(BISON_FILE)

# =============== FLEX ==============
$(FLEX_OUTPUT): $(FLEX_FILE) $(BISON_HEADER)
	$(FLEX) $(FLEX_FILE)

# ============ COMPILAÇÃO ============

# --- Regras para objetos gerados ---
$(GENERATED_OBJECTS): $(BISON_HEADER) $(ASD_HEADER) $(TABELA_HEADER) $(ERROS_HEADER) $(SEMANTICA_HEADER) $(HEADERS)
$(GENERATED_OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- Regras para objetos manuais ---
$(ASD_OBJECT): $(ASD_SOURCE) $(ASD_HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

$(TABELA_OBJECT): $(TABELA_SOURCE) $(TABELA_HEADER) $(BISON_HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

$(ERROS_OBJECT): $(ERROS_SOURCE) $(ERROS_HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

$(SEMANTICA_OBJECT): $(SEMANTICA_SOURCE) $(SEMANTICA_HEADER) $(BISON_HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

$(MAIN_OBJECT): $(MAIN_SOURCE) $(ASD_HEADER) $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# --- Regra de Linkagem Final ---
$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) -lfl

# ============== CLEAN ==============

clean:
	rm -f $(EXEC) $(BISON_OUTPUT) $(BISON_HEADER) $(FLEX_OUTPUT) $(OBJECTS)

# ============== COMPACTAR ==============

tgz: clean
	tar -czvf $(EXEC).tgz $(SEMANTICA_SOURCE) $(SEMANTICA_HEADER) $(ERROS_SOURCE) $(ERROS_HEADER) $(TABELA_SOURCE) $(TABELA_HEADER) $(ASD_SOURCE) $(ASD_HEADER) $(HEADERS) $(MAIN_SOURCE) $(SELF) $(BISON_FILE) $(FLEX_FILE)

# --> "It makes your life easier" <--