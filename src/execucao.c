#include <stdio.h>

#include "execucao.h"
#include "tabela.h"

/* Representa o resultado de uma expressão. */
typedef struct
{
    int tipo;
    long i;
    double r;
    int ok;
} Valor;

/* Cria um valor inteiro. */
static Valor valor_int(int tipo, long v)
{
    Valor x = {tipo, v, 0.0, 1};
    return x;
}

/* Cria um valor real. */
static Valor valor_real(int tipo, double v)
{
    Valor x = {tipo, 0, v, 1};
    return x;
}

/* Representa uma expressão inválida. */
static Valor valor_invalido(void)
{
    Valor x = {TIPO_INT, 0, 0.0, 0};
    return x;
}

/* Converte um valor para outro tipo. */
static Valor converter(Valor v, int tipo)
{
    if (!v.ok)
        return v;

    if (tipo_eh_real(tipo))
    {
        double r = tipo_eh_real(v.tipo) ? v.r : (double)v.i;

        if (tipo == TIPO_FLOAT)
            r = (float)r;

        return valor_real(tipo, r);
    }

    long i = tipo_eh_real(v.tipo) ? (long)v.r : v.i;

    if (tipo == TIPO_CHAR)
        i = (char)i;
    else if (tipo == TIPO_INT)
        i = (int)i;

    return valor_int(tipo, i);
}

/* Imprime o resultado de uma expressão. */
static void imprimir(Valor v)
{
    if (!v.ok)
        return;

    switch (v.tipo)
    {
    case TIPO_CHAR:
        printf("%c\n", (char)v.i);
        break;

    case TIPO_FLOAT:
    case TIPO_DOUBLE:
        printf("%g\n", v.r);
        break;

    default:
        printf("%ld\n", v.i);
        break;
    }
}

/* Lê o valor atual de uma variável. */
static Valor ler_variavel(Simbolo *simbolo)
{
    if (simbolo == NULL)
        return valor_invalido();

    if (tipo_eh_real(simbolo->tipo))
        return valor_real(simbolo->tipo, simbolo->valor_real);

    return valor_int(simbolo->tipo, simbolo->valor_int);
}

/* Avalia uma expressão da AST. */
static Valor avaliar_expressao(NoAST *no)
{
    if (no == NULL)
        return valor_invalido();

    Valor a;
    Valor b;
    int tipo;

    switch (no->tipo)
    {
    case AST_NUMERO:
        if (tipo_eh_real(no->tipo_c))
            return valor_real(no->tipo_c, no->valor.r);

        return valor_int(no->tipo_c, no->valor.i);

    case AST_IDENTIFICADOR:
        return ler_variavel(no->simbolo);

    case AST_EXPRESSAO:
        return avaliar_expressao(no->esq);

    case AST_MENOS_UNARIO:
        a = avaliar_expressao(no->esq);

        if (!a.ok)
            return a;

        if (tipo_eh_real(a.tipo))
            return valor_real(a.tipo, -a.r);

        return valor_int(a.tipo, -a.i);

    case AST_SOMA:
    case AST_SUBTRACAO:
    case AST_MULTIPLICACAO:
    case AST_DIVISAO:
    case AST_IGUAL:
    case AST_DIFERENTE:
    case AST_MENOR:
    case AST_MENOR_IGUAL:
    case AST_MAIOR:
    case AST_MAIOR_IGUAL:
        a = avaliar_expressao(no->esq);
        b = avaliar_expressao(no->dir);

        if (!a.ok || !b.ok)
            return valor_invalido();

        tipo = a.tipo > b.tipo ? a.tipo : b.tipo;

        if (tipo < TIPO_INT)
            tipo = TIPO_INT;

        a = converter(a, tipo);
        b = converter(b, tipo);

        /* Comparações sempre produzem um inteiro: 0 ou 1. */
        if (no->tipo == AST_IGUAL || no->tipo == AST_DIFERENTE || no->tipo == AST_MENOR || no->tipo == AST_MENOR_IGUAL || no->tipo == AST_MAIOR || no->tipo == AST_MAIOR_IGUAL)
        {
            int resultado = 0;

            if (tipo_eh_real(tipo))
            {
                switch (no->tipo)
                {
                case AST_IGUAL:        resultado = a.r == b.r; break;
                case AST_DIFERENTE:    resultado = a.r != b.r; break;
                case AST_MENOR:        resultado = a.r < b.r; break;
                case AST_MENOR_IGUAL:  resultado = a.r <= b.r; break;
                case AST_MAIOR:        resultado = a.r > b.r; break;
                case AST_MAIOR_IGUAL:  resultado = a.r >= b.r; break;
                default: break;
                }
            }
            else
            {
                switch (no->tipo)
                {
                case AST_IGUAL:        resultado = a.i == b.i; break;
                case AST_DIFERENTE:    resultado = a.i != b.i; break;
                case AST_MENOR:        resultado = a.i < b.i; break;
                case AST_MENOR_IGUAL:  resultado = a.i <= b.i; break;
                case AST_MAIOR:        resultado = a.i > b.i; break;
                case AST_MAIOR_IGUAL:  resultado = a.i >= b.i; break;
                default: break;
                }
            }

            return valor_int(TIPO_INT, resultado);
        }

        /* Operações aritméticas com valores reais. */
        if (tipo_eh_real(tipo))
        {
            switch (no->tipo)
            {
            case AST_SOMA:
                return valor_real(tipo, a.r + b.r);

            case AST_SUBTRACAO:
                return valor_real(tipo, a.r - b.r);

            case AST_MULTIPLICACAO:
                return valor_real(tipo, a.r * b.r);

            case AST_DIVISAO:
                if (b.r == 0.0)
                {
                    fprintf(stderr,
                            "Erro de execução na linha %d: divisão por zero.\n",
                            no->linha);
                    return valor_invalido();
                }

                return valor_real(tipo, a.r / b.r);

            default:
                break;
            }
        }
        else
        {
            switch (no->tipo)
            {
            case AST_SOMA:
                return valor_int(tipo, a.i + b.i);

            case AST_SUBTRACAO:
                return valor_int(tipo, a.i - b.i);

            case AST_MULTIPLICACAO:
                return valor_int(tipo, a.i * b.i);

            case AST_DIVISAO:
                if (b.i == 0)
                {
                    fprintf(stderr,
                            "Erro de execução na linha %d: divisão por zero.\n",
                            no->linha);
                    return valor_invalido();
                }

                return valor_int(tipo, a.i / b.i);

            default:
                break;
            }
        }

        return valor_invalido();

    default:
        return valor_invalido();
    }
}

static int executar_no(NoAST *no);

/* Executa uma instrução ou sequência de instruções. */
static int executar_instrucoes(NoAST *no)
{
    while (no != NULL)
    {
        if (!executar_no(no))
            return 0;

        no = no->proximo;
    }

    return 1;
}

/* Executa um nó da AST. */
static int executar_no(NoAST *no)
{
    if (no == NULL)
        return 1;

    Valor v;

    switch (no->tipo)
    {
    case AST_PROGRAMA:
    case AST_BLOCO:
        return executar_instrucoes(no->esq);

    case AST_DECLARACAO:
        if (no->simbolo == NULL)
            return 0;

        if (no->esq == NULL)
            return 1;

        v = avaliar_expressao(no->esq);

        if (!v.ok)
            return 0;

        v = converter(v, no->simbolo->tipo);

        if (tipo_eh_real(no->simbolo->tipo))
            no->simbolo->valor_real = v.r;
        else
            no->simbolo->valor_int = v.i;

        return 1;

    case AST_ATRIBUICAO:
        if (no->simbolo == NULL)
            return 0;

        v = avaliar_expressao(no->esq);

        if (!v.ok)
            return 0;

        v = converter(v, no->simbolo->tipo);

        if (tipo_eh_real(no->simbolo->tipo))
            no->simbolo->valor_real = v.r;
        else
            no->simbolo->valor_int = v.i;

        return 1;

    case AST_EXPRESSAO:
        v = avaliar_expressao(no->esq);
        imprimir(v);
        return v.ok;

    case AST_IF:
        v = avaliar_expressao(no->esq);

        if (!v.ok)
            return 0;

        if (tipo_eh_real(v.tipo))
        {
            if (v.r != 0.0)
                return executar_no(no->dir);
        }
        else
        {
            if (v.i != 0)
                return executar_no(no->dir);
        }

        return executar_no(no->terceiro);

    default:
        return 1;
    }
}

/* Ponto de entrada da execução. */
int execucao_executar(NoAST *raiz)
{
    return executar_no(raiz);
}