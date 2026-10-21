EXEC = parser
BISON_FILE = parser.y
FLEX_FILE  = scanner.l
BISON_C = parser.tab.c
BISON_H = parser.tab.h
FLEX_C  = lex.yy.c
AST_C   = ast.c
HEADERS = tabela.h ast.h

CC     = gcc
# Adicionada a flag -g para facilitar o debug de memória com o valgrind
CFLAGS = -Wall -Wextra -g -std=gnu11

all: $(EXEC)

# Adicionado $(AST_C) nas dependências e no comando de compilação
$(EXEC): $(BISON_C) $(FLEX_C) $(AST_C) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(BISON_C) $(FLEX_C) $(AST_C)

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
