# Compilador C para JavaScript — v1.0

## Visão geral

A versão 1.0 reúne as funcionalidades implementadas pelo grupo
no projeto Compiladores 1, incluindo análise léxica, sintática,
semântica, construção de AST, otimização e geração de código.

## Funcionalidades

- Análise léxica e sintática.
- Análise semântica com acúmulo de diagnósticos.
- Construção e visualização da AST.
- Tabela de símbolos e controle de escopos.
- Geração de Código de Três Endereços (TAC).
- Otimização por constant folding.
- Geração de JavaScript.
- Suporte a estruturas condicionais e de repetição implementadas.
- Suporte a printf e runtime JavaScript.

## Arquivos da versão

- parser-linux-x86_64: executável Linux x86-64.
- parser.exe: executável Windows x86-64.

## Testes

- Linux: executável testado com geração de JavaScript e execução
  do exemplo de fatorial usando Node.js.
- Windows: executável PE32+ gerado por compilação cruzada com
  MinGW-w64. A execução não pôde ser validada neste ambiente
  devido ao bloqueio da política de Controle de Aplicativo do Windows.

## SHA-256

| Arquivo | SHA-256 |
|---|---|
| parser-linux-x86_64 | 0420f943ae24d294a2f49c140b79cd96433f99bbd26810a0fa819e6eba61bd9d |
| parser.exe | ed03112d3e104e23ee9d034e7e7132d543eb737acd0232dec30c741e1cd739e6 |

## Requisitos

- Linux x86-64 ou Windows x86-64, conforme o executável.
- Node.js para executar os arquivos JavaScript gerados.
