/* FGA0003 - Compiladores 1
 * tabela.c - Tabela de Símbolos (implementação)
 *
 * Lista simplesmente encadeada com ponteiro para o início e para o fim.
 * A inserção é feita no final para que a impressão saia na mesma ordem
 * em que as declarações aparecem no código-fonte.
 */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "tabela.h"

static Simbolo *inicio = NULL;
static Simbolo *fim = NULL;

int erros_semanticos = 0;
static int pos_linha = 0;
static int pos_coluna = 0;


void definir_posicao(int linha, int coluna)
{
    pos_linha = linha;
    pos_coluna = coluna;
}

void erro_semantico(const char *fmt, ...)
{
    va_list args;

    fprintf(stderr, "Erro Semântico [Linha %d, Col %d]: ", pos_linha, pos_coluna);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    erros_semanticos++;
}


const char *nome_tipo(int tipo)
{
    switch (tipo)
    {
    case TIPO_CHAR:
        return "char";
    case TIPO_INT:
        return "int";
    case TIPO_LONG:
        return "long";
    case TIPO_FLOAT:
        return "float";
    case TIPO_DOUBLE:
        return "double";
    case TIPO_VOID:
        return "void";
    default:
        return "desconhecido";
    }
}

const char *nome_categoria(int categoria)
{
    switch (categoria)
    {
    case CAT_VARIAVEL:
        return "variavel";
    case CAT_FUNCAO:
        return "funcao";
    case CAT_PARAMETRO:
        return "parametro";
    default:
        return "desconhecida";
    }
}

int tipo_eh_real(int tipo)
{
    return tipo == TIPO_FLOAT || tipo == TIPO_DOUBLE;
}


/* Busca só dentro de um escopo, sem cair para o global. */
static Simbolo *buscar_no_escopo(const char *nome, int escopo)
{
    for (Simbolo *s = inicio; s != NULL; s = s->prox)
    {
        if (s->escopo == escopo && strcmp(s->nome, nome) == 0)
        {
            return s;
        }
    }
    return NULL;
}

Simbolo *tabela_buscar(const char *nome, int escopo)
{
    Simbolo *s = buscar_no_escopo(nome, escopo);
    if (s == NULL && escopo != ESCOPO_GLOBAL)
    {
        s = buscar_no_escopo(nome, ESCOPO_GLOBAL);
    }
    return s;
}

Simbolo *tabela_inserir(const char *nome, Categoria categoria, Tipo tipo,
                        int escopo, int linha, int coluna)
{
    /* Redeclaração só é erro dentro do mesmo escopo
     * (um escopo interno pode "esconder" um nome do global). */
    Simbolo *existente = buscar_no_escopo(nome, escopo);
    if (existente != NULL)
    {
        erro_semantico("Variável '%s' já foi declarada na linha %d",
                       nome, existente->linha);
        return NULL;
    }

    Simbolo *novo = malloc(sizeof(Simbolo));
    if (novo == NULL)
    {
        perror("malloc");
        exit(1);
    }
    novo->nome = strdup(nome);
    if (novo->nome == NULL)
    {
        perror("strdup");
        exit(1);
    }
    novo->categoria = categoria;
    novo->tipo = tipo;
    novo->escopo = escopo;
    novo->linha = linha;
    novo->coluna = coluna;
    novo->valor_int = 0;
    novo->valor_real = 0.0;
    novo->prox = NULL;

    /* insere no final: O(1) graças ao ponteiro 'fim' */
    if (fim == NULL)
    {
        inicio = novo;
    }
    else
    {
        fim->prox = novo;
    }
    fim = novo;

    return novo;
}

void tabela_imprimir(FILE *saida)
{
    fprintf(saida, "%-12s %-10s %-7s %-7s %s\n",
            "NOME", "CATEGORIA", "TIPO", "ESCOPO", "LINHA");

    for (Simbolo *s = inicio; s != NULL; s = s->prox)
    {
        char escopo[16];
        if (s->escopo == ESCOPO_GLOBAL)
        {
            snprintf(escopo, sizeof escopo, "global");
        }
        else
        {
            snprintf(escopo, sizeof escopo, "%d", s->escopo);
        }

        fprintf(saida, "%-12s %-10s %-7s %-7s %d\n",
                s->nome, nome_categoria(s->categoria), nome_tipo(s->tipo),
                escopo, s->linha);
    }
}

void tabela_liberar(void)
{
    Simbolo *s = inicio;
    while (s != NULL)
    {
        Simbolo *prox = s->prox;
        free(s->nome);
        free(s);
        s = prox;
    }
    inicio = NULL;
    fim = NULL;
}