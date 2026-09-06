# Relatório Semanal - Semana 3

**Projeto:** Compilador (comp1-2026.2-grupo17)  
**Disciplina:** Compiladores  

---

## Objetivos da Semana

* Desenvolver e refinar as regras do analisador léxico (`scanner.l`) com a ferramenta Flex.
* Criar e estruturar o pipeline de testes automatizados para validação do compilador.
* Atualizar a documentação técnica do projeto com guias de execução e relatórios de progresso da equipe.

---

## Issues Fechadas na Semana 3

### 1. Issue #2: `Semana 3 - dev scanner.l` (enhancement)
* **Status:** Concluída
* **Atividades Desenvolvidas:**
  * Implementação e ajuste das regras de expressões regulares no arquivo `src/scanner.l` para reconhecimento dos tokens da linguagem.
  * Tratamento e descarte de caracteres em branco (espaços, tabulações e quebras de linha `\r` e `\n`).
  * Emissão de mensagens descritivas para caracteres inválidos detectados na entrada léxica.

### 2. Issue #3: `Semana 3 - Test`
* **Status:** Concluída
* **Atividades Desenvolvidas:**
  * Criação do script de automação de testes em lote (`run_tests.sh`) dentro do diretório `tests/`.
  * Geração automática do arquivo `relatorio.txt` contendo a listagem detalhada de diferenças (`diff`) para testes que falharam.

### 3. Issues #4 e #5: `Semana 3 - Documentação`
* **Status:** Concluídas
* **Atividades Desenvolvidas:**
  * Elaboração do `README_TESTES.md` com instruções detalhadas sobre como compilar via `make` e rodar a suíte de testes[cite: 1].
  * Documentação da nova arquitetura de pastas do repositório (separação de `src/`, `tests/` e `Docs/`).
  * Atualização dos relatórios de acompanhamento semanal e templates de contribuição (`pull_request_template.md`).

---

## Dificuldades Encontradas e Soluções Adotadas

* Nenhuma dificuladade relatada.

---

## Próximos Passos (Semana 4)

* **Issues planejadas para a próxima semana:**
 * Tipos e Novos operadores: Atribuição (=) e ponto e virgula (;)
 * Testes: Aritmética básica e atribuição de variáveis
 * Criar um MAKEFILE para o compilador
 * Documentação Semana 04