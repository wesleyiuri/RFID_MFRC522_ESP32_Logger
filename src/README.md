# README da pasta `src/`

## Índice rápido
- Visão geral do projeto: ../README.md
- Índice de documentação: ../docs/README.md
- Relatório completo: ../docs/RELATORIO_PROJETO.md

## Objetivo
Esta pasta contém as implementações (arquivos `.cpp`) das classes e componentes declarados nos headers em `include/`. Mantemos as interfaces em `include/` e a lógica aqui para organização e reutilização.

## Conteúdo (principais arquivos)
- `main.cpp` — Ponto de entrada do firmware Arduino/ESP32.
- `AppController.cpp` — Orquestrador (FSM) do ciclo principal.
- `RfidReader.cpp` — Interface com o MFRC522 (SPI) + deduplicação.
- `NetManager.cpp` — Unidade de compilação associada ao gerenciador Wi‑Fi (lógica principal está inline no header).
- `HttpSender.cpp` — Envio HTTP/HTTPS do payload com UID e metadados.

## Como usar
- Compile o projeto pela environment `esp32dev` no PlatformIO (VS Code ou CLI). As dependências são resolvidas automaticamente.
- Ajustes de comportamento (ex.: `UID_BUFFER_CAPACITY=1024`, `DEDUP_INTERVAL_MS`) são feitos em `platformio.ini` (seção `build_flags`).

## Notas
- `PersistentStore.cpp` não existe: a persistência está implementada em `PersistentStore.h` via condicionais de compilação (`PERSIST_BUFFER`). Pode ganhar TU própria no futuro.
- Fluxo de dependências:
  - `main.cpp` → `AppController.cpp` → (`RfidReader.cpp`, `NetManager.h`, `HttpSender.cpp`, `UidBuffer.h`, `PersistentStore.h`).

## Próximos passos sugeridos
- Migrar parte de `NetManager` para `.cpp` se a lógica crescer.
- Implementar envio em lote de UIDs.
- Adicionar testes de `HttpSender` com cliente mockado.

---
Para visão geral e links por tópico, use o índice em `../docs/README.md`.
