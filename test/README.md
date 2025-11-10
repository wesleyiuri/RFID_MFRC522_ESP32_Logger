# README da pasta `test/`

## Índice rápido
- Visão geral do projeto: ../README.md
- Índice de documentação: ../docs/README.md
- Relatório completo: ../docs/RELATORIO_PROJETO.md

## Objetivo
Esta pasta contém os testes do projeto (PlatformIO + Unity) para validar componentes de forma automatizada, preferencialmente sem depender do hardware.

## Conteúdo (pastas de teste)

## Como usar
- VS Code (PlatformIO): abra a aba de testes e execute na environment `esp32dev`.
- CLI:
	```powershell
	pio test -e esp32dev
	```

## Notas
- Para diagnósticos, aumente `LOG_LEVEL` nas `build_flags` do `platformio.ini`.
- Para testar RFID sem hardware, crie camadas de abstração ou compile blocos com macros de teste (ex.: simulando `isDuplicate`).

## Próximos passos sugeridos
- Adicionar testes para `HttpSender` usando um cliente HTTP mockado.
- Incluir testes de reconexão de `NetManager` com simulação de estados.
- Cobrir persistência (`PersistentStore`) com um namespace NVS de teste para não conflitar com produção.

---
Para visão geral e links por tópico, use o índice em `../docs/README.md`.
