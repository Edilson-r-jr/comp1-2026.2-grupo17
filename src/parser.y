%{
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Protótipos das funções necessárias para o Bison */
int yylex(void);
void yyerror(const char *s);
extern FILE *yyin;
%}

/* Define a estrutura yylval que compartilha dados entre Flex e Bison */
%union {
    int intValue;
}

/* Declaração dos Tokens combinando exatamente com o seu Lexer */
%token <intValue> NUM
%token PLUS MINUS TIMES DIV LPAREN RPAREN NEWLINE

/* Novos Tokens para atender a issue */
%token ID
%token IF ELSE WHILE RETURN INT FLOAT CHAR VOID
%token ASSIGN EQ NEQ LT LE GT GE
%token LBRACE RBRACE COMMA SEMICOLON

%type <intValue> exp

/* Definição de Precedência de Operadores (De baixo para cima = maior precedência) */
%left PLUS MINUS
%left TIMES DIV

%%

/* Regras da Gramática (Context-Free Grammar) */

input:
    /* vazio - permite que o arquivo comece em branco ou leia múltiplas linhas */
    | input line
    ;

line:
    NEWLINE
    | exp NEWLINE { 
        printf("%d\n", $1); 
    }
    | error NEWLINE { 
        /* Recuperação simples de erros sintáticos por linha */
        yyerrok; 
    }
    ;

exp:
    NUM { 
        $$ = $1; 
    }
    | exp PLUS exp { 
        $$ = $1 + $3; 
    }
    | exp MINUS exp { 
        $$ = $1 - $3; 
    }
    | exp TIMES exp { 
        $$ = $1 * $3; 
    }
    | exp DIV exp { 
        if ($3 == 0) {
            yyerror("Erro de divisão por zero!");
            $$ = 0;
        } else {
            $$ = $1 / $3; 
        }
    }
    | LPAREN exp RPAREN { 
        $$ = $2; 
    }
    ;

%%

/* Função obrigatória do Bison para relatar erros de sintaxe */
void yyerror(const char *s) {
    fprintf(stderr, "Erro de Sintaxe: %s\n", s);
}

/* Função principal (Main) colocada aqui na raiz do Parser */
int main(int argc, char **argv) {
    FILE *f = NULL;                    // guarda referência para fechar depois
    if (argc > 1) {
        f = fopen(argv[1], "r");
        if (!f) {
            perror("Erro ao abrir o arquivo fornecido");
            return 1;
        }
        yyin = f;
    } else {
        if (isatty(STDIN_FILENO)) {
        printf("Modo interativo. Digite contas (ex: 2 + 3 * 4) e aperte Enter:\n");
    }
    }
    int resultado = yyparse();        // captura o retorno antes de fechar

    if (f) fclose(f);                 // só fecha se um arquivo foi aberto
    return resultado;
}