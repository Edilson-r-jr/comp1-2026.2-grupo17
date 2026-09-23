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
| Literais de string                                                    |   ⬜   |
| Comentários de linha e de bloco                                       |   ✅   |
| `;` como fim de instrução                                             |   ✅   |
| Declaração com e sem inicialização (`tipo x;`, `tipo x = e;`)         |   ✅   |
| Várias variáveis por declaração (`int a, b;`)                         |   ⬜   |
| Tabela de símbolos: erros de redeclaração e de variável não declarada |   ✅   |
| Operadores `+ - * /` e `-` unário, com promoção de tipos              |   ✅   |
| `%`, operadores relacionais e lógicos, `!`                            |   ⬜   |
| `if`/`else`, `while`, `do-while`, `for`, `break`, `continue`          |   ⬜   |
| Blocos e pilha de escopos                                             |   ⬜   |
| Funções, `return`, recursão, `main`                                   |   ⬜   |
| `printf` e _runtime_ JS                                               |   ⬜   |
| Ignorar `#include <stdio.h>` e `#include <stdbool.h>`                 |   ⬜   |
| **Geração de código JavaScript**                                      |   ⬜   |
| Erro explícito de "recurso fora do escopo"                            |   ⬜   |

✅ implementado · ⬜ pendente

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

**Integrantes**

**Nome:** Edilson Ribeiro da Cruz Júnior

**Matricula:** 222024461

**GitHub:** (https://github.com/Edilson-r-jr)

<br>

**Nome:** Marcus Vinicius Rezende dos Santos

**Matricula:** 232038335

**GitHub:** (https://github.com/MarcusVRezende)

<br>

**Nome:** Maria Laura Regis Cabral Dias

**Matricula:** 232005361

**GitHub:** (https://github.com/Maria-Laura-Regis)

<br>

**Nome:** Miqueias Ezequiel Gonçalves Carvalho

**Matricula:** 232005450

**GitHub:** (https://github.com/Kael-web7)

<br>

**Nome:** Pedro Henrique Conceição De Souza

**Matricula:** 232030032

**GitHub:** (https://github.com/PedroHenriqueCo)
