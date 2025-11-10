/*
    Arquivo: include/HttpSender.h
    Propósito: Declara a classe HttpSender responsável por montar e enviar via
    HTTP/HTTPS os dados de UID lidos, com metadados do dispositivo e política de
    retry exponencial para códigos 429/5xx ou erros de transporte.
*/

#pragma once // Garante inclusão única deste cabeçalho durante a compilação
#include <Arduino.h> // String, millis
#include <WiFi.h> // Estado de conexão
#include <WiFiClientSecure.h> // HTTPS
#include <HTTPClient.h> // Cliente HTTP do Arduino
#include <time.h> // time(), gmtime_r, strftime
#include "Log.h" // Macros de log
#include "UidBuffer.h" // UidEntry com uid/capture_ms
// Configuração: tenta usar include/ProjectConfig.h (local, ignorado no Git), ou fallback para include/ProjectConfig.example.h
#if defined(__has_include)
#  if __has_include("ProjectConfig.h")
#    include "ProjectConfig.h" // Constantes de configuração definidas pelo usuário
#  else
#    include "ProjectConfig.example.h" // Fallback para CI/builds padrão
#  endif
#else
#  include "ProjectConfig.h" // Constantes de configuração definidas pelo usuário
#endif

// Cliente HTTP/HTTPS responsável por montar payloads e enviar UIDs com retries
class HttpSender { // Início da definição da classe HttpSender
public: // Seção pública: API exposta a outros módulos
    explicit HttpSender(uint32_t timeoutMs = HTTP_TIMEOUT_MS); // Define timeouts do cliente
    bool postUid(const UidEntry &entry); // Envia 1 entrada; true em HTTP 2xx
private: // Seção privada: detalhes internos não expostos
    bool performPost(const String &payload, const String &url, int &httpCode); // Executa POST
    bool shouldRetry(int httpCode, uint8_t attempt) const; // Decide retry por código/erro
private: // Campos privados
    uint32_t _timeout; // Timeout em ms para conexão e requisição
}; // Fim da classe HttpSender
