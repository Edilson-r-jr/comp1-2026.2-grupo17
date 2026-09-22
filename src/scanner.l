/* FGA0003 - Compiladores 1 */
/* Curso de Engenharia de Software */
/* Universidade de Brasília (UnB) */

%{
#include "parser.tab.h"   /* Cabeçalho gerado pelo Bison (bison -d parser.y) */
#include <stdlib.h>       /* strtol, strtod */
#include <stdio.h>
#include <string.h>

extern int erros_lexicos;   /* definido em parser.y */

/* Converte o conteúdo de um literal de caractere ('a', '\n', ...) no seu código. */
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
%}

/* noyywrap: dispensa yywrap/-lfl.  yylineno: número da linha para mensagens de erro.
 * nounput/noinput: evitam warnings de funções geradas e não usadas. */
%option noyywrap yylineno nounput noinput

DIGITO     [0-9]
LETRA      [a-zA-Z_]
EXPOENTE   [eE][+-]?{DIGITO}+

%%

[ \t\r\n]+                  { /* ignora espaços, tabs e quebras de linha */ }
"//".*                      { /* ignora comentarios de linha */ }
"/*"([^*]|\*+[^*/])*\*+"/"  { /* ignora comentarios de bloco */ }

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
    yylval.intValue = strtol(yytext, NULL, 10);
    return NUM_LONG;
}

 /* Literal inteiro: 42 */
{DIGITO}+  {
    yylval.intValue = strtol(yytext, NULL, 10);
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

.       { fprintf(stderr, "Erro Lexico (linha %d): caractere invalido '%s'\n", yylineno, yytext); erros_lexicos++; }

%%
