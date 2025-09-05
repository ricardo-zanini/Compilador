# ============ VARIÁVEIS ===========

BISON = bison
FLEX = flex
CC = gcc
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

# ============ COMPILAÇÃO ===========

$(BISON_OBJECT): $(FLEX_OUTPUT) $(BISON_OUTPUT)
	$(CC) -c $(FLEX_OUTPUT) $(BISON_OUTPUT)

$(EXEC): $(BISON_OBJECT) $(FLEX_OBJECT)
	$(CC) -o $(EXEC) $(BISON_OBJECT) $(FLEX_OBJECT) -lfl

# ============== CLEAN ==============

clean:
	rm -f $(EXEC) $(BISON_OUTPUT) $(BISON_HEADER) $(FLEX_OUTPUT) $(BISON_OBJECT) $(FLEX_OBJECT)

tgz: clean
	tar -czvf $(EXEC).tgz *

# --> "It makes your life easier" <--