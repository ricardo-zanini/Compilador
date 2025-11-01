# ============ GRUPO R ==================
# Bernardo Cobalchini Zietolie - 00550164
# Ricardo Zanini de Costa - 00344523

# ============ VARIÁVEIS ===========

BISON = bison
FLEX = flex
CC = gcc
CFLAGS = -Wall -Werror -fsanitize=address
BISON_FILE = parser.y
FLEX_FILE = scanner.l
BISON_OUTPUT = parser.tab.c
BISON_HEADER = parser.tab.h
BISON_OBJECT = parser.tab.o
FLEX_OUTPUT = lex.yy.c
FLEX_OBJECT = lex.yy.o
ASD_SOURCE = asd.c
ASD_HEADER = asd.h
ASD_OBJECT = asd.o
TABELA_SOURCE = tabelaSimbolos.c
TABELA_HEADER = tabelaSimbolos.h
TABELA_OBJECT = tabelaSimbolos.o
ERROS_HEADER = erros.h
ERROS_SOURCE = erros.c
ERROS_OBJECT = erros.o
HEADERS = uthash.h tipos.h
MAIN = main.c
EXEC = etapa4
SELF = makefile

all: $(EXEC)

# ============== BISON ==============

$(BISON_OUTPUT) $(BISON_HEADER): $(BISON_FILE)
	$(BISON) -d $(BISON_FILE)

# =============== FLEX ==============

$(FLEX_OUTPUT): $(FLEX_FILE) $(BISON_HEADER)
	$(FLEX) $(FLEX_FILE)

# ============ COMPILAÇÃO ============

$(BISON_OBJECT): $(BISON_OUTPUT) $(BISON_HEADER) $(ASD_HEADER) $(TABELA_HEADER) $(ERROS_HEADER) $(HEADERS)
	$(CC) $(CFLAGS) -c $(BISON_OUTPUT)

$(FLEX_OBJECT): $(FLEX_OUTPUT) $(BISON_HEADER)
	$(CC) $(CFLAGS) -c $(FLEX_OUTPUT)

$(ASD_OBJECT): $(ASD_SOURCE) $(ASD_HEADER)
	$(CC) $(CFLAGS) -c $(ASD_SOURCE)

$(TABELA_OBJECT): $(TABELA_SOURCE) $(TABELA_HEADER) $(BISON_HEADER)
	$(CC) $(CFLAGS) -c $(TABELA_SOURCE)

$(ERROS_OBJECT): $(ERROS_SOURCE) $(ERROS_HEADER)
	$(CC) $(CFLAGS) -c $(ERROS_SOURCE)

$(EXEC): $(BISON_OBJECT) $(FLEX_OBJECT) $(ASD_OBJECT) $(TABELA_OBJECT) $(ERROS_OBJECT) $(MAIN)
	$(CC) $(CFLAGS) -o $(EXEC) $(BISON_OBJECT) $(FLEX_OBJECT) $(ASD_OBJECT) $(TABELA_OBJECT) $(ERROS_OBJECT) $(MAIN) -lfl

# ============== CLEAN ==============

clean:
	rm -f $(EXEC) $(BISON_OUTPUT) $(BISON_HEADER) $(FLEX_OUTPUT) $(BISON_OBJECT) $(FLEX_OBJECT) $(ASD_OBJECT) $(TABELA_OBJECT) $(ERROS_OBJECT)

tgz: clean
	tar -czvf $(EXEC).tgz $(ERROS_SOURCE) $(TABELA_SOURCE) $(TABELA_HEADER) $(ASD_SOURCE) $(ASD_HEADER) $(HEADERS) $(MAIN) $(SELF) $(BISON_FILE) $(FLEX_FILE)

# --> "It makes your life easier" <--