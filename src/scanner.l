/* FGA0003 - Compiladores 1 */
/* Curso de Engenharia de Software */
/* Universidade de Brasília (UnB) */

%{
#include "parser.tab.h"  /* Cabeçalho gerado pelo Bison */
#include <stdlib.h>       /* Necessário para o atoi() */
#include <stdio.h>
int yycolumn = 1;

#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}

/* Opção para evitar ter que linkar com -lfl e dispensar a função yywrap */
%option noyywrap

%option yylineno

%%

[ \t\r]+                  { /* ignora apenas espaços e tabs */ }
"//".*                  { /* ignora comentarios de linha */ }
"/*"([^*]|\*+[^*/])*\*+"/" {
    for (int i = 0; i < yyleng; i++) {
        if (yytext[i] == '\n') { yylineno++; yycolumn = 1; }
    }
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
    yylval.intValue = atoi(yytext);
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
"\n"    { yylineno++; yycolumn = 1; }
.       {
            fprintf(stderr, "Erro Léxico [Linha %d, Col %d]: Caractere inesperado '%s'\n",
                    yylloc.first_line, yylloc.first_column, yytext);
        }

%%
