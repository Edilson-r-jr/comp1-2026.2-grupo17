/* FGA0003 - Compiladores 1
 * semantica.c - Análise semântica sobre a AST (ver semantica.h)
 *
 * O programa é percorrido UMA vez, na ordem em que as instruções aparecem.
 * Isso garante a regra do C de "declarar antes de usar": quando um uso é
 * visitado, só as declarações anteriores já estão na tabela.
 * (Duas passagens completas — primeiro todas as declarações, depois todos
 * os usos — aceitariam "a = 5; int a;" sem erro.)
 */
#include "semantica.h"
#include "tabela.h"

/* Escopo atual. Por enquanto só existe o global; quando a gramática tiver
 * blocos e funções, a análise passa a entrar e sair de escopos aqui. */
static int escopo_atual = ESCOPO_GLOBAL;

static void analisar_expressao(NoAST *no);

/* Divisão cujo divisor é o literal 0 ou 0.0 (ex.: "x / 0").
 * Divisores calculados (ex.: "x / y") só são conhecidos ao executar. */
static int eh_literal_zero(const NoAST *no)
{
    if (no == NULL || no->tipo != AST_NUMERO)
        return 0;
    if (tipo_eh_real(no->tipo_c))
        return no->valor.r == 0.0;
    return no->valor.i == 0;
}

/* Resolve um nome usado no programa (identificador ou lado esquerdo de uma
 * atribuição). Se achar, anota no nó o símbolo e o tipo dele. */
static void resolver_nome(NoAST *no)
{
    Simbolo *s = tabela_buscar(no->texto, escopo_atual);
    if (s == NULL)
    {
        definir_posicao(no->linha, no->coluna);
        erro_semantico("Variável '%s' não declarada", no->texto);
        return;
    }
    no->simbolo = s;
    no->tipo_c = s->tipo;
}

/* Visita uma expressão inteira (os filhos antes do pai). */
static void analisar_expressao(NoAST *no)
{
    if (no == NULL)
        return;

    switch (no->tipo)
    {
    case AST_IDENTIFICADOR:
        resolver_nome(no);
        break;

    case AST_NUMERO:
        /* literal: nada a resolver (o tipo já vem do parser) */
        break;

    case AST_SOMA:
    case AST_SUBTRACAO:
    case AST_MULTIPLICACAO:
        analisar_expressao(no->esq);
        analisar_expressao(no->dir);
        break;

    case AST_DIVISAO:
        analisar_expressao(no->esq);
        analisar_expressao(no->dir);
        if (eh_literal_zero(no->dir))
        {
            /* aponta para o divisor, como a mensagem antiga do parser */
            definir_posicao(no->dir->linha, no->dir->coluna);
            erro_semantico("Divisão por zero!");
        }
        break;

    case AST_MENOS_UNARIO:
        analisar_expressao(no->esq);
        break;

    default:
        /* instruções nunca aparecem dentro de uma expressão */
        break;
    }
}

/* Visita uma instrução do programa. */
static void analisar_instrucao(NoAST *no)
{
    switch (no->tipo)
    {
    case AST_DECLARACAO:
        /* A inicialização é analisada ANTES de inserir o nome: assim, em
         * "int x = x + 1;" o 'x' da direita ainda não existe e gera erro. */
        analisar_expressao(no->esq);

        /* tabela_inserir já reporta a redeclaração e devolve NULL */
        definir_posicao(no->linha, no->coluna);
        no->simbolo = tabela_inserir(no->texto, CAT_VARIAVEL, (Tipo)no->tipo_c,
                                     escopo_atual, no->linha, no->coluna);
        break;

    case AST_ATRIBUICAO:
        resolver_nome(no);
        analisar_expressao(no->esq);
        break;

    case AST_EXPRESSAO:
        analisar_expressao(no->esq);
        break;

    default:
        break;
    }
}

int semantica_analisar(NoAST *raiz)
{
    if (raiz == NULL)
        return 0;

    int erros_antes = erros_semanticos;

    /* raiz é AST_PROGRAMA; as instruções ficam em esq, ligadas por 'proximo' */
    for (NoAST *no = raiz->esq; no != NULL; no = no->proximo)
        analisar_instrucao(no);

    return erros_semanticos - erros_antes;
}
