/* FGA0003 - Compiladores 1 */
/* Curso de Engenharia de Software */
/* Universidade de Brasília (UnB) */

%{
#include "token.tab.h"  /* Cabeçalho gerado pelo Bison */
#include <stdlib.h>       /* Necessário para o atoi() */
#include <stdio.h>
%}

/* Opção para evitar ter que linkar com -lfl e dispensar a função yywrap */
%option noyywrap

%%

[ \t]+                  { /* ignora apenas espaços e tabs */ }
[0-9]+  {
    yylval.intValue = atoi(yytext);
    return NUM;
}
"+"     { return PLUS; }
"-"     { return MINUS; }
"*"     { return TIMES; }
"/"     { return DIV; }
"("     { return LPAREN; }
")"     { return RPAREN; }
"\n"    { return NEWLINE; } /* Adicionado o ponto e vírgula */
.       { printf("caractere invalido: %s\n", yytext); } /* Corrigido de $s para %s e adicionado ; */

%%
