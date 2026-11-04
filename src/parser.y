%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tabela.h"
#include "semantica.h"

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

/* Variável global que guardará a raiz da AST ao final do parsing */
struct NoAST *ast_raiz_programa = NULL;
%}

/* INJEÇÃO NO HEADER: Garante que parser.tab.h conheça ast.h antes do %union */
%code requires {
    #include "ast.h"
}

/* Define a estrutura yylval que compartilha dados entre Flex e Bison */
%union {
    long intValue;
    double realValue;
    char *strValue;
    struct NoAST *no; /* CORREÇÃO: Uso de 'struct' para evitar conflito de importação no Flex */
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

/* Declara os tipos não-terminais que agora retornam nós da AST */
%type <no>       exp instrucao declaracao programa
%type <intValue> tipo

/* Libera o nome do ID se ele for descartado durante a recuperação de erro */
%destructor { free($$); } <strValue>
/* Libera a subárvore da memória caso haja erro sintático no meio dela */
%destructor { ast_liberar($$); } <no>
/* EXCEÇÃO: a lista de instruções de 'programa' já pertence a ast_raiz_programa
 * (ast_raiz_programa->esq) e é liberada no main. Sem este destructor vazio, o
 * Bison libera a lista inteira ao fim do parse (símbolo inicial) e a AST fica
 * com ponteiros inválidos, além de dar double free no main. */
%destructor { } programa

/* Precedência de operadores (de baixo para cima = maior precedência) */
%left PLUS MINUS
%left TIMES DIV
%precedence UMINUS

%%

/* Um programa é uma sequência (possivelmente vazia) de instruções,
 * cada uma terminada por ';'. Quebras de linha não têm significado. */
programa:
    %empty { 
        $$ = NULL; 
        ast_raiz_programa = ast_criar_programa(NULL); /* Inicia a raiz do programa vazia */
    }
    | programa instrucao {
        $$ = ast_anexar($1, $2);
        ast_raiz_programa->esq = $$; /* Encadeia a instrução na raiz global */
        erro_na_instrucao = 0;
    }
    ;

instrucao:
    SEMICOLON {
        $$ = NULL; /* instrução vazia */
    }
    | declaracao {
        $$ = $1;
    }
    | ID ASSIGN exp SEMICOLON {
        $$ = ast_criar_atribuicao($1, $3, @1.first_line, @1.first_column);
        free($1);
    }
    | exp SEMICOLON {
        $$ = ast_criar_expressao($1, @1.first_line, @1.first_column);
    }
    | error SEMICOLON {
        /* Recuperação de erro sintático: descarta até o próximo ';' e retorna NULL para a AST ignorar */
        $$ = NULL; 
        yyerrok;
    }
    ;

declaracao:
    tipo ID SEMICOLON {
        $$ = ast_criar_declaracao((int)$1, $2, NULL, @2.first_line, @2.first_column);
        free($2);
    }
    | tipo ID ASSIGN exp SEMICOLON {
        $$ = ast_criar_declaracao((int)$1, $2, $4, @2.first_line, @2.first_column);
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
    NUM                     { $$ = ast_criar_numero_int(TIPO_INT, $1, @1.first_line, @1.first_column); }
    | NUM_LONG              { $$ = ast_criar_numero_int(TIPO_LONG, $1, @1.first_line, @1.first_column); }
    | CARACTERE             { $$ = ast_criar_numero_int(TIPO_CHAR, $1, @1.first_line, @1.first_column); }
    | NUM_REAL              { $$ = ast_criar_numero_real(TIPO_DOUBLE, $1, @1.first_line, @1.first_column); }
    | ID                    { 
                                $$ = ast_criar_identificador($1, @1.first_line, @1.first_column); 
                                free($1); 
                            }
    | exp PLUS exp          { $$ = ast_criar_operacao(AST_SOMA, $1, $3, @2.first_line, @2.first_column); }
    | exp MINUS exp         { $$ = ast_criar_operacao(AST_SUBTRACAO, $1, $3, @2.first_line, @2.first_column); }
    | exp TIMES exp         { $$ = ast_criar_operacao(AST_MULTIPLICACAO, $1, $3, @2.first_line, @2.first_column); }
    | exp DIV exp           { $$ = ast_criar_operacao(AST_DIVISAO, $1, $3, @2.first_line, @2.first_column); }
    | MINUS exp %prec UMINUS { $$ = ast_criar_menos_unario($2, @1.first_line, @1.first_column); }
    | LPAREN exp RPAREN     { $$ = $2; /* Repassa subárvore conforme a issue exige */ }
    ;

%%

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

    /* Uso: ./parser [--tabela] [--ast] [arquivo] */
    int mostrar_tabela = 0;
    int mostrar_ast = 0;
    const char *arquivo = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tabela") == 0 || strcmp(argv[i], "-t") == 0)
            mostrar_tabela = 1;
        else if (strcmp(argv[i], "--ast") == 0)
            mostrar_ast = 1;
        else
            arquivo = argv[i];
    }

    FILE *f = NULL;                    // guarda referência para fechar depois
    if (arquivo) {
        f = fopen(arquivo, "r");
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

    /* Análise semântica: só roda se o programa estiver correto léxica e
     * sintaticamente; senão a AST está incompleta e geraria erros falsos. */
    if (resultado == 0 && erros_lexicos == 0 && erros_sintaticos == 0) {
        semantica_analisar(ast_raiz_programa);
    }

    /* Imprime a AST se o argumento --ast foi passado (Exigência da Issue #28) */
    if (mostrar_ast && ast_raiz_programa != NULL) {
        ast_imprimir(ast_raiz_programa, 0);
    }

    if (mostrar_tabela) {
        printf("\n--- Tabela de Símbolos ---\n");
        tabela_imprimir(stdout);
    }
    
    tabela_liberar();
    
    /* Libera toda a árvore sintática gerada antes do encerramento */
    ast_liberar(ast_raiz_programa);

    /* O programa é rejeitado se houve qualquer erro léxico, sintático ou semântico */
    if (resultado != 0 || erros_lexicos > 0 || erros_sintaticos > 0 || erros_semanticos > 0) {
        return 1;
    }
    return 0;
}
