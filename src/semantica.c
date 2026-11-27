/* FGA0003 - Compiladores 1
 * semantica.c - Análise semântica sobre a AST (ver semantica.h)
 *
 * O programa é percorrido UMA vez, na ordem em que as instruções aparecem.
 * Isso garante a regra do C de "declarar antes de usar": quando um uso é
 * visitado, só as declarações anteriores já estão na tabela.
 * (Duas passagens completas — primeiro todas as declarações, depois todos
 * os usos — aceitariam "a = 5; int a;" sem erro.)
 */
#include <ctype.h>
#include <string.h>
#include "semantica.h"
#include "tabela.h"

/* Escopo atual. Começa no global; cada bloco { } abre um escopo filho
 * (ver analisar_bloco) e, ao terminar, volta para o anterior. */
static int escopo_atual = ESCOPO_GLOBAL;

static int analisar_expressao(NoAST *no);
static void analisar_instrucao(NoAST *no);

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

    no->analisado_semanticamente = 1;

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

    /* Comparações: os operandos podem ser de qualquer tipo numérico e o
     * resultado é int (1 = verdadeiro, 0 = falso), como no C. */
    case AST_IGUAL:
    case AST_DIFERENTE:
    case AST_MENOR:
    case AST_MENOR_IGUAL:
    case AST_MAIOR:
    case AST_MAIOR_IGUAL:
    {
        int esq = analisar_expressao(no->esq);
        int dir = analisar_expressao(no->dir);
        no->tipo_c = (esq == TIPO_ERRO || dir == TIPO_ERRO) ? TIPO_ERRO : TIPO_INT;
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

/* ---------- printf ---------- */

/* Especificadores aceitos no formato e os tipos que cada um aceita.
 * As regras seguem o que o C faz de verdade ao passar o valor para o
 * printf: char vira int, float vira double. Qualquer outra combinação
 * (ex.: "%d" com double) é comportamento indefinido em C e imprimiria
 * lixo, então é rejeitada: assim o JS gerado sempre imprime o mesmo que
 * o programa compilado com gcc. */
static int especificador_aceita(const char *esp, int tipo)
{
    if (strcmp(esp, "%d") == 0 || strcmp(esp, "%i") == 0 || strcmp(esp, "%c") == 0)
        return tipo == TIPO_CHAR || tipo == TIPO_INT;
    if (strcmp(esp, "%ld") == 0 || strcmp(esp, "%li") == 0)
        return tipo == TIPO_LONG;
    /* %f e %lf */
    return tipo == TIPO_FLOAT || tipo == TIPO_DOUBLE;
}

static const char *descricao_especificador(const char *esp)
{
    if (strcmp(esp, "%ld") == 0 || strcmp(esp, "%li") == 0)
        return "long";
    if (strcmp(esp, "%f") == 0 || strcmp(esp, "%lf") == 0)
        return "float ou double";
    return "char ou int";
}

/* Lê o especificador que começa em fmt[0] == '%'. Copia-o para 'esp'
 * (ex.: "%ld") e devolve quantos caracteres ele ocupa, ou 0 se não é um
 * especificador suportado. "%%" devolve 2 com esp = "%%". */
static int ler_especificador(const char *fmt, char esp[4])
{
    int n = 1;
    if (fmt[1] == 'l' && (fmt[2] == 'd' || fmt[2] == 'i' || fmt[2] == 'f'))
        n = 3;
    else if (fmt[1] == 'd' || fmt[1] == 'i' || fmt[1] == 'c' ||
             fmt[1] == 'f' || fmt[1] == '%')
        n = 2;
    else
        return 0;

    memcpy(esp, fmt, (size_t)n);
    esp[n] = '\0';
    return n;
}

/* Confere o formato do printf contra os argumentos (já analisados):
 * especificadores suportados, quantidade e tipo de cada argumento. */
static void verificar_printf(const NoAST *no)
{
    const NoAST *arg = no->esq;
    int esperados = 0;
    int recebidos = 0;

    for (const NoAST *a = no->esq; a != NULL; a = a->proximo)
        recebidos++;

    for (const char *c = no->texto; *c; c++)
    {
        if (*c != '%')
            continue;

        char esp[4];
        int tamanho = ler_especificador(c, esp);
        if (tamanho == 0)
        {
            /* Sem saber o que o formato pede, contar argumentos não faz
             * sentido: reporta só o especificador e para aqui. */
            /* Mostra o especificador até a letra que o termina
             * (ex.: "%.2f", "%5d", "%s") */
            int fim = 1;
            while (c[fim] && !isalpha((unsigned char)c[fim]) && c[fim] != '%')
                fim++;
            if (isalpha((unsigned char)c[fim]))
                fim++;
            definir_posicao(no->linha, no->coluna);
            erro_semantico("especificador '%.*s' não suportado no printf "
                           "(use %%d, %%i, %%ld, %%c, %%f, %%lf ou %%%%)",
                           fim, c);
            return;
        }
        c += tamanho - 1;
        if (strcmp(esp, "%%") == 0)
            continue;

        esperados++;
        if (arg == NULL)
            continue; /* falta argumento: reportado depois do laço */

        /* Argumento com erro já foi reportado: não gera erro em cascata */
        if (arg->tipo_c != TIPO_ERRO && !especificador_aceita(esp, arg->tipo_c))
        {
            definir_posicao(arg->linha, arg->coluna);
            erro_semantico("'%s' no printf espera %s, mas o argumento %d é '%s'",
                           esp, descricao_especificador(esp), esperados,
                           nome_tipo(arg->tipo_c));
        }
        arg = arg->proximo;
    }

    if (esperados != recebidos)
    {
        definir_posicao(no->linha, no->coluna);
        erro_semantico("o formato do printf pede %d argumento(s), mas foram passados %d",
                       esperados, recebidos);
    }
}

/* Bloco { ... }: as declarações feitas dentro dele valem só até o '}'.
 * Um nome declarado aqui pode "esconder" outro de fora sem ser redeclaração
 * (tabela_inserir só compara com o mesmo escopo), e tabela_buscar procura
 * do escopo atual para os de fora até o global. */
static void analisar_bloco(NoAST *bloco)
{
    int escopo_anterior = escopo_atual;
    escopo_atual = tabela_criar_escopo(escopo_anterior);

    for (NoAST *no = bloco->esq; no != NULL; no = no->proximo)
        analisar_instrucao(no);

    escopo_atual = escopo_anterior;
}

/* Visita uma instrução do programa. */
static void analisar_instrucao(NoAST *no)
{
    if (no == NULL)
        return;

    no->analisado_semanticamente = 1;

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

    case AST_BLOCO:
        analisar_bloco(no);
        break;

    case AST_IF:
        /* A condição é uma expressão comum (como no C, qualquer valor
         * diferente de zero é verdadeiro). Os dois caminhos são analisados,
         * porque um erro no else é erro mesmo que ele nunca execute. */
        analisar_expressao(no->esq);
        analisar_instrucao(no->dir);
        analisar_instrucao(no->terceiro);
        break;

    case AST_PRINTF:
        /* Cada argumento é uma expressão comum (variável não declarada
         * etc. são reportadas como sempre); depois, o formato. */
        for (NoAST *arg = no->esq; arg != NULL; arg = arg->proximo)
            analisar_expressao(arg);
        verificar_printf(no);
        break;

    case AST_WHILE:
        /* Condição: expressão comum, com erros reportados normalmente.
         * Corpo: AST_BLOCO, que abre escopo próprio em analisar_bloco,
         * então o que é declarado dentro do laço não é visível fora. */
        analisar_expressao(no->esq);
        analisar_instrucao(no->dir);
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
