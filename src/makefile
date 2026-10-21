EXEC = parser
BISON_FILE = parser.y
FLEX_FILE  = scanner.l
BISON_C = parser.tab.c
BISON_H = parser.tab.h
FLEX_C  = lex.yy.c
HEADERS = tabela.h ast.h
SOURCES = tabela.c ast.c

CC     = gcc
# Adicionada a flag -g para facilitar o debug de memória com o valgrind
CFLAGS = -Wall -Wextra -g -std=gnu11

all: $(EXEC)

# Não precisa de -lfl: o scanner usa %option noyywrap e o main está no parser.y
$(EXEC): $(BISON_C) $(FLEX_C) $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(BISON_C) $(FLEX_C) $(SOURCES)

$(BISON_C) $(BISON_H): $(BISON_FILE)
	bison -d -Wall $(BISON_FILE)

$(FLEX_C): $(FLEX_FILE) $(BISON_H)
	flex $(FLEX_FILE)

# Os scripts ficam em ../tests (o makefile roda dentro de src/)
test: $(EXEC)
	../tests/run_tests.sh
	../tests/run_ast_tests.sh

clean:
	rm -f $(EXEC) $(BISON_C) $(BISON_H) $(FLEX_C)

.PHONY: all test clean