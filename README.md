# comp1-2026.2-grupo17

Projeto: Compilador de C para JavaScript

**Ideia do Projeto**

O projeto consiste no desenvolvimento de um compilador capaz de receber um código escrito em C e realizar sua tradução para JavaScript.

O compilador será responsável por analisar o código-fonte, depois da analise o compilador vai trabalhar na geração de um código equivalente em JavaScript.

Para o desenvolvimento do projeto, serão utilizadas as ferramentas Flex, Bison, WSL, git, GitHub e VS CODE.

**Planejamento de Execução**

1. Configuração do ambiente

- Configurar o ambiente de desenvolvimento.
- Instalar e configurar WSL, Flex, Bison e Git.
- Configurar o repositório e a estrutura inicial do projeto.

2. Análise Léxica

- Definir os tokens da linguagem.
- Implementar o analisador léxico utilizando Flex.
- Realizar testes com diferentes códigos de entrada.

3. Análise Sintática

- Definir a gramática da linguagem.
- Integrar o analisador léxico ao analisador sintático.
- Testar estruturas válidas e inválidas.

4. Geração de Código

- Definir as regras de conversão de C para JavaScript.
- Implementar a geração do código JavaScript.
- Testar a saída gerada com diferentes programas.

5. Integração

- Integrar todas as etapas do compilador.
- Documentar o funcionamento do projeto.

6. Finalização

- Revisar o código e a documentação.
- Organizar o repositório.
- Realizar os testes finais.
- Preparar o projeto para a entrega.

## Escopo

C e JavaScript têm arquitetura, regras e design muito diferentes. Para tornar o projeto viável, o grupo definiu um escopo com as funcionalidades que serão cobertas. A ideia por trás dessa decisão é implementar os recursos necessários para que programas comuns possam ser compilados, respeitando o prazo da disciplina.

Para entender o que é possível fazer neste projeto, é importante conhecer as principais diferenças entre as duas linguagens. O C é estaticamente tipado e tem vários tipos numéricos de tamanho fixo, como `char`, `int`, `long`, `float` e `double`. O JavaScript é dinamicamente tipado e representa todo número como um `double` de 64 bits. Por isso, o compilador precisa usar a tabela de símbolos para emitir as conversões que simulam o comportamento do C:

- `| 0` para o estouro de `int` e para a divisão inteira;
- `Math.fround` para `float`;
- truncamento nas atribuições.

O C também dá acesso direto à memória, por meio de ponteiros, de `malloc`/`free` e do layout de `struct`/`union`. Já o JavaScript tem a memória gerenciada por um coletor de lixo e não trabalha com endereços. Esses recursos exigiriam simular uma memória inteira e, por isso, ficam fora do escopo. Outras diferenças também pesam: o JavaScript não tem `goto` nem pré-processador, e a biblioteca padrão do C, como o `printf`, precisa ser reimplementada em um pequeno _runtime_ em JS. Por outro lado, as estruturas de controle (`if`, `while`, `for`, `switch`) e as funções têm sintaxe e semântica quase idênticas nas duas linguagens, e o escopo de bloco do C corresponde ao `let` do JavaScript. Isso torna essa parte da tradução praticamente direta.

Com base nessa análise, o grupo chegou à tabela abaixo, com os requisitos que serão implementados ao longo da disciplina.

### Requisitos dentro do escopo do projeto

| Requisito                                                             | Status |
| --------------------------------------------------------------------- | :----: |
| Tipos `char`, `int`, `long`, `float`, `double`                        |   ✅   |
| Tipo `bool` com `true`/`false`                                        |   ⬜   |
| Literais inteiro, `long`, real e caractere                            |   ✅   |
| Literais de string (só como formato do `printf`)                      |   ✅   |
| Comentários de linha e de bloco                                       |   ✅   |
| `;` como fim de instrução                                             |   ✅   |
| Declaração com e sem inicialização (`tipo x;`, `tipo x = e;`)         |   ✅   |
| Várias variáveis por declaração (`int a, b;`)                         |   ⬜   |
| Tabela de símbolos: erros de redeclaração e de variável não declarada |   ✅   |
| Operadores `+ - * /` e `-` unário, com promoção de tipos              |   ✅   |
| Operadores relacionais `==`, `!=`, `<`, `<=`, `>`, `>=`               |   ✅   |
| `%`, operadores lógicos `&&`, `||` e `!`                              |   ⬜   |
| `if`/`else` e `while` (corpo sempre entre chaves)                     |   ✅   |
| `do-while`, `for`, `break`, `continue`                                |   ⬜   |
| Blocos e pilha de escopos                                             |   ✅   |
| Funções, `return`, recursão, `main`                                   |   ⬜   |
| `printf` e _runtime_ JS                                               |   ✅   |
| Ignorar `#include <stdio.h>` e `#include <stdbool.h>`                 |   ✅   |
| **Geração de código JavaScript**                                      |   ✅   |
| Erro explícito de "recurso fora do escopo"                            |   ⬜   |

✅ implementado · ⬜ pendente


### Arquitetura

```text
Código C
   |
scanner.l       Análise léxica
   |
parser.y        Análise sintática e construção da AST
   |
ast.c           Estrutura e impressão da AST
   |
semantica.c + tabela.c   Tipos, símbolos e escopos
   |
otimizacao.c    Constant folding opcional (--otimizar)
   |
   +----------------------+
   |                      |
tac.c                  gerador_js.c
TAC (--tac)            JavaScript (--js)
```

A análise semântica só ocorre após parsing sem erros. TAC e JavaScript só são emitidos quando não há erros léxicos, sintáticos ou semânticos. A otimização, quando solicitada, ocorre depois da semântica e antes das saídas.

### Como usar

Compile no Linux/WSL, a partir da raiz:

```bash
make -C src
```

O executável será `src/parser`. Opções disponíveis:

| Opção | Função |
| --- | --- |
| `--ast` | Imprime a AST anotada com tipos e símbolos. |
| `--tabela` ou `-t` | Imprime a tabela de símbolos. |
| `--tac` | Gera código intermediário de três endereços. |
| `--otimizar` | Aplica constant folding e simplificações algébricas. |
| `--js` | Gera JavaScript a partir da AST. |

Exemplos:

```bash
./src/parser --ast programa.c
./src/parser --tabela programa.c
./src/parser --otimizar --tac programa.c
./src/parser --otimizar --js programa.c
```

Exemplo completo:

```c
#include <stdio.h>
int n = 5;
int fatorial = 1;
while (n > 1) {
    fatorial = fatorial * n;
    n = n - 1;
}
printf("fatorial = %d\n", fatorial);
```

```bash
./src/parser --js fatorial.c > fatorial.js
node fatorial.js
```

Saída: `fatorial = 120`.

### Decisões técnicas

- **C → JavaScript:** o destino permite executar o resultado com Node.js, mantendo o escopo da disciplina.
- **JS diretamente da AST:** o TAC usa rótulos e `goto`, que não existem em JavaScript. A AST permite emitir diretamente `if`, `else` e `while`.
- **Conversões numéricas:** `tipo_c` orienta o uso de `| 0`, `Math.imul`, `Math.fround` e `Math.trunc`. `long` tem precisão exata limitada a `2^53` no tipo `Number` do JavaScript.
- **Escopos:** nomes internos como `x$1` preservam o sombreamento sem colidir com identificadores C.
- **Otimização opcional:** `--otimizar` permite comparar a AST e as saídas com e sem constant folding.
- **printf:** o runtime é incluído somente quando o programa contém `printf`.

### Testes

Execute todas as baterias com:

```bash
make -C src test
```

O script `tests/run_all_tests.sh` reúne os testes do compilador/AST, AST isolada, scanner, TAC/otimização e JavaScript no Node.js. A bateria JS requer Node.js e compara a execução gerada com as respostas de referência em C. Para executar somente essa bateria:

```bash
tests/run_js_tests.sh
```

As respostas de referência podem ser regeneradas com `tests/gerar_respostas_js.sh` (requer GCC).

### Trabalhos futuros

- **Controle de fluxo:** `for`, `do-while`, `break` e `continue`.
- **Funções:** declaração/definição de funções, parâmetros, retorno, recursão e `main`; o subconjunto atual aceita instruções no nível superior.
- **Operadores e tipos:** `%`, `&&`, `||`, `!`, `bool`, `true` e `false`.
- **Strings como dados:** variáveis string, arrays de caracteres e manipulação de texto; strings são aceitas apenas como formato de `printf`.
- **Recursos de C:** ponteiros, alocação dinâmica, estruturas, pré-processador completo e demais funções da biblioteca padrão.

Esses recursos ampliariam a gramática, a semântica, a representação de dados ou o runtime além do prazo da disciplina. As simplificações `x + 0`, `x - 0`, `x * 1` e `x / 1` já estão implementadas em `--otimizar`.

### Recursos fora do escopo

A tabela a seguir lista os recursos que não farão parte do projeto e o motivo de cada exclusão.

| Recurso                                                                                             | Motivo                                                                                                 |
| --------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------ |
| Ponteiros (`int *p`, `*p`, `&x` fora do `scanf`, aritmética de ponteiros)                           | Exigem emular memória endereçável.                                                                     |
| `malloc`, `free`, alocação dinâmica                                                                 | Mesmo motivo.                                                                                          |
| `struct`, `union`, `enum`, `typedef`                                                                | `union` depende do layout em bytes; `struct`, `enum` e `typedef` foram cortados para reduzir o escopo. |
| `sizeof`                                                                                            | Depende do layout em bytes.                                                                            |
| `unsigned`, `signed`, `short`                                                                       | Aumentariam muito as regras de conversão, com pouco ganho.                                             |
| `goto`, rótulos, `setjmp`/`longjmp`                                                                 | Não existem em JavaScript.                                                                             |
| `static`, `extern`, `const`, `volatile`, `register`, `auto`                                         | São qualificadores e classes de armazenamento fora do modelo adotado.                                  |
| Strings como tipo, manipulação de strings, `<string.h>`                                             | Exigiriam arrays de `char` terminados em `\0` e ponteiros.                                             |
| Pré-processador completo: macros com parâmetros, `#if`, `#ifdef`, `#include` de arquivos do usuário | É uma ferramenta separada (`cpp`).                                                                     |
| Operador vírgula                                                                                    | Tem pouco uso e conflita com as listas de argumentos.                                                  |
| Funções variádicas do usuário, ponteiros para função                                                | Dependem de ponteiros.                                                                                 |
| Biblioteca padrão além de `printf`/`scanf` (`math.h`, `stdlib.h`, arquivos)                         | Está fora do objetivo do trabalho.                                                                     |
| `argc`/`argv`, threads, sinais                                                                      | Dependem do sistema operacional.                                                                       |

## Regras de tipos

A análise semântica calcula o tipo de cada expressão de baixo para cima, seguindo o C. A ordem dos tipos, do mais estreito ao mais largo, é `char < int < long < float < double`.

| Construção                  | Tipo resultante                                                |
| --------------------------- | -------------------------------------------------------------- |
| Literal inteiro / `long` / caractere | `int` / `long` / `char`                               |
| Literal real (`1.5`, `2e3`) | `double`, como no C                                            |
| Identificador               | o tipo declarado da variável                                   |
| `a + b`, `a - b`, `a * b`, `a / b` | o mais largo entre `a` e `b`, no mínimo `int` (`char + char` → `int`) |
| `-a`                        | o tipo de `a`, no mínimo `int`                                 |

Em atribuições (`x = e;`) e declarações com inicialização (`tipo x = e;`):

- Promoções são aceitas (ex.: `int → float`, `char → int`).
- Estreitamentos entre inteiros (`long → int`) e entre reais (`double → float`) também são aceitos, como no C.
- Só **real → inteiro** é erro: `Erro Semântico [Linha X, Col Y]: atribuição de 'double' para 'int' não permitida em 'a'`.

Uma expressão com erro (variável não declarada, divisão por literal zero) recebe o tipo `TIPO_ERRO`, que se propaga para as expressões que a contêm sem gerar novas mensagens. Assim, `a = x + 1;` com `x` não declarado gera um único erro.


## Código intermediário (TAC)

Com a opção `--tac`, o compilador gera o código de três endereços (TAC) do programa, depois da análise semântica. Cada instrução tem no máximo um operador; resultados intermediários ficam em temporárias `t0`, `t1`, ...

```bash
./parser --tac programa.c
```

Entrada:

```c
int a = 1; int b = 2; int c = 3; int y;
y = (a + b) * (c - 2);
```

Saída:

```text
a = 1
b = 2
c = 3
t0 = a + b
t1 = c - 2
t2 = t0 * t1
y = t2
```

- A atribuição copia o resultado direto para a variável (`y = t2`), sem temporária extra.
- Declaração sem inicialização (`int y;`) não gera instrução.
- O TAC só é gerado se o programa não tiver erros léxicos, sintáticos ou semânticos.
- `if`/`else` e `while` usam rótulos (`L0`, `L1`, ...) e saltos (`goto`, `ifFalse ... goto`).

Exemplo com `while`. Entrada:

```c
int i = 0;
while (i < 10) { i = i + 1; }
```

Saída:

```text
i = 0
L0:
t0 = i < 10
ifFalse t0 goto L1
t1 = i + 1
i = t1
goto L0
L1:
```

A condição fica logo depois do rótulo de início, para ser reavaliada a cada volta do laço.

O `printf` usa a forma padrão de chamada do TAC: cada parâmetro com `param` e a chamada com `call printf, N`, em que `N` conta também o formato. Os argumentos são todos calculados antes do primeiro `param`:

```text
t0 = a * 2              // printf("a = %d, dobro = %d\n", a, a * 2);
param "a = %d, dobro = %d\n"
param a
param t0
call printf, 3
```

### Testes de geração de TAC

Os casos de teste de código intermediário ficam em `tests/entradas_tac/` e suas saídas esperadas em `tests/respostas_tac/`. Para executar todos os testes do projeto (incluindo a bateria de TAC):

```bash
cd src
make test
```

Só a bateria de TAC:

```bash
./tests/run_tac_tests.sh
```

## Geração de JavaScript

Com a opção `--js`, o compilador traduz o programa para JavaScript, a linguagem-alvo do projeto. O código é gerado a partir da AST (já validada pela análise semântica), e não do TAC, porque o JavaScript não tem `goto`.

```bash
./parser --js programa.c > programa.js
node programa.js
```

Entrada:

```c
int a = 7;
int b = 2;
int c = a / b;
float f = 1.5;
double d;
d = f * 2;
c = -a + b * 3;
```

Saída:

```js
"use strict";
let a = 7;
let b = 2;
let c = ((a / b) | 0);
let f = Math.fround(1.5);
let d = 0;
d = Math.fround(f * 2);
c = (((-a | 0) + Math.imul(b, 3)) | 0);
```

O C tem vários tipos numéricos de tamanho fixo, e o JavaScript guarda todo número como um `double`. Para que o programa gerado calcule exatamente o mesmo que o programa em C, o gerador usa o tipo de cada expressão (anotado pela análise semântica):

| Tipo do resultado | Operação         | JavaScript gerado         | Motivo                                             |
| ----------------- | ---------------- | ------------------------- | -------------------------------------------------- |
| `int`, `char`     | `a + b`, `a - b` | `((a + b) \| 0)`          | trunca para 32 bits, como o estouro do `int`       |
| `int`, `char`     | `a * b`          | `Math.imul(a, b)`         | multiplicação exata em 32 bits                     |
| `int`, `char`     | `a / b`          | `((a / b) \| 0)`          | divisão inteira (`7 / 2` é `3`, e não `3.5`)       |
| `long`            | `a / b`          | `Math.trunc(a / b)`       | divisão inteira sem limitar a 32 bits              |
| `float`           | qualquer         | `Math.fround(...)`        | arredonda para a precisão do `float`               |
| `double`          | qualquer         | sem conversão             | já é o número do JavaScript                        |
| `int`             | `a < b`, `a == b`| `(a < b ? 1 : 0)`         | no C a comparação vale `1` ou `0`, não `true`/`false` |

Nas atribuições e declarações, o valor é convertido para o tipo da variável: `char` com `((v << 24) >> 24)`, `int` vindo de `long` com `(v | 0)` e `float` com `Math.fround(v)`.

- Declaração sem inicialização (`int x;`) vira `let x = 0;`: no JavaScript a variável ficaria `undefined`.
- Variáveis de escopos internos recebem o número do escopo no nome (`x$1`), no TAC e no JavaScript. O `$` não existe em identificadores C, então não há colisão com nomes do usuário.
- `char` vira o código numérico do caractere (`'a'` → `97`).
- O arquivo começa com `"use strict";`, que transforma em erro o uso de variável não declarada.
- Limitação: `long` é exato só até 2^53, o maior inteiro que um `double` representa sem perda.

Exemplo com `if`/`else` e `while`. Entrada:

```c
int i = 0;
int soma = 0;
while (i < 5) {
    if (i == 2) {
        soma = soma + 10;
    } else {
        soma = soma + i;
    }
    i = i + 1;
}
```

Saída:

```js
"use strict";
let i = 0;
let soma = 0;
while (i < 5) {
    if (i === 2) {
        soma = ((soma + 10) | 0);
    } else {
        soma = ((soma + i) | 0);
    }
    i = ((i + 1) | 0);
}
```

Nas condições de `if` e `while`, comparações saem diretamente (`i < 5`, `i === 2`), sem a conversão `? 1 : 0` usada em expressões atribuídas a variáveis.

A tradução foi verificada comparando, em cerca de 2.000 programas gerados aleatoriamente (com todos os tipos, operadores e conversões), o resultado do JavaScript no Node.js com o do mesmo programa compilado pelo `gcc`.

### `printf` e _runtime_ JS

O `printf` é traduzido para uma chamada a uma função `printf` de um pequeno _runtime_ em JavaScript. O _runtime_ é emitido no início do arquivo gerado só quando o programa usa `printf`. As linhas `#include <stdio.h>` e `#include <stdbool.h>` são ignoradas, então programas C comuns são aceitos. Qualquer outra diretiva (`#include <math.h>`, `#define`, ...) é erro léxico.

Entrada:

```c
#include <stdio.h>
int n = 5;
int fat = 1;
while (n > 1) {
    fat = fat * n;
    n = n - 1;
}
printf("fatorial = %d\n", fat);
printf("media = %f\n", 7 / 2.0);
```

Execução:

```bash
./parser --js programa.c > programa.js
node programa.js
```

Saída (idêntica à do mesmo programa compilado com `gcc`):

```text
fatorial = 120
media = 3.500000
```

| Especificador  | Tipo do argumento  | Saída                                      |
| -------------- | ------------------ | ------------------------------------------ |
| `%d`, `%i`     | `char`, `int`      | inteiro                                    |
| `%ld`, `%li`   | `long`             | inteiro                                    |
| `%c`           | `char`, `int`      | o caractere com aquele código              |
| `%f`, `%lf`    | `float`, `double`  | 6 casas decimais, como no C (`3.500000`)   |
| `%%`           | nenhum             | o próprio `%`                              |

- O formato é sempre um literal de string, com os escapes `\n`, `\t`, `\\` e `\"`. A linguagem não tem tipo string: `x = "oi";` é erro de sintaxe.
- A saída usa `process.stdout.write`, que não acrescenta quebra de linha (o `console.log` acrescentaria).
- A análise semântica confere o formato: especificador não suportado (`%5d`, `%.2f`, `%s`), número errado de argumentos e tipo incompatível (`%d` com `double`) são erros. Em C essas combinações são comportamento indefinido e imprimiriam lixo, o que impediria o JavaScript de imprimir o mesmo que o C.
- Limitações: em um empate exato no arredondamento do `%f` (ex.: `0.0078125`), o C arredonda para o par e o JavaScript para cima; valores com `%f` acima de 10^21 saem em notação científica no JavaScript.

### Testes de JavaScript (Node.js)

Os testes de ponta a ponta compilam cada programa de `tests/entradas_js/` com `--js`, executam o resultado no Node.js e comparam a saída com `tests/respostas_js/`. Cada programa roda duas vezes, sem e com `--otimizar`, para garantir que a otimização não muda o resultado.

**Requisito:** [Node.js](https://nodejs.org) instalado (comando `node` no `PATH`). Sem ele, a bateria avisa e é pulada, sem falhar o `make test`.

```bash
cd src
make test                    # todas as baterias, incluindo a de JavaScript
../tests/run_js_tests.sh     # só a bateria de JavaScript
```

As respostas esperadas são a saída do mesmo programa compilado com `gcc`, e não o que achamos que o C imprimiria. Ao criar ou alterar um teste, gere a resposta com o script abaixo e confira antes de commitar (requer `gcc`):

```bash
./tests/gerar_respostas_js.sh              # todos os testes
./tests/gerar_respostas_js.sh while_soma   # só um teste
```

Como as instruções da linguagem ficam fora de funções, o script coloca as linhas `#include` no topo e o resto do programa dentro de um `int main(void) { ... return 0; }` antes de compilar.

## Integrantes
 
<table align="center">
  <tr>
    <td align="center" width="160">
      <a href="https://github.com/Edilson-r-jr">
        <img src="https://github.com/Edilson-r-jr.png?size=200" width="100" alt="Foto de Edilson"/><br/>
        <b>Edilson Júnior</b>
      </a><br/>
      <sub>Edilson Ribeiro da Cruz Júnior</sub><br/>
      <sub>222024461</sub>
    </td>
    <td align="center" width="160">
      <a href="https://github.com/MarcusVRezende">
        <img src="https://github.com/MarcusVRezende.png?size=200" width="100" alt="Foto de Marcus"/><br/>
        <b>Marcus Rezende</b>
      </a><br/>
      <sub>Marcus Vinicius Rezende dos Santos</sub><br/>
      <sub>232038335</sub>
    </td>
    <td align="center" width="160">
      <a href="https://github.com/Maria-Laura-Regis">
        <img src="https://github.com/Maria-Laura-Regis.png?size=200" width="100" alt="Foto de Maria Laura"/><br/>
        <b>Maria Laura</b>
      </a><br/>
      <sub>Maria Laura Regis Cabral Dias</sub><br/>
      <sub>232005361</sub>
    </td>
    <td align="center" width="160">
      <a href="https://github.com/Kael-web7">
        <img src="https://github.com/Kael-web7.png?size=200" width="100" alt="Foto de Miqueias"/><br/>
        <b>Miqueias Carvalho</b>
      </a><br/>
      <sub>Miqueias Ezequiel Gonçalves Carvalho</sub><br/>
      <sub>232005450</sub>
    </td>
    <td align="center" width="160">
      <a href="https://github.com/PedroHenriqueCo">
        <img src="https://github.com/PedroHenriqueCo.png?size=200" width="100" alt="Foto de Pedro Henrique"/><br/>
        <b>Pedro Henrique</b>
      </a><br/>
      <sub>Pedro Henrique Conceição De Souza</sub><br/>
      <sub>232030032</sub>
    </td>
  </tr>
</table>
 
