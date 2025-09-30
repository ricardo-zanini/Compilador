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
MAIN = main.c
EXEC = etapa3

all: $(EXEC)

# ============== BISON ==============

$(BISON_OUTPUT) $(BISON_HEADER): $(BISON_FILE)
	$(BISON) -d $(BISON_FILE)

# =============== FLEX ==============

$(FLEX_OUTPUT): $(FLEX_FILE) $(BISON_HEADER)
	$(FLEX) $(FLEX_FILE)

# ============ COMPILAÇÃO ============

$(BISON_OBJECT): $(BISON_OUTPUT)
	$(CC) $(CFLAGS) -c $(BISON_OUTPUT)

$(FLEX_OBJECT): $(FLEX_OUTPUT) $(BISON_HEADER)
	$(CC) $(CFLAGS) -c $(FLEX_OUTPUT)

$(ASD_OBJECT): $(ASD_SOURCE) $(ASD_HEADER)
	$(CC) $(CFLAGS) -c $(ASD_SOURCE)

$(EXEC): $(BISON_OBJECT) $(FLEX_OBJECT) $(ASD_OBJECT) $(MAIN)
	$(CC) $(CFLAGS) -o $(EXEC) $(BISON_OBJECT) $(FLEX_OBJECT) $(ASD_OBJECT) $(MAIN) -lfl

# ============== CLEAN ==============

clean:
	rm -f $(EXEC) $(BISON_OUTPUT) $(BISON_HEADER) $(FLEX_OUTPUT) $(BISON_OBJECT) $(FLEX_OBJECT) $(ASD_OBJECT)

tgz: clean
	tar -czvf $(EXEC).tgz *

# --> "It makes your life easier" <--