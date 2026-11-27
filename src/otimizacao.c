/* FGA0003 - Compiladores 1
 * Curso de Engenharia de Software
 * Universidade de Brasília (UnB)
 * Semana 9 - prática de otimização de código
 * otimizacao.c - Constant folding sobre a AST (ver otimizacao.h)
 *
 * Segue a ideia da função optimize() do exemplo da Semana 9: percorre a
 * expressão de baixo para cima e, quando um operador tem filhos numéricos,
 * substitui o nó pelo resultado. Diferente do exemplo, que só usa int, o
 * resultado respeita as regras do C já seguidas na semântica:
 *   - o tipo do resultado é o tipo_c anotado no nó pela semântica;
 *   - inteiros (char/int/long) usam aritmética inteira, e a divisão trunca;
 *   - reais usam ponto flutuante, e float é arredondado com (float);
 *   - comparações resultam em int 1 ou 0.
 *
 * Estouro de int dá a volta em 32 bits, como o "| 0" que o gerador de
 * JavaScript aplica a toda operação int; assim o resultado dobrado é o
 * mesmo do programa sem otimização.
 *
 * Uma operação não é dobrada quando o resultado não pode ser calculado
 * aqui com o mesmo efeito do programa: divisão por zero que só aparece
 * depois da otimização (ex.: 1 / (2 - 2)) e estouro de long. Nesses casos
 * a expressão fica como está.
 *
 * Também faz as simplificações algébricas x + 0, x - 0, x * 1 e x / 1 -> x,
 * quando x já tem o tipo do resultado.
 */
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include "otimizacao.h"

static void otimizar_expressao(NoAST *no);
static void otimizar_lista(NoAST *instrucoes);

/* ---------- Leitura dos operandos ---------- */

static int eh_constante(const NoAST *no)
{
    return no != NULL && no->tipo == AST_NUMERO;
}

/* Valor de uma constante como real (para operações em ponto flutuante) */
static double valor_real(const NoAST *no)
{
    return tipo_eh_real(no->tipo_c) ? no->valor.r : (double)no->valor.i;
}

/* Resultado inteiro no tipo do nó: int (32 bits) dá a volta, como o "| 0"
 * do JavaScript gerado; long fica como está (o estouro é tratado antes). */
static long ajustar_inteiro(long valor, int tipo_c)
{
    if (tipo_c == TIPO_LONG)
        return valor;
    return (long)(int32_t)(uint32_t)valor;
}

/* ---------- Troca do nó pelo resultado ---------- */

/* Transforma 'no' em AST_NUMERO, liberando os filhos. Linha, coluna e
 * tipo_c (anotado pela semântica) são mantidos. */
static void virar_constante(NoAST *no)
{
    ast_liberar(no->esq);
    ast_liberar(no->dir);
    no->esq = NULL;
    no->dir = NULL;
    no->tipo = AST_NUMERO;
}

static void virar_inteiro(NoAST *no, long valor)
{
    virar_constante(no);
    no->valor.i = valor;
}

static void virar_real(NoAST *no, double valor)
{
    virar_constante(no);
    if (no->tipo_c == TIPO_FLOAT)
        valor = (float)valor;
    no->valor.r = valor;
}

/* ---------- Dobra de cada tipo de operação ---------- */

static int eh_comparacao(TipoNoAST tipo)
{
    return tipo == AST_IGUAL || tipo == AST_DIFERENTE ||
           tipo == AST_MENOR || tipo == AST_MENOR_IGUAL ||
           tipo == AST_MAIOR || tipo == AST_MAIOR_IGUAL;
}

static int eh_aritmetica(TipoNoAST tipo)
{
    return tipo == AST_SOMA || tipo == AST_SUBTRACAO ||
           tipo == AST_MULTIPLICACAO || tipo == AST_DIVISAO;
}

/* Comparação entre constantes -> int 1 ou 0. Se algum lado é real, a
 * comparação é feita em ponto flutuante, como no C. */
static void dobrar_comparacao(NoAST *no)
{
    const NoAST *e = no->esq, *d = no->dir;
    int resultado;

    if (tipo_eh_real(e->tipo_c) || tipo_eh_real(d->tipo_c))
    {
        double a = valor_real(e), b = valor_real(d);
        switch (no->tipo)
        {
        case AST_IGUAL:       resultado = a == b; break;
        case AST_DIFERENTE:   resultado = a != b; break;
        case AST_MENOR:       resultado = a <  b; break;
        case AST_MENOR_IGUAL: resultado = a <= b; break;
        case AST_MAIOR:       resultado = a >  b; break;
        default:              resultado = a >= b; break;
        }
    }
    else
    {
        long a = e->valor.i, b = d->valor.i;
        switch (no->tipo)
        {
        case AST_IGUAL:       resultado = a == b; break;
        case AST_DIFERENTE:   resultado = a != b; break;
        case AST_MENOR:       resultado = a <  b; break;
        case AST_MENOR_IGUAL: resultado = a <= b; break;
        case AST_MAIOR:       resultado = a >  b; break;
        default:              resultado = a >= b; break;
        }
    }
    virar_inteiro(no, resultado);
}

/* + - * / entre constantes, no tipo do resultado anotado pela semântica */
static void dobrar_aritmetica(NoAST *no)
{
    const NoAST *e = no->esq, *d = no->dir;

    if (tipo_eh_real(no->tipo_c))
    {
        double a = valor_real(e), b = valor_real(d);
        if (no->tipo == AST_DIVISAO && b == 0.0)
            return; /* divisão por zero: mantém a expressão */
        switch (no->tipo)
        {
        case AST_SOMA:          virar_real(no, a + b); break;
        case AST_SUBTRACAO:     virar_real(no, a - b); break;
        case AST_MULTIPLICACAO: virar_real(no, a * b); break;
        default:                virar_real(no, a / b); break;
        }
        return;
    }

    long a = e->valor.i, b = d->valor.i, r;
    int estourou;
    switch (no->tipo)
    {
    case AST_SOMA:          estourou = __builtin_add_overflow(a, b, &r); break;
    case AST_SUBTRACAO:     estourou = __builtin_sub_overflow(a, b, &r); break;
    case AST_MULTIPLICACAO: estourou = __builtin_mul_overflow(a, b, &r); break;
    default:
        if (b == 0)
            return; /* divisão inteira por zero: mantém a expressão */
        estourou = (a == LONG_MIN && b == -1);
        r = estourou ? 0 : a / b; /* em C a divisão inteira já trunca */
        break;
    }
    /* Operandos int cabem em 32 bits, então só um long pode estourar aqui */
    if (estourou)
        return; /* estouro de long: mantém a expressão */
    virar_inteiro(no, ajustar_inteiro(r, no->tipo_c));
}

/* -constante -> constante negativa */
static void dobrar_menos_unario(NoAST *no)
{
    const NoAST *operando = no->esq;

    if (tipo_eh_real(no->tipo_c))
    {
        virar_real(no, -valor_real(operando));
        return;
    }

    long v = operando->valor.i;
    if (v == LONG_MIN)
        return; /* estouro de long: mantém a expressão */
    virar_inteiro(no, ajustar_inteiro(-v, no->tipo_c));
}

/* ---------- Simplificação algébrica ---------- */

/* A constante vale exatamente 'n' (0 ou 1)? */
static int constante_vale(const NoAST *no, long n)
{
    if (!eh_constante(no))
        return 0;
    return tipo_eh_real(no->tipo_c) ? no->valor.r == (double)n
                                    : no->valor.i == n;
}

/* x + 0, x - 0, x * 1, x / 1 -> x. Só quando x já tem o tipo do resultado:
 * em "x + 0.0" com x int o resultado é double, e trocar por x mudaria,
 * por exemplo, (x + 0.0) / 2 de divisão real para divisão inteira. */
static void simplificar(NoAST *no)
{
    int neutro;
    switch (no->tipo)
    {
    case AST_SOMA:
    case AST_SUBTRACAO:     neutro = constante_vale(no->dir, 0); break;
    case AST_MULTIPLICACAO:
    case AST_DIVISAO:       neutro = constante_vale(no->dir, 1); break;
    default:                return;
    }
    if (!neutro || no->esq->tipo_c != no->tipo_c)
        return;

    /* O nó passa a ser o próprio x: copia o filho para o lugar do nó e
     * libera a constante e a casca do filho (os netos agora são do nó). */
    NoAST *x = no->esq;
    ast_liberar(no->dir);
    *no = *x;
    free(x);
}

/* ---------- Percurso ---------- */

/* Otimiza os filhos antes do nó (de baixo para cima) */
static void otimizar_expressao(NoAST *no)
{
    if (no == NULL)
        return;

    otimizar_expressao(no->esq);
    otimizar_expressao(no->dir);

    if (no->tipo == AST_MENOS_UNARIO)
    {
        if (eh_constante(no->esq))
            dobrar_menos_unario(no);
    }
    else if (eh_constante(no->esq) && eh_constante(no->dir))
    {
        if (eh_aritmetica(no->tipo))
            dobrar_aritmetica(no);
        else if (eh_comparacao(no->tipo))
            dobrar_comparacao(no);
    }

    /* Sem dobra (x não é constante): tenta x + 0, x * 1, ... -> x */
    if (eh_aritmetica(no->tipo))
        simplificar(no);
}

static void otimizar_instrucao(NoAST *no)
{
    switch (no->tipo)
    {
    case AST_DECLARACAO:   /* esq = inicialização (ou NULL) */
    case AST_ATRIBUICAO:
    case AST_EXPRESSAO:
        otimizar_expressao(no->esq);
        break;

    case AST_BLOCO:
        otimizar_lista(no->esq);
        break;

    case AST_IF:
        otimizar_expressao(no->esq);
        if (no->dir)
            otimizar_instrucao(no->dir);
        if (no->terceiro)
            otimizar_instrucao(no->terceiro);
        break;

    case AST_WHILE:
        otimizar_expressao(no->esq);
        if (no->dir)
            otimizar_instrucao(no->dir);
        break;

    default:
        break;
    }
}

static void otimizar_lista(NoAST *instrucoes)
{
    for (NoAST *no = instrucoes; no != NULL; no = no->proximo)
        otimizar_instrucao(no);
}

void otimizacao_aplicar(NoAST *raiz)
{
    if (raiz != NULL)
        otimizar_lista(raiz->esq); /* AST_PROGRAMA: esq = primeira instrução */
}
