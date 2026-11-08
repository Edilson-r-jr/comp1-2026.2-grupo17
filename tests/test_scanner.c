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
        printf("token=%d texto=%s\n", token, yytext);
    }

    if (argc > 1) {
        fclose(yyin);
    }

    return 0;
}
