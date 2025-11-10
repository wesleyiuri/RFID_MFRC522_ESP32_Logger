# README da pasta `lib/`

## Índice rápido
- Visão geral do projeto: ../README.md
- Índice de documentação: ../docs/README.md
- Relatório completo: ../docs/RELATORIO_PROJETO.md

## Objetivo
Esta pasta é reservada para bibliotecas privadas do projeto (código reutilizável que não vem por `lib_deps`). O PlatformIO compila cada subpasta de `lib/` como uma biblioteca estática e a vincula ao firmware.

## Estrutura sugerida
```
lib/
  MinhaLib/
    src/
      MinhaLib.cpp
      MinhaLib.h
    library.json   # (opcional) metadados e configurações da lib
```

## Como usar
- Coloque o código reutilizável em subpastas dentro de `lib/` (uma pasta por biblioteca).
- Inclua os headers normalmente a partir de `src/*.cpp`:
  ```cpp
  #include <MinhaLib.h>
  ```
- O LDF (Library Dependency Finder) do PlatformIO resolve dependências automaticamente.

## Notas
- Para dependências de terceiros, prefira `lib_deps` no `platformio.ini` (mais fácil de atualizar e reproduzir em outros ambientes).
- Use `lib/` para código proprietário do projeto, utilitários comuns entre módulos ou camadas de abstração (ex.: mocks para testes).
- Referência do LDF: https://docs.platformio.org/page/librarymanager/ldf.html

## Próximos passos sugeridos
- Se a biblioteca crescer, adicione um `library.json` com nome, versão e compatibilidade.
- Organize testes específicos da biblioteca em `test/` com casos focados na API pública da lib.
- Documente a API pública das bibliotecas internas diretamente nos headers (Doxygen/observações).

---
Para visão geral e links por tópico, use o índice em `../docs/README.md`.
