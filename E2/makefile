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
MAIN = main.c
EXEC = etapa2

all: $(EXEC)

# ============== BISON ==============

$(BISON_OUTPUT) $(BISON_HEADER): $(BISON_FILE)
	$(BISON) -d $(BISON_FILE)

# =============== FLEX ==============

$(FLEX_OUTPUT): $(FLEX_FILE)
	$(FLEX) $(FLEX_FILE)

# ============ COMPILAÇÃO ============

$(BISON_OBJECT): $(BISON_OUTPUT)
	$(CC) $(CFLAGS) -c $(BISON_OUTPUT)

$(FLEX_OBJECT): $(FLEX_OUTPUT)
	$(CC) $(CFLAGS) -c $(FLEX_OUTPUT)

$(EXEC): $(BISON_OBJECT) $(FLEX_OBJECT) $(MAIN)
	$(CC) $(CFLAGS) -o $(EXEC) $(BISON_OBJECT) $(FLEX_OBJECT) $(MAIN) -lfl

# ============== CLEAN ==============

clean:
	rm -f $(EXEC) $(BISON_OUTPUT) $(BISON_HEADER) $(FLEX_OUTPUT) $(BISON_OBJECT) $(FLEX_OBJECT)

tgz: clean
	tar -czvf $(EXEC).tgz *

# --> "It makes your life easier" <--