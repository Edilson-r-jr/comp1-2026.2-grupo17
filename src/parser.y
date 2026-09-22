%{
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tabela.h"

/* Protótipos das funções necessárias para o Bison */
int yylex(void);
void yyerror(const char *s);
extern FILE *yyin;

static int erros_sintaticos = 0;
int erros_lexicos = 0;   /* incrementado pelo scanner */
%}

/* Tipo do valor semântico de uma expressão.
 * Vai para o parser.tab.h porque é usado dentro do %union. */
%code requires {
    typedef struct {
        int tipo;       /* um dos TIPO_* de tabela.h */
        long i;         /* valor se o tipo for char/int/long */
        double r;       /* valor se o tipo for float/double */
        int ok;         /* 0 se a expressão contém erro semântico */
    } Valor;
}

/* Protótipos que dependem de Valor (emitidos depois da definição do YYSTYPE) */
%code {
/* Funções auxiliares de expressões (definidas no final do arquivo) */
static Valor valor_int(int tipo, long v);
static Valor valor_real(int tipo, double v);
static Valor valor_invalido(void);
static Valor converter(Valor v, int tipo);
static Valor operacao(Valor a, char op, Valor b);
static Valor ler_variavel(char *nome);
static void atribuir(char *nome, Valor v);
static void imprimir(Valor v);
}

/* Define a estrutura yylval que compartilha dados entre Flex e Bison */
%union {
    long intValue;
    double realValue;
    char *strValue;
    Valor valor;
}

%define parse.error verbose

/* Literais */
%token <intValue>  NUM NUM_LONG CARACTERE
%token <realValue> NUM_REAL
%token <strValue>  ID

/* Palavras-reservadas */
%token IF ELSE WHILE RETURN VOID
%token CHAR INT LONG FLOAT DOUBLE

/* Operadores e pontuação */
%token PLUS MINUS TIMES DIV LPAREN RPAREN
%token ASSIGN EQ NEQ LT LE GT GE
%token LBRACE RBRACE COMMA
%token SEMICOLON   /* fim de instrução */

%type <valor>    exp
%type <intValue> tipo

/* Libera o nome do ID se ele for descartado durante a recuperação de erro */
%destructor { free($$); } <strValue>

/* Precedência de operadores (de baixo para cima = maior precedência) */
%left PLUS MINUS
%left TIMES DIV
%precedence UMINUS

%%

/* Um programa é uma sequência (possivelmente vazia) de instruções,
 * cada uma terminada por ';'. Quebras de linha não têm significado. */
programa:
    %empty    /* programa vazio */
    | programa instrucao
    ;

instrucao:
    SEMICOLON                         /* instrução vazia */
    | declaracao
    | ID ASSIGN exp SEMICOLON {
        atribuir($1, $3);
        free($1);
    }
    | exp SEMICOLON {
        imprimir($1);
    }
    | error SEMICOLON {
        /* Recuperação de erro sintático: descarta até o próximo ';' */
        yyerrok;
    }
    ;

declaracao:
    tipo ID SEMICOLON {
        inserir_variavel($2, (int)$1);
        free($2);
    }
    | tipo ID ASSIGN exp SEMICOLON {
        if (buscar_variavel($2) == NULL) {
            inserir_variavel($2, (int)$1);
            atribuir($2, $4);
        } else {
            inserir_variavel($2, (int)$1);   /* reporta a redeclaração */
        }
        free($2);
    }
    ;

tipo:
    CHAR     { $$ = TIPO_CHAR; }
    | INT    { $$ = TIPO_INT; }
    | LONG   { $$ = TIPO_LONG; }
    | FLOAT  { $$ = TIPO_FLOAT; }
    | DOUBLE { $$ = TIPO_DOUBLE; }
    ;

exp:
    NUM                     { $$ = valor_int(TIPO_INT, $1); }
    | NUM_LONG              { $$ = valor_int(TIPO_LONG, $1); }
    | CARACTERE             { $$ = valor_int(TIPO_CHAR, $1); }
    | NUM_REAL              { $$ = valor_real(TIPO_DOUBLE, $1); }
    | ID                    { $$ = ler_variavel($1); free($1); }
    | exp PLUS exp          { $$ = operacao($1, '+', $3); }
    | exp MINUS exp         { $$ = operacao($1, '-', $3); }
    | exp TIMES exp         { $$ = operacao($1, '*', $3); }
    | exp DIV exp           { $$ = operacao($1, '/', $3); }
    | MINUS exp %prec UMINUS { $$ = operacao(valor_int(TIPO_INT, 0), '-', $2); }
    | LPAREN exp RPAREN     { $$ = $2; }
    ;

%%

/* ---------- Funções auxiliares de expressões ---------- */

static Valor valor_int(int tipo, long v) {
    Valor x = { tipo, v, 0.0, 1 };
    return x;
}

static Valor valor_real(int tipo, double v) {
    Valor x = { tipo, 0, v, 1 };
    return x;
}

static Valor valor_invalido(void) {
    Valor x = { TIPO_INT, 0, 0.0, 0 };
    return x;
}

/* Converte um valor para outro tipo, com as mesmas regras de truncamento do C. */
static Valor converter(Valor v, int tipo) {
    if (!v.ok) return v;
    if (tipo_eh_real(tipo)) {
        double r = tipo_eh_real(v.tipo) ? v.r : (double)v.i;
        if (tipo == TIPO_FLOAT) r = (float)r;
        return valor_real(tipo, r);
    } else {
        long i = tipo_eh_real(v.tipo) ? (long)v.r : v.i;
        if (tipo == TIPO_CHAR)     i = (char)i;
        else if (tipo == TIPO_INT) i = (int)i;
        return valor_int(tipo, i);
    }
}

/* Operação binária com promoção de tipos:
 * o resultado tem o tipo "mais largo" dos dois operandos, e no mínimo int
 * (char + char -> int, igual ao C). Divisão entre inteiros é inteira. */
static Valor operacao(Valor a, char op, Valor b) {
    if (!a.ok || !b.ok) return valor_invalido();

    int tipo = a.tipo > b.tipo ? a.tipo : b.tipo;
    if (tipo < TIPO_INT) tipo = TIPO_INT;

    a = converter(a, tipo);
    b = converter(b, tipo);

    if (tipo_eh_real(tipo)) {
        double r = 0.0;
        switch (op) {
            case '+': r = a.r + b.r; break;
            case '-': r = a.r - b.r; break;
            case '*': r = a.r * b.r; break;
            case '/':
                if (b.r == 0.0) { erro_semantico("divisao por zero%s", ""); return valor_invalido(); }
                r = a.r / b.r;
                break;
        }
        return converter(valor_real(TIPO_DOUBLE, r), tipo);
    } else {
        long i = 0;
        switch (op) {
            case '+': i = a.i + b.i; break;
            case '-': i = a.i - b.i; break;
            case '*': i = a.i * b.i; break;
            case '/':
                if (b.i == 0) { erro_semantico("divisao por zero%s", ""); return valor_invalido(); }
                i = a.i / b.i;
                break;
        }
        return converter(valor_int(TIPO_LONG, i), tipo);
    }
}

/* Uso de variável: precisa ter sido declarada antes. */
static Valor ler_variavel(char *nome) {
    if (verificar_tipo(nome) == TIPO_INEXISTENTE) {
        erro_semantico("variavel '%s' nao declarada", nome);
        return valor_invalido();
    }
    Simbolo *s = buscar_variavel(nome);
    return tipo_eh_real(s->tipo) ? valor_real(s->tipo, s->valor_real)
                                 : valor_int(s->tipo, s->valor_int);
}

/* Atribuição: a variável precisa existir; o valor é convertido para o tipo dela. */
static void atribuir(char *nome, Valor v) {
    if (verificar_tipo(nome) == TIPO_INEXISTENTE) {
        erro_semantico("variavel '%s' nao declarada", nome);
        return;
    }
    if (!v.ok) return;
    Simbolo *s = buscar_variavel(nome);
    v = converter(v, s->tipo);
    s->valor_int = v.i;
    s->valor_real = v.r;
}

/* Imprime o resultado de uma instrução-expressão. */
static void imprimir(Valor v) {
    if (!v.ok) return;
    switch (v.tipo) {
        case TIPO_CHAR:   printf("%c\n", (char)v.i); break;
        case TIPO_FLOAT:
        case TIPO_DOUBLE: printf("%g\n", v.r); break;
        default:          printf("%ld\n", v.i); break;
    }
}

/* Função obrigatória do Bison para relatar erros de sintaxe */
void yyerror(const char *s) {
    fprintf(stderr, "Erro de Sintaxe (linha %d): %s\n", yylineno, s);
    erros_sintaticos++;
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
    } else if (isatty(STDIN_FILENO)) {
        printf("Modo interativo. Digite instrucoes terminadas em ';' (ex: int x = 2; x * 3;)\n");
        printf("Ctrl+D para sair.\n");
    }

    int resultado = yyparse();

    if (f) fclose(f);
    liberar_tabela();

    /* O programa é rejeitado se houve qualquer erro léxico, sintático ou semântico */
    if (resultado != 0 || erros_lexicos > 0 || erros_sintaticos > 0 || erros_semanticos > 0) {
        return 1;
    }
    return 0;
}
