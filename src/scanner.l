/* FGA0003 - Compiladores 1 */
/* Curso de Engenharia de Software */
/* Universidade de Brasília (UnB) */

%{
#include "parser.tab.h"   /* Cabeçalho gerado pelo Bison (bison -d parser.y) */
#include <stdlib.h>       /* strtol, strtod */
#include <stdio.h>
#include <string.h>       /* strdup */
#include <errno.h>        /* errno / ERANGE */
#include <limits.h>       /* INT_MAX */

int yycolumn = 1;

/* Definidas no parser.y:
 *  - erro_na_instrucao: a instrução atual teve erro e não deve imprimir resultado
 *  - erros_lexicos: total de erros léxicos (define o código de saída) */
extern int erro_na_instrucao;
extern int erros_lexicos;

/* Registra um erro léxico na posição do lexema atual */
#define ERRO_LEXICO(...) do {                                              \
        fprintf(stderr, "Erro Léxico [Linha %d, Col %d]: ",                \
                yylloc.first_line, yylloc.first_column);                   \
        fprintf(stderr, __VA_ARGS__);                                      \
        fputc('\n', stderr);                                               \
        erro_na_instrucao = 1;                                             \
        erros_lexicos++;                                                   \
    } while (0)

/* Atualiza yylloc a partir do lexema atual (definida no fim do arquivo) */
static void atualiza_local(void);

/* Converte um literal de caractere ('a', '\n', ...) no seu código */
static char valor_char(const char *texto);

/* Executada automaticamente antes da ação de TODA regra */
#define YY_USER_ACTION atualiza_local();
%}

/* noyywrap: dispensa yywrap/-lfl.  nounput/noinput: evitam warnings de funções não usadas. */
%option noyywrap nounput noinput

/* O Flex incrementa yylineno sozinho a cada '\n' casado.
   NUNCA faça yylineno++ manualmente nas regras. */
%option yylineno

DIGITO     [0-9]
LETRA      [a-zA-Z_]
EXPOENTE   [eE][+-]?{DIGITO}+

%%

[ \t\r]+                { /* ignora espaços, tabs e CR residual do Windows */ }
"\n"                    { /* quebra de linha não tem significado (fim de instrução é ';');
                             a coluna volta para 1 em atualiza_local() */ }
"//".*                  { /* ignora comentários de linha */ }

"/*"([^*]|\*+[^*/])*\*+"/" { /* comentário multilinha fechado: ignora */ }

"/*"([^*]|\*+[^*/])*\**    {
    // Comentário que nunca foi fechado: casa da abertura até o fim do arquivo.
    // Quando o comentário está fechado, a regra de cima casa um caractere
    // a mais (a barra final) e vence pela regra do maior casamento.
    ERRO_LEXICO("Comentário não fechado");
    // Fecha a instrução atual para o parser não gerar um erro sintático em cascata
    return SEMICOLON;
}

 /* Palavras-reservadas (precisam vir ANTES da regra de ID) */
"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }
"return"                { return RETURN; }
"int"                   { return INT; }
"char"                  { return CHAR; }
"float"                 { return FLOAT; }
"double"                { return DOUBLE; }
"long"                  { return LONG; }
"void"                  { return VOID; }

 /* Literais de ponto flutuante: 10.5  10.  .5  1e3  2.5e-3 */
{DIGITO}+"."{DIGITO}*{EXPOENTE}?  |
"."{DIGITO}+{EXPOENTE}?          |
{DIGITO}+{EXPOENTE}              {
    yylval.realValue = strtod(yytext, NULL);
    return NUM_REAL;
}

 /* Literal inteiro long: 10L */
{DIGITO}+[lL]  {
    errno = 0;
    long valor = strtol(yytext, NULL, 10);
    if (errno == ERANGE) {
        ERRO_LEXICO("Número muito grande '%s' (máximo %ld)", yytext, LONG_MAX);
        valor = 0;
    }
    yylval.intValue = valor;
    return NUM_LONG;
}

 /* Literal inteiro: 42 */
{DIGITO}+  {
    errno = 0;
    long valor = strtol(yytext, NULL, 10);
    if (errno == ERANGE || valor > INT_MAX) {
        ERRO_LEXICO("Número muito grande '%s' (máximo %d)", yytext, INT_MAX);
        valor = 0;
    }
    yylval.intValue = valor;
    return NUM;
}

 /* Literal de caractere: 'a'  '\n'  '\'' */
\'([^'\\\n]|\\.)\'  {
    yylval.intValue = valor_char(yytext);
    return CARACTERE;
}

{LETRA}({LETRA}|{DIGITO})*  {
    yylval.strValue = strdup(yytext);
    return ID;
}

"+"     { return PLUS; }
"-"     { return MINUS; }
"*"     { return TIMES; }
"/"     { return DIV; }
"=="    { return EQ; }
"!="    { return NEQ; }
"<="    { return LE; }
">="    { return GE; }
"="     { return ASSIGN; }
"<"     { return LT; }
">"     { return GT; }

"("     { return LPAREN; }
")"     { return RPAREN; }
"{"     { return LBRACE; }
"}"     { return RBRACE; }
","     { return COMMA; }
";"     { return SEMICOLON; }   /* fim de instrução */

.       { ERRO_LEXICO("Caractere inesperado '%s'", yytext); }

%%

static char valor_char(const char *texto) {
    /* texto aponta para algo como  'a'  ou  '\n'  */
    if (texto[1] != '\\') {
        return texto[1];
    }
    switch (texto[2]) {
        case 'n':  return '\n';
        case 't':  return '\t';
        case 'r':  return '\r';
        case '0':  return '\0';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"':  return '"';
        default:   return texto[2];
    }
}

/*
 * Chamada antes de cada ação (via YY_USER_ACTION).
 *
 * Com %option yylineno, o Flex já somou em yylineno todas as quebras de
 * linha do lexema ANTES de chegar aqui. Por isso:
 *   - linha inicial = yylineno - (quebras dentro do lexema)
 *   - se o lexema contém '\n', a próxima coluna é contada a partir
 *     da última quebra (vale para "\n" e para comentários multilinha).
 */
static void atualiza_local(void) {
    int quebras = 0;
    int pos_ultima_quebra = -1;

    for (int i = 0; i < yyleng; i++) {
        if (yytext[i] == '\n') {
            quebras++;
            pos_ultima_quebra = i;
        }
    }

    yylloc.first_line   = yylineno - quebras;
    yylloc.last_line    = yylineno;
    yylloc.first_column = yycolumn;

    if (pos_ultima_quebra >= 0)
        yycolumn = yyleng - pos_ultima_quebra;  /* 1 + caracteres após o último '\n' */
    else
        yycolumn += yyleng;

    yylloc.last_column = yycolumn - 1;
}
