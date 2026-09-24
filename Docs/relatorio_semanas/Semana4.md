# Relatório Semanal - Semana 4

**Projeto:** Compilador (comp1-2026.2-grupo17)

**Disciplina:** Compiladores

## Objetivos da Semana

- Reorganizar a arquitetura do repositório e padronizar o processo de contribuição com templates de PR.
- Implementar a automação de compilação do compilador através de um `Makefile` na pasta `src/`.
- Aprimorar o script de testes (`run_tests.sh`) para suportar verificação em lote com distinção entre testes de sucesso (`stdout`) e erro (`stderr`).
- Expandir a cobertura de testes para operações de aritmética básica, precedência, associatividade e casos de borda.
- Implementar a primeira versão da Tabela de Símbolos (`tabela.h`) e suporte aos tipos primitivos (`char`, `float`, `double`, `long`).
- Atualizar os analisadores léxico e sintático para introduzir o terminador de instrução ponto e vírgula (`;`), declaração de variáveis e validação de escopo básico.

## Issues Fechadas na Semana 4

### 1. Issue #11: `Reorganização do repositório + Nova implementação do script de teste`

- **Status:** Concluída
- **Atividades Desenvolvidas:**
    - Reestruturação das pastas do repositório para melhor divisão de responsabilidades.
    - Adição do template de Pull Request (`pull_request_template.md`) padronizando revisões de código.
    - Implementação do `Makefile` no diretório `src/` para geração automatizada do executável a partir das dependências do código.
    - Reformulação do script `tests/run_tests.sh` para diferenciar checagens de saída esperada (`tests/entradas/` e `tests/respostas/`) de validações de mensagens de erro (`tests/entradas_erro/` e `tests/respostas_erro/`), gerando o log `tests/relatorio.txt`.

### 2. Issue #12: `Testes: Aritmética básica e atribuição de variáveis`

- **Status:** Concluída
- **Atividades Desenvolvidas:**
    - Implementação de testes para todas as operações aritméticas elementares (adição, subtração, multiplicação e divisão exata).
    - Validação de precedência de operadores (`2 + 3 * 4 = 14`), associatividade à esquerda (`10 - 3 - 2 = 5`), agrupamentos por parênteses (`(2 + 3) * 4 = 20`) e expressões encadeadas longas.
    - Validação estrita do erro semântico de divisão por zero (`teste02`).
    - Mapeamento do comportamento das futuras instruções de variáveis por meio de testes de rejeição sintática: atribuição simples (`x = 10;`), atribuição com expressões (`x = 2 + 3 * 4;`) e declaração com tipo (`int x = 5;`).
    - Suíte de testes homologada com 100% de sucesso (11 testes executados, 0 falhas).

### 3. Issue #9: `Tipos e Novos operadores: Atribuição (=) e ponto e virgula (;)`

- **Status:** Concluída
- **Atividades Desenvolvidas:**
    - Criação da Tabela de Símbolos (`tabela.h`) contendo o `enum` com os tipos suportados (`TIPO_CHAR`, `TIPO_FLOAT`, `TIPO_DOUBLE`, `TIPO_LONG`), a estrutura `struct Simbolo` e a implementação das rotinas de inserção com detecção de redeclaração (`inserir_variavel`) e consulta (`verificar_tipo`).
    - Atualização do analisador léxico para identificar as palavras-reservadas dos tipos primitivos (`char`, `float`, `double`, `long`), o operador de atribuição (`=`), o caractere delimitador (`;`) e expressões regulares para literais complexos (como caracteres entre aspas e números de ponto flutuante).
    - Atualização do analisador sintático para incluir `tabela.h`, declarar novos tokens, substituir a quebra de linha `\n` pelo delimitador `;` no controle de instruções e adicionar regras gramaticais para declaração de variáveis com validação de uso prévio.

## Dificuldades Encontradas e Soluções Adotadas

* Nenhuma dificuldade relatada.

## Próximos Passos (Semana 5)

- **Consolidação do pipeline entre Flex e Bison com foco em:**
    - Integração de novos tokens léxicos com a gramática no analisador sintático.
    - Implementação de regras de derivação para estruturas de decisão (`if`, `else`) e blocos de comandos.
    - Padronização do tratamento de erros léxicos e sintáticos com indicação de linha e coluna.
    - Consolidação da documentação de escopo e garantia de cobertura por testes automatizados.