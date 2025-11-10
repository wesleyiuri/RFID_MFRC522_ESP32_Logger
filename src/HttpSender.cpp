/*
    Arquivo: src/HttpSender.cpp
    Propósito: Implementa o envio HTTP/HTTPS das leituras (UidEntry), construindo
    o payload JSON com metadados e aplicando retries exponenciais em falhas
    transitórias. Suporta HTTPS com validação de CA ou modo inseguro (DEV).
*/

#include "HttpSender.h" // Declarações da classe
#include "UidBuffer.h" // Estrutura UidEntry
#include "Log.h" // Macros de log

// Construtor: define o timeout (ms) aplicado às operações do HTTPClient
HttpSender::HttpSender(uint32_t timeoutMs) : _timeout(timeoutMs) {} // Inicialização do campo

// postUid(): envia um único UidEntry; monta payload JSON e aplica retries
bool HttpSender::postUid(const UidEntry &entry) { // Envia um único UidEntry
    if (WiFi.status() != WL_CONNECTED) return false; // Sem rede, aborta cedo
#ifndef HTTP_ENDPOINT_URL // Se a URL não está definida em config
    return false; // Endpoint não configurado
#else // Caso a URL exista
    // Monta payload JSON com metadados
    time_t now = time(nullptr); // Obtém tempo atual (se RTC/NTP configurado)
    bool hasTime = (now > 1609459200); // Considera válido > 2021-01-01
    char iso[25] = {0}; // Buffer para ISO-8601
    if (hasTime) { // Se o tempo for confiável
        struct tm tmUTC; // Estrutura tm para UTC
        gmtime_r(&now, &tmUTC); // Converte para UTC (thread-safe)
        strftime(iso, sizeof(iso), "%Y-%m-%dT%H:%M:%SZ", &tmUTC); // ISO-8601
    }

    String payload = "{"; // Monta JSON manualmente (leve e sem alocação extra)
    payload += "\"uid\":\""; payload += entry.uid; payload += "\","; // Campo uid
    payload += "\"capture_timestamp_ms\":"; payload += entry.capture_ms; payload += ','; // ts de captura
    payload += "\"timestamp_ms\":"; payload += millis(); payload += ','; // ts local do envio
    payload += "\"timestamp_iso\":\""; payload += hasTime ? iso : ""; payload += "\","; // ISO-8601
    payload += "\"device_id\":\""; payload += DEVICE_ID; payload += "\","; // ID do dispositivo
    payload += "\"site\":\""; payload += DEVICE_SITE; payload += "\","; // Site (fecha aspas antes da vírgula)
    payload += "\"unit\":\""; payload += DEVICE_UNIT; payload += "\","; // Unidade (fecha aspas antes da vírgula)
    payload += "\"sector\":\""; payload += DEVICE_SECTOR; payload += "\","; // Setor
    payload += "\"firmware_version\":\""; payload += FW_VERSION; payload += "\","; // FW
    payload += "\"operator_id\":\""; payload += DEVICE_OPERATOR_ID; payload += "\""; // Operador
    payload += '}'; // Fecha JSON

    int code = -1; // Código HTTP resultante
    String url = String(HTTP_ENDPOINT_URL); // URL de destino
    uint8_t maxRetries = (uint8_t)HTTP_RETRY_MAX; // Tentativas extras
    uint16_t baseDelay = (uint16_t)HTTP_RETRY_BASE_DELAY_MS; // Base de backoff

    for (uint8_t attempt = 0; attempt <= maxRetries; ++attempt) { // Loop de tentativas
        if (!performPost(payload, url, code)) { // Faz POST efetivo
            code = -1; // Erro no cliente/transporte
        }
        if (code >= 200 && code < 300) { // Sucesso 2xx
            LOG_INFO("HTTP %d", code); // Log de sucesso
            return true; // Retorna sucesso
        }
        if (attempt == maxRetries || !shouldRetry(code, attempt)) break; // Não cabe retry
        uint16_t waitMs = baseDelay * (1 << attempt); // Backoff exponencial simples
        LOG_DEBUG("Retry HTTP em %ums (tentativa %u)", waitMs, (unsigned)attempt+1); // Log de retry
        delay(waitMs); // Backoff bloqueante (curto)
    }
    if (code <= 0) LOG_ERROR("POST erro (client) code=%d", code); // Erro de transporte
    else LOG_ERROR("HTTP falhou code=%d", code); // HTTP != 2xx e sem retry
    return false; // Falha final
#endif // HTTP_ENDPOINT_URL
} // fim: postUid()

// performPost(): executa POST via HTTPClient (HTTPS/HTTP) com cabeçalhos e retorno do status
bool HttpSender::performPost(const String &payload, const String &url, int &code) { // Executa POST com HTTPClient
    HTTPClient http; // Instância do cliente HTTP
    http.setConnectTimeout(_timeout); // Timeout de conexão
    http.setTimeout(_timeout); // Timeout da requisição
    if (url.startsWith("https://")) { // Caminho HTTPS
        WiFiClientSecure sclient; // Cliente TLS
#if HTTPS_SECURITY_MODE == 1 // HTTPS com validação de CA
        #ifdef HTTPS_CA_CERT_PEM // Se a CA foi fornecida
        if (!sclient.setCACert(HTTPS_CA_CERT_PEM)) { // Carrega CA em PEM
            LOG_ERROR("Falha ao carregar CA"); // Loga falha
            return false; // Aborta
        }
        #else // Sem CA definida
        LOG_ERROR("CA não definida (HTTPS_SECURITY_MODE=1)"); // Alerta de configuração
        return false; // Aborta (não segue inseguro)
    #endif // HTTPS_CA_CERT_PEM
#else // HTTPS sem validação (DEV)
    sclient.setInsecure(); // Sem validação de CA (apenas DEV)
    LOG_DEBUG("HTTPS inseguro (DEV)"); // Aviso de modo DEV
#endif // fim: HTTPS_SECURITY_MODE
        if (!http.begin(sclient, url)) { // Abre sessão HTTPS
            LOG_ERROR("begin HTTPS falhou"); // Falha ao iniciar
            return false; // Aborta
        }
        http.addHeader("Content-Type", "application/json"); // Define cabeçalho
        code = http.POST(payload); // Executa POST
        http.end(); // Libera recursos
    } else { // Caminho HTTP simples
        WiFiClient nclient; // Cliente TCP
        if (!http.begin(nclient, url)) { // Abre sessão HTTP
            LOG_ERROR("begin HTTP falhou"); // Falha ao iniciar
            return false; // Aborta
        }
        http.addHeader("Content-Type", "application/json"); // Define cabeçalho
        code = http.POST(payload); // Executa POST
        http.end(); // Libera recursos
    }
    return true; // Sinaliza que a requisição foi tentada
} // fim: performPost()

// shouldRetry(): define regras de retry para códigos/transporte e tentativas
bool HttpSender::shouldRetry(int httpCode, uint8_t attempt) const { // Regras de retry
    if (httpCode < 0) return true; // Erro de transporte: tentar de novo
    if (httpCode == 429) return true; // Rate limited: aguardar e tentar
    if (httpCode >= 500 && httpCode < 600) return true; // Erro 5xx do servidor
    return false; // Outros códigos: não insistir
} // fim: shouldRetry()
