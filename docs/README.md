# README da pasta `docs/`

## Índice rápido
- Visão geral do projeto: ../README.md
- Relatório completo (fonte única de detalhes): ./RELATORIO_PROJETO.md
- READMEs locais:
  - Headers/APIs: ../include/README
  - Implementações: ../src/README.md
- Diagramas (Mermaid):
  - Arquitetura: ./diagrams/architecture.mmd
  - AppController (visão/fluxos/FSM): ./diagrams/appcontroller.mmd · ./diagrams/appcontroller-interactions.mmd · ./diagrams/appcontroller-fsm.mmd
  - HttpSender: ./diagrams/httpsender.mmd
  - NetManager: ./diagrams/netmanager.mmd
  - PersistentStore: ./diagrams/persistentstore.mmd
  - RfidReader: ./diagrams/rfidreader.mmd
  - UidBuffer: ./diagrams/uidbuffer.mmd

## Objetivo
Centralizar a documentação aprofundada do projeto, decisões e diagramas. Os READMEs das pastas apenas resumem e encaminham para cá.

## Conteúdo (principais arquivos)
- `RELATORIO_PROJETO.md` — Guia completo de conceitos, arquitetura, componentes e apêndice com todas as funções.
- `diagrams/*.mmd` — Diagramas em Mermaid para arquitetura e componentes.

## Como usar
1) Para uma visão geral, comece no README raiz (../README.md) e retorne aqui para detalhes.
2) Leia `RELATORIO_PROJETO.md` nas seções:
   - O que é / Como funciona (visão geral para leigos).
   - Arquitetura em blocos e componentes (o que cada parte faz).
   - Apêndice: funções detalhadas (100% de cobertura por arquivo).
3) Navegue pelos diagramas conforme o componente que estiver trabalhando.
4) Nos READMEs de `include/`, `src/` e `test/`, use os links de “Índice rápido” para voltar a este hub quando precisar de mais contexto.

## Notas
- Plataforma e build: ../platformio.ini
- Configuração do dispositivo (não versionar segredos): `include/ProjectConfig.h` (modelo: ../include/ProjectConfig.example.h)

## Próximos passos sugeridos
- Adicionar âncoras estáveis nas seções do relatório para deep-links diretos (ex.: segurança, troubleshooting, cada componente).
- Considerar publicação com MkDocs/Docusaurus + GitHub Pages para navegação e busca.

---
Este índice é a referência central da documentação. Para operação rápida, use o README na raiz; para detalhes, utilize `RELATORIO_PROJETO.md` e os diagramas.
