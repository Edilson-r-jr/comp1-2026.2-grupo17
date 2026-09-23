%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Protótipos das funções necessárias para o Bison */
int yylex(void);
void yyerror(const char *s);
extern FILE *yyin;

/* Marca que a linha atual teve erro (léxico ou semântico) e, por isso,
   não deve imprimir resultado. Também é usada pelo scanner.l. */
int erro_na_linha = 0;
%}

/* Define a estrutura yylval que compartilha dados entre Flex e Bison */
%union {
    int intValue;
}

/* Declaração dos Tokens combinando exatamente com o seu Lexer */
%token <intValue> NUM
%token PLUS MINUS TIMES DIV LPAREN RPAREN NEWLINE

/* Habilita @N e yylloc (linha/coluna) */
%locations

/* Mensagens de erro sintático montadas por nós em yyreport_syntax_error()
   (requer Bison 3.6 ou mais novo) */
%define parse.error custom

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
    NEWLINE {
        erro_na_linha = 0;
    }
    | exp NEWLINE {
        if (!erro_na_linha) {
            printf("%d\n", $1);
        }
        erro_na_linha = 0;
    }
    | error NEWLINE {
        /* Recuperação simples de erros sintáticos por linha */
        yyerrok;
        erro_na_linha = 0;
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
            /* @3 = posição do divisor */
            fprintf(stderr, "Erro Semântico [Linha %d, Col %d]: Divisão por zero!\n",
                    @3.first_line, @3.first_column);
            erro_na_linha = 1;
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

/* Nome amigável, em português, de cada símbolo da gramática */
static const char *nome_simbolo(yysymbol_kind_t s) {
    switch (s) {
        case YYSYMBOL_YYEOF:     return "fim de arquivo";
        case YYSYMBOL_NUM:       return "número";
        case YYSYMBOL_ID:        return "identificador";
        case YYSYMBOL_NEWLINE:   return "quebra de linha";
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
        case YYSYMBOL_FLOAT:     return "palavra reservada 'float'";
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

    fprintf(stderr, "Erro Sintático [Linha %d, Col %d]: ",
            loc->first_line, loc->first_column);

    if (encontrado != YYSYMBOL_YYEMPTY)
        fprintf(stderr, "encontrado %s", nome_simbolo(encontrado));
    else
        fprintf(stderr, "erro de sintaxe");

    int espera_quebra = 0;
    for (int i = 0; i < n; i++) {
        if (esperados[i] == YYSYMBOL_NEWLINE) espera_quebra = 1;
        if (i == 0)          fprintf(stderr, ", mas era esperado ");
        else if (i == n - 1) fprintf(stderr, " ou ");
        else                 fprintf(stderr, ", ");
        fprintf(stderr, "%s", nome_simbolo(esperados[i]));
    }

    /* Dica para o caso mais comum: arquivo sem Enter na última linha */
    if (encontrado == YYSYMBOL_YYEOF && espera_quebra)
        fprintf(stderr, " (faltou uma quebra de linha no final do arquivo?)");

    fprintf(stderr, "\n");
    return 0;
}

/* Continua sendo usada pelo Bison para erros internos (ex.: memória esgotada) */
void yyerror(const char *s) {
    fprintf(stderr, "Erro Sintático [Linha %d, Col %d]: %s\n",
            yylloc.first_line, yylloc.first_column, s);
}

/* Função principal (Main) colocada aqui na raiz do Parser */
int main(int argc, char **argv) {
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
    } else {
        if (isatty(STDIN_FILENO)) {
            printf("Modo interativo. Digite contas (ex: 2 + 3 * 4) e aperte Enter:\n");
        }
    }
    int resultado = yyparse();        // captura o retorno antes de fechar

    if (f) fclose(f);                 // só fecha se um arquivo foi aberto
    return resultado;
}
