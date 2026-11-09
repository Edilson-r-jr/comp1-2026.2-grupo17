#include <stdio.h>
#include "../src/parser.tab.h"

YYSTYPE yylval;
YYLTYPE yylloc;
int erro_na_instrucao = 0;
int erros_lexicos = 0;

extern int yylex(void);
extern char *yytext;
extern FILE *yyin;

int main(int argc, char **argv) {
    int token;

    if (argc > 1) {
        yyin = fopen(argv[1], "r");

        if (!yyin) {
            perror("Erro ao abrir arquivo");
            return 1;
        }
    }

    while ((token = yylex()) != 0) {
        /* Nome simbólico (do parser.tab.h) em vez do número: o número muda
         * sempre que um token novo é declarado no parser.y */
        const char *nome;
        switch (token) {
            case IF:   nome = "IF"; break;
            case ELSE: nome = "ELSE"; break;
            case ID:   nome = "ID"; break;
            default:   nome = "OUTRO"; break;
        }
        printf("%s %s\n", nome, yytext);
    }

    if (argc > 1) {
        fclose(yyin);
    }

    return 0;
}
