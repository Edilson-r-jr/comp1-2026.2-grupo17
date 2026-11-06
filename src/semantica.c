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

static int analisar_expressao(NoAST *no);

/* Tipo do resultado de uma operação aritmética: o mais largo dos
 * operandos, no mínimo int (char + char -> int, como no C). */
static int tipo_promovido(int a, int b)
{
    int t = a > b ? a : b;
    return t < TIPO_INT ? TIPO_INT : t;
}

/* Atribuição e declaração inicializada: promoções e estreitamentos entre
 * inteiros ou entre reais são aceitos, como no C; só real -> inteiro é erro.
 * Se algum lado já tem erro, nada é reportado (evita erro em cascata). */
static void verificar_atribuicao(const NoAST *no, int destino, int origem)
{
    if (destino == TIPO_ERRO || origem == TIPO_ERRO)
        return;
    if (tipo_eh_real(origem) && !tipo_eh_real(destino))
    {
        definir_posicao(no->linha, no->coluna);
        erro_semantico("atribuição de '%s' para '%s' não permitida em '%s'",
                       nome_tipo(origem), nome_tipo(destino), no->texto);
    }
}

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
 * atribuição). Se achar, anota no nó o símbolo e o tipo dele; senão, o nó
 * fica com TIPO_ERRO. */
static void resolver_nome(NoAST *no)
{
    Simbolo *s = tabela_buscar(no->texto, escopo_atual);
    if (s == NULL)
    {
        definir_posicao(no->linha, no->coluna);
        erro_semantico("Variável '%s' não declarada", no->texto);
        no->tipo_c = TIPO_ERRO;
        return;
    }
    no->simbolo = s;
    no->tipo_c = s->tipo;
}

/* Visita uma expressão inteira (os filhos antes do pai), anota o tipo de
 * cada nó em no->tipo_c e devolve o tipo da expressão. */
static int analisar_expressao(NoAST *no)
{
    if (no == NULL)
        return TIPO_ERRO;

    switch (no->tipo)
    {
    case AST_IDENTIFICADOR:
        resolver_nome(no);
        break;

    case AST_NUMERO:
        /* literal: o tipo já vem do parser (real é double) */
        break;

    case AST_SOMA:
    case AST_SUBTRACAO:
    case AST_MULTIPLICACAO:
    case AST_DIVISAO:
    {
        int esq = analisar_expressao(no->esq);
        int dir = analisar_expressao(no->dir);
        if (esq == TIPO_ERRO || dir == TIPO_ERRO)
            no->tipo_c = TIPO_ERRO; /* erro já reportado no filho */
        else if (no->tipo == AST_DIVISAO && eh_literal_zero(no->dir))
        {
            /* aponta para o divisor, como a mensagem antiga do parser */
            definir_posicao(no->dir->linha, no->dir->coluna);
            erro_semantico("Divisão por zero!");
            no->tipo_c = TIPO_ERRO;
        }
        else
            no->tipo_c = tipo_promovido(esq, dir);
        break;
    }

    case AST_MENOS_UNARIO:
    {
        int t = analisar_expressao(no->esq);
        no->tipo_c = (t == TIPO_ERRO) ? TIPO_ERRO : tipo_promovido(t, t);
        break;
    }

    default:
        /* instruções nunca aparecem dentro de uma expressão */
        no->tipo_c = TIPO_ERRO;
        break;
    }
    return no->tipo_c;
}

/* Visita uma instrução do programa. */
static void analisar_instrucao(NoAST *no)
{
    switch (no->tipo)
    {
    case AST_DECLARACAO:
        /* A inicialização é analisada ANTES de inserir o nome: assim, em
         * "int x = x + 1;" o 'x' da direita ainda não existe e gera erro. */
        if (no->esq)
            verificar_atribuicao(no, no->tipo_c, analisar_expressao(no->esq));

        /* tabela_inserir já reporta a redeclaração e devolve NULL */
        definir_posicao(no->linha, no->coluna);
        no->simbolo = tabela_inserir(no->texto, CAT_VARIAVEL, (Tipo)no->tipo_c,
                                     escopo_atual, no->linha, no->coluna);
        break;

    case AST_ATRIBUICAO:
        resolver_nome(no);
        verificar_atribuicao(no, no->tipo_c, analisar_expressao(no->esq));
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
