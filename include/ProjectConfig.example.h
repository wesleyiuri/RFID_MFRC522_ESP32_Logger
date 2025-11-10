/*
    Arquivo: include/config.example.h
    Propósito: Exemplo de configuração (sem segredos) a ser copiado para
    include/ProjectConfig.h e ajustado conforme seu ambiente (Wi‑Fi, endpoint, pinos,
    metadados e HTTPS). Este arquivo pode ser versionado.
*/

#pragma once // Garante inclusão única deste cabeçalho durante a compilação

// Copie este arquivo para include/ProjectConfig.h e preencha suas credenciais.
// NÃO versione include/ProjectConfig.h (está no .gitignore) para proteger segredos.

// Credenciais Wi‑Fi
#define WIFI_SSID "YOUR_WIFI_SSID" // SSID da rede Wi‑Fi
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD" // Senha da rede Wi‑Fi

// Endpoint principal HTTP/HTTPS para enviar UIDs via POST (genérico)
// Exemplos:
//  - https://api.seusistema.com/entrada-uid
//  - https://webhook.site/<uuid> (apenas para testes rápidos)
#define HTTP_ENDPOINT_URL "https://example.com/api/uid" // URL do endpoint

// Opcional: timeout de HTTP em milissegundos
#define HTTP_TIMEOUT_MS 5000 // Timeout do HTTPClient (ms)

// Mapeamento de pinos SPI para MFRC522 no ESP32
// Pinos padrão conforme especificação do projeto
#define PIN_SDA 5 // Pino SDA (SS) do leitor MFRC522
#define PIN_SCK 18 // Pino SCK (clock) do SPI
#define PIN_MOSI 23 // Pino MOSI do SPI
#define PIN_MISO 19 // Pino MISO do SPI
#define PIN_RST 17 // Pino RST do MFRC522

// Metadados do dispositivo/origem (preencha conforme seu ambiente)
#ifndef DEVICE_ID // Permite sobrescrever por build_flags
#define DEVICE_ID "esp32-leitor-01" // Identificador único do dispositivo/leitor
#endif // fim: DEVICE_ID

#ifndef DEVICE_SITE // Site/filial de origem
#define DEVICE_SITE "fabrica-a" // Nome/código do site
#endif // fim: DEVICE_SITE

#ifndef DEVICE_UNIT // Unidade/linha dentro do site
#define DEVICE_UNIT "unidade-1" // Nome/código da unidade
#endif // fim: DEVICE_UNIT

#ifndef DEVICE_SECTOR // Setor/área específica
#define DEVICE_SECTOR "setor-1" // Nome/código do setor
#endif // fim: DEVICE_SECTOR

#ifndef FW_VERSION // Versão de firmware (sobrescrevível via build_flags)
#define FW_VERSION "1.0.0" // Versão do firmware
#endif // fim: FW_VERSION

#ifndef DEVICE_OPERATOR_ID // Identificador de operador
#define DEVICE_OPERATOR_ID "operador-001" // ID do operador do dispositivo
#endif // fim: DEVICE_OPERATOR_ID

// =============================
// Segurança HTTPS (opcional e recomendada em produção)
// HTTPS_SECURITY_MODE: 0 = inseguro (apenas DEV), 1 = validar CA do servidor
#ifndef HTTPS_SECURITY_MODE // Política de segurança HTTPS
#define HTTPS_SECURITY_MODE 0 // Valor padrão (DEV)
#endif // fim: HTTPS_SECURITY_MODE default

// Se usar HTTPS_SECURITY_MODE = 1, cole abaixo a CA raiz/intermediária (PEM)
// da sua API/servidor. Observação: obtenha a CA do servidor (não o cert. do host).
#if HTTPS_SECURITY_MODE == 1 // Ativa validação de CA
static const char HTTPS_CA_CERT_PEM[] PROGMEM = R"EOF( // Início do certificado PEM
-----BEGIN CERTIFICATE----- // Delimitador de início
MIID...SEU_CERTIFICADO_CA_AQUI...IDAQAB // Conteúdo da CA (exemplo)
-----END CERTIFICATE----- // Delimitador de fim
)EOF"; // Fim do bloco PEM
#endif // fim: bloco CA condicional
