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
"//".*                  { /* ignora comentarios de linha */ }
"/*"([^*]|\*+[^*/])*\*+"/" { /* ignora comentarios de bloco */ }

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
"\n"    { return NEWLINE; } /* Adicionado o ponto e vírgula */
.       { printf("caractere invalido: %s\n", yytext); } /* Corrigido de $s para %s e adicionado ; */

%%