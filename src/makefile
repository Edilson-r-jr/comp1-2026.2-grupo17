EXEC = parser
BISON_FILE = parser.y
FLEX_FILE  = scanner.l
BISON_C = parser.tab.c
BISON_H = parser.tab.h
FLEX_C  = lex.yy.c
HEADERS = tabela.h

CC     = gcc
CFLAGS = -Wall -Wextra -std=gnu11

all: $(EXEC)

# Não precisa de -lfl: o scanner usa %option noyywrap e o main está no parser.y
$(EXEC): $(BISON_C) $(FLEX_C) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(BISON_C) $(FLEX_C)

$(BISON_C) $(BISON_H): $(BISON_FILE)
	bison -d -Wall $(BISON_FILE)

$(FLEX_C): $(FLEX_FILE) $(BISON_H)
	flex $(FLEX_FILE)

test: $(EXEC)
	tests/run_tests.sh

clean:
	rm -f $(EXEC) $(BISON_C) $(BISON_H) $(FLEX_C)

.PHONY: all test clean
