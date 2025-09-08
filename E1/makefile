CC=gcc
CFLAGS=-I.
DEPS = tokens.h
LEX = flex

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

etapa1: main.o lex.yy.o 
	$(CC) -o etapa1 main.o lex.yy.o

lex.yy.o: lex.yy.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

lex.yy.c: scanner.l
	$(LEX) $<