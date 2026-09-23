/* FGA0003 - Compiladores 1 */
/* Curso de Engenharia de Software */
/* Universidade de Brasília (UnB) */

%{
#include "parser.tab.h"  /* Cabeçalho gerado pelo Bison */
#include <stdlib.h>       /* strtol() */
#include <stdio.h>
#include <errno.h>        /* errno / ERANGE */
#include <limits.h>       /* INT_MAX */

int yycolumn = 1;

/* Definida no parser.y: marca que a linha atual teve erro
   e, por isso, não deve imprimir resultado */
extern int erro_na_linha;

/* Atualiza yylloc a partir do lexema atual (definida no fim do arquivo) */
static void atualiza_local(void);

/* Executada automaticamente antes da ação de TODA regra */
#define YY_USER_ACTION atualiza_local();
%}

/* Opção para evitar ter que linkar com -lfl e dispensar a função yywrap */
%option noyywrap

/* O Flex incrementa yylineno sozinho a cada '\n' casado.
   NUNCA faça yylineno++ manualmente nas regras. */
%option yylineno

%%

[ \t\r]+                { /* ignora espaços, tabs e CR residual do Windows */ }
"//".*                  { /* ignora comentários de linha */ }

"/*"([^*]|\*+[^*/])*\*+"/" { /* comentário multilinha fechado: ignora */ }

"/*"([^*]|\*+[^*/])*\**    {
    // Comentário que nunca foi fechado: casa da abertura até o fim do arquivo.
    // Quando o comentário está fechado, a regra de cima casa um caractere
    // a mais (a barra final) e vence pela regra do maior casamento.
    fprintf(stderr, "Erro Léxico [Linha %d, Col %d]: Comentário não fechado\n",
            yylloc.first_line, yylloc.first_column);
    // Fecha a linha atual para o parser não gerar um erro sintático em cascata
    erro_na_linha = 1;
    return NEWLINE;
}

"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }
"return"                { return RETURN; }
"int"                   { return INT; }
"float"                 { return FLOAT; }
"char"                  { return CHAR; }
"void"                  { return VOID; }

[0-9]+  {
    errno = 0;
    long valor = strtol(yytext, NULL, 10);
    if (errno == ERANGE || valor > INT_MAX) {
        fprintf(stderr, "Erro Léxico [Linha %d, Col %d]: Número muito grande '%s' (máximo %d)\n",
                yylloc.first_line, yylloc.first_column, yytext, INT_MAX);
        erro_na_linha = 1;
        yylval.intValue = 0;
    } else {
        yylval.intValue = (int) valor;
    }
    return NUM;
}

[a-zA-Z_][a-zA-Z0-9_]*  { return ID; }

"+"     { return PLUS; }
"-"     { return MINUS; }
"*"     { return TIMES; }
"/"     { return DIV; }
"="     { return ASSIGN; }
"=="    { return EQ; }
"!="    { return NEQ; }
"<"     { return LT; }
"<="    { return LE; }
">"     { return GT; }
">="    { return GE; }

"("     { return LPAREN; }
")"     { return RPAREN; }
"{"     { return LBRACE; }
"}"     { return RBRACE; }
","     { return COMMA; }
";"     { return SEMICOLON; }

"\n"    { return NEWLINE; }   /* coluna já volta para 1 em atualiza_local() */

.       {
            fprintf(stderr, "Erro Léxico [Linha %d, Col %d]: Caractere inesperado '%s'\n",
                    yylloc.first_line, yylloc.first_column, yytext);
            erro_na_linha = 1;
        }

%%

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
