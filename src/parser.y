%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include "tabela.h"

/* Protótipos das funções necessárias para o Bison */
int yylex(void);
void yyerror(const char *s);
extern FILE *yyin;

/* Marca que a instrução atual teve erro (léxico ou semântico) e, por isso,
   não deve imprimir resultado. Também é usada pelo scanner.l. */
int erro_na_instrucao = 0;

/* Contadores de erro: se algum for > 0, o programa sai com código 1 */
int erros_lexicos = 0;          /* incrementado pelo scanner.l */
static int erros_sintaticos = 0;

/* Posição usada nas mensagens de erro semântico (ver tabela.h) */
#define POSICAO(loc) definir_posicao((loc).first_line, (loc).first_column)
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

/* Habilita @N e yylloc (linha/coluna) */
%locations

/* Mensagens de erro sintático montadas por nós em yyreport_syntax_error()
   (requer Bison 3.6 ou mais novo) */
%define parse.error custom

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
    | programa instrucao {
        erro_na_instrucao = 0;
    }
    ;

instrucao:
    SEMICOLON                         /* instrução vazia */
    | declaracao
    | ID ASSIGN exp SEMICOLON {
        POSICAO(@1);
        atribuir($1, $3);
        free($1);
    }
    | exp SEMICOLON {
        if (!erro_na_instrucao) {
            imprimir($1);
        }
    }
    | error SEMICOLON {
        /* Recuperação de erro sintático: descarta até o próximo ';' */
        yyerrok;
    }
    ;

declaracao:
    tipo ID SEMICOLON {
        POSICAO(@2);
        inserir_variavel($2, (int)$1);
        free($2);
    }
    | tipo ID ASSIGN exp SEMICOLON {
        POSICAO(@2);
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
    | ID                    { POSICAO(@1); $$ = ler_variavel($1); free($1); }
    | exp PLUS exp          { $$ = operacao($1, '+', $3); }
    | exp MINUS exp         { $$ = operacao($1, '-', $3); }
    | exp TIMES exp         { $$ = operacao($1, '*', $3); }
    | exp DIV exp           { POSICAO(@3); /* posição do divisor */
                              $$ = operacao($1, '/', $3); }
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
                if (b.r == 0.0) { erro_semantico("Divisão por zero!%s", ""); return valor_invalido(); }
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
                if (b.i == 0) { erro_semantico("Divisão por zero!%s", ""); return valor_invalido(); }
                i = a.i / b.i;
                break;
        }
        return converter(valor_int(TIPO_LONG, i), tipo);
    }
}

/* Uso de variável: precisa ter sido declarada antes. */
static Valor ler_variavel(char *nome) {
    if (verificar_tipo(nome) == TIPO_INEXISTENTE) {
        erro_semantico("Variável '%s' não declarada", nome);
        return valor_invalido();
    }
    Simbolo *s = buscar_variavel(nome);
    return tipo_eh_real(s->tipo) ? valor_real(s->tipo, s->valor_real)
                                 : valor_int(s->tipo, s->valor_int);
}

/* Atribuição: a variável precisa existir; o valor é convertido para o tipo dela. */
static void atribuir(char *nome, Valor v) {
    if (verificar_tipo(nome) == TIPO_INEXISTENTE) {
        erro_semantico("Variável '%s' não declarada", nome);
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

/* ---------- Mensagens de erro sintático ---------- */

/* Nome amigável, em português, de cada símbolo da gramática */
static const char *nome_simbolo(yysymbol_kind_t s) {
    switch (s) {
        case YYSYMBOL_YYEOF:     return "fim de arquivo";
        case YYSYMBOL_NUM:       return "número";
        case YYSYMBOL_NUM_LONG:  return "número long";
        case YYSYMBOL_NUM_REAL:  return "número real";
        case YYSYMBOL_CARACTERE: return "caractere";
        case YYSYMBOL_ID:        return "identificador";
        case YYSYMBOL_PLUS:      return "'+'";
        case YYSYMBOL_MINUS:     return "'-'";
        case YYSYMBOL_TIMES:     return "'*'";
        case YYSYMBOL_DIV:       return "'/'";
        case YYSYMBOL_LPAREN:    return "'('";
        case YYSYMBOL_RPAREN:    return "')'";
        case YYSYMBOL_LBRACE:    return "'{'";
        case YYSYMBOL_RBRACE:    return "'}'";
        case YYSYMBOL_COMMA:     return "','";
        case YYSYMBOL_SEMICOLON: return "';'";
        case YYSYMBOL_ASSIGN:    return "'='";
        case YYSYMBOL_EQ:        return "'=='";
        case YYSYMBOL_NEQ:       return "'!='";
        case YYSYMBOL_LT:        return "'<'";
        case YYSYMBOL_LE:        return "'<='";
        case YYSYMBOL_GT:        return "'>'";
        case YYSYMBOL_GE:        return "'>='";
        case YYSYMBOL_IF:        return "palavra reservada 'if'";
        case YYSYMBOL_ELSE:      return "palavra reservada 'else'";
        case YYSYMBOL_WHILE:     return "palavra reservada 'while'";
        case YYSYMBOL_RETURN:    return "palavra reservada 'return'";
        case YYSYMBOL_INT:       return "palavra reservada 'int'";
        case YYSYMBOL_LONG:      return "palavra reservada 'long'";
        case YYSYMBOL_FLOAT:     return "palavra reservada 'float'";
        case YYSYMBOL_DOUBLE:    return "palavra reservada 'double'";
        case YYSYMBOL_CHAR:      return "palavra reservada 'char'";
        case YYSYMBOL_VOID:      return "palavra reservada 'void'";
        default:                 return yysymbol_name(s);
    }
}

/*
 * Chamada pelo Bison (parse.error custom) quando há erro sintático.
 * Monta: Erro Sintático [Linha X, Col Y]: encontrado <token>, mas era esperado A, B ou C
 */
static int yyreport_syntax_error(const yypcontext_t *ctx) {
    enum { MAX_ESPERADOS = 10 };
    yysymbol_kind_t esperados[MAX_ESPERADOS];
    int n = yypcontext_expected_tokens(ctx, esperados, MAX_ESPERADOS);
    yysymbol_kind_t encontrado = yypcontext_token(ctx);
    const YYLTYPE *loc = yypcontext_location(ctx);

    erros_sintaticos++;

    fprintf(stderr, "Erro Sintático [Linha %d, Col %d]: ",
            loc->first_line, loc->first_column);

    if (encontrado != YYSYMBOL_YYEMPTY)
        fprintf(stderr, "encontrado %s", nome_simbolo(encontrado));
    else
        fprintf(stderr, "erro de sintaxe");

    int espera_ponto_virgula = 0;
    for (int i = 0; i < n; i++) {
        if (esperados[i] == YYSYMBOL_SEMICOLON) espera_ponto_virgula = 1;
        if (i == 0)          fprintf(stderr, ", mas era esperado ");
        else if (i == n - 1) fprintf(stderr, " ou ");
        else                 fprintf(stderr, ", ");
        fprintf(stderr, "%s", nome_simbolo(esperados[i]));
    }

    /* Dica para o caso mais comum: esqueceu o ';' da última instrução */
    if (encontrado == YYSYMBOL_YYEOF && espera_ponto_virgula)
        fprintf(stderr, " (faltou ';' no final da última instrução?)");

    fprintf(stderr, "\n");
    return 0;
}

/* Continua sendo usada pelo Bison para erros internos (ex.: memória esgotada) */
void yyerror(const char *s) {
    fprintf(stderr, "Erro Sintático [Linha %d, Col %d]: %s\n",
            yylloc.first_line, yylloc.first_column, s);
    erros_sintaticos++;
}

/* Função principal (Main) colocada aqui na raiz do Parser */
int main(int argc, char **argv) {
    /* stdout sem buffer: mantém a ordem correta entre resultados (stdout)
       e mensagens de erro (stderr) quando os testes usam 2>&1 */
    setvbuf(stdout, NULL, _IONBF, 0);

    /* stdout sem buffer: mantém a ordem correta entre resultados (stdout)
       e mensagens de erro (stderr) quando os testes usam 2>&1 */
    setvbuf(stdout, NULL, _IONBF, 0);

    FILE *f = NULL;                    // guarda referência para fechar depois
    if (argc > 1) {
        f = fopen(argv[1], "r");
        if (!f) {
            perror("Erro ao abrir o arquivo fornecido");
            return 1;
        }
        yyin = f;
    } else if (isatty(STDIN_FILENO)) {
        printf("Modo interativo. Digite instruções terminadas em ';' (ex: int x = 2; x * 3;)\n");
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

