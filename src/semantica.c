/* FGA0003 - Compiladores 1
 * semantica.c - Análise semântica sobre a AST
 *
 * Percorre o programa na ordem das instruções, verificando declarações,
 * tipos, usos de variáveis, expressões, blocos e condicionais.
 */

#include "semantica.h"
#include "tabela.h"

/* Escopo atualmente analisado. */
static int escopo_atual = ESCOPO_GLOBAL;

static int analisar_expressao(NoAST *no);
static void analisar_instrucao(NoAST *no);

/* Tipo do resultado de uma operação aritmética.
 * O resultado é o tipo mais largo dos operandos, no mínimo int. */
static int tipo_promovido(int a, int b)
{
    int t = a > b ? a : b;
    return t < TIPO_INT ? TIPO_INT : t;
}

/* Verifica atribuições e inicializações.
 * Conversões de real para inteiro não são permitidas.
 * Erros anteriores não geram mensagens em cascata. */
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

/* Verifica se o nó representa um literal numérico igual a zero. */
static int eh_literal_zero(const NoAST *no)
{
    if (no == NULL || no->tipo != AST_NUMERO)
        return 0;

    if (tipo_eh_real(no->tipo_c))
        return no->valor.r == 0.0;

    return no->valor.i == 0;
}

/* Resolve um identificador e registra seu símbolo e tipo no nó. */
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

/* Analisa uma expressão e devolve seu tipo. */
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
        /* O tipo do literal já foi definido pelo parser. */
        break;

    case AST_SOMA:
    case AST_SUBTRACAO:
    case AST_MULTIPLICACAO:
    case AST_DIVISAO:
    {
        int esq = analisar_expressao(no->esq);
        int dir = analisar_expressao(no->dir);

        if (esq == TIPO_ERRO || dir == TIPO_ERRO)
        {
            no->tipo_c = TIPO_ERRO;
        }
        else if (no->tipo == AST_DIVISAO && eh_literal_zero(no->dir))
        {
            definir_posicao(no->dir->linha, no->dir->coluna);
            erro_semantico("Divisão por zero!");
            no->tipo_c = TIPO_ERRO;
        }
        else
        {
            no->tipo_c = tipo_promovido(esq, dir);
        }
        break;
    }

    case AST_IGUAL:
    case AST_DIFERENTE:
    case AST_MENOR:
    case AST_MENOR_IGUAL:
    case AST_MAIOR:
    case AST_MAIOR_IGUAL:
    {
        int esq = analisar_expressao(no->esq);
        int dir = analisar_expressao(no->dir);

        if (esq == TIPO_ERRO || dir == TIPO_ERRO)
            no->tipo_c = TIPO_ERRO;
        else
            no->tipo_c = TIPO_INT;

        break;
    }

    case AST_MENOS_UNARIO:
    {
        int tipo = analisar_expressao(no->esq);

        if (tipo == TIPO_ERRO)
            no->tipo_c = TIPO_ERRO;
        else
            no->tipo_c = tipo_promovido(tipo, tipo);

        break;
    }

    default:
        no->tipo_c = TIPO_ERRO;
        break;
    }

    return no->tipo_c;
}

/* Analisa todas as instruções de um bloco em um novo escopo. */
static void analisar_bloco(NoAST *bloco)
{
    if (bloco == NULL)
        return;

    int escopo_anterior = escopo_atual;

    escopo_atual = tabela_criar_escopo(escopo_anterior);

    for (NoAST *instrucao = bloco->esq;
         instrucao != NULL;
         instrucao = instrucao->proximo)
    {
        analisar_instrucao(instrucao);
    }

    escopo_atual = escopo_anterior;
}

/* Analisa uma instrução individual. */
static void analisar_instrucao(NoAST *no)
{
    if (no == NULL)
        return;

    switch (no->tipo)
    {
    case AST_DECLARACAO:
        /* Analisa a inicialização antes de inserir o identificador. */
        if (no->esq != NULL)
        {
            int origem = analisar_expressao(no->esq);
            verificar_atribuicao(no, no->tipo_c, origem);
        }

        definir_posicao(no->linha, no->coluna);

        no->simbolo = tabela_inserir(
            no->texto,
            CAT_VARIAVEL,
            (Tipo)no->tipo_c,
            escopo_atual,
            no->linha,
            no->coluna
        );
        break;

    case AST_ATRIBUICAO:
    {
        resolver_nome(no);
        int origem = analisar_expressao(no->esq);
        verificar_atribuicao(no, no->tipo_c, origem);
        break;
    }

    case AST_EXPRESSAO:
        analisar_expressao(no->esq);
        break;

    case AST_BLOCO:
        analisar_bloco(no);
        break;

    case AST_IF:
        /* A condição também precisa ter identificadores válidos. */
        analisar_expressao(no->esq);

        /* Analisa os dois caminhos, independentemente da execução. */
        analisar_instrucao(no->dir);
        analisar_instrucao(no->terceiro);
        break;

    default:
        break;
    }
}

/* Analisa o programa inteiro na ordem das instruções. */
int semantica_analisar(NoAST *raiz)
{
    if (raiz == NULL)
        return 0;

    escopo_atual = ESCOPO_GLOBAL;
    int erros_antes = erros_semanticos;

    for (NoAST *no = raiz->esq;
         no != NULL;
         no = no->proximo)
    {
        analisar_instrucao(no);
    }

    return erros_semanticos - erros_antes;
}
