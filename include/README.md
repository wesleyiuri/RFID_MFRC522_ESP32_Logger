# README da pasta `include/`

## Índice rápido
- Visão geral do projeto: ../README.md
- Índice de documentação: ../docs/README.md
- Relatório completo: ../docs/RELATORIO_PROJETO.md

## Objetivo
Esta pasta contém os cabeçalhos (headers) públicos do projeto. Eles expõem a API utilizada pelas implementações em `src/` e concentram a documentação das interfaces.

## Conteúdo (principais arquivos)
- `AppController.h` — Orquestrador (FSM) do firmware.
- `RfidReader.h` — Leitura MFRC522 + deduplicação por UID (cache + janela).
- `RfidDedupCache.h` — Componente de deduplicação testável (sem hardware).
- `NetManager.h` — Wi‑Fi com backoff e callbacks.
- `HttpSender.h` — Envio HTTP/HTTPS com retries.
- `UidBuffer.h` — Buffer circular fixo (ring buffer) em RAM.
- `PersistentStore.h` — Persistência NVS opcional (ativável por flag).
- `Log.h` — Macros de log por nível.
- `ProjectConfig.h` — Configurações locais (Wi‑Fi, endpoint, pinos, metadados). NÃO versionar; baseie‑se em `ProjectConfig.example.h`.

## Como usar
- Inclua os headers necessários nos seus `.cpp` em `src/`:
	```cpp
	#include "AppController.h"
	```
- Crie `include/config.h` a partir de `include/config.example.h` e preencha credenciais e metadados do dispositivo. O arquivo real com segredos não deve ser commitado (está no `.gitignore`).

## Notas
- Várias opções de comportamento são definidas via `build_flags` no `platformio.ini` (ex.: `UID_BUFFER_CAPACITY`, `DEDUP_INTERVAL_MS`, `LOG_LEVEL`).
- Em produção, prefira `HTTPS_SECURITY_MODE=1` em `config.h` com a CA adequada.

## Próximos passos sugeridos
1. Extrair implementações maiores (ex.: partes de `NetManager` ou futuras expansões de `PersistentStore`) para `.cpp` dedicados se o tamanho crescer.
2. Criar interface de abstração para o leitor RFID (ex.: `IRfidDriver`) para facilitar testes sem hardware.
3. Acrescentar um header `Version.h` gerado em build para embutir hash de commit e data.

---
Para visão geral e links por tópico, use o índice em `../docs/README.md`.
