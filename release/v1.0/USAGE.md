# Guia de utilização — Compilador v1.0

## Linux

1. Baixe o arquivo `parser-linux-x86_64`.
2. Dê permissão de execução:

   ```bash
   chmod +x parser-linux-x86_64
   ```

3. Gere o JavaScript:

   ```bash
   ./parser-linux-x86_64 --js programa.c > programa.js
   ```

4. Execute com Node.js:

   ```bash
   node programa.js
   ```

## Windows

1. Baixe o arquivo `parser.exe`.
2. Coloque-o na mesma pasta do arquivo C.
3. Abra o PowerShell nessa pasta.
4. Gere o JavaScript:

   ```powershell
   .\parser.exe --js programa.c > programa.js
   ```

5. Execute:

   ```powershell
   node programa.js
   ```

A execução depende das políticas de segurança do Windows. Caso o Controle de Aplicativo bloqueie o executável, consulte o administrador.

## Observações

- O parser recebe o caminho do arquivo C como argumento.
- A opção `--js` imprime JavaScript na saída padrão.
- O operador `>` salva essa saída em um arquivo.
- Node.js é necessário para executar o JavaScript gerado.
- Consulte o README para outras opções: `--ast`, `--tabela`, `--tac` e `--otimizar`.