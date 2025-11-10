/*
    Arquivo: include/NetManager.h
    Propósito: Declara o gerenciador de rede Wi‑Fi com reconexão (backoff exponencial
    com jitter) e callbacks de conexão/desconexão, mantendo a aplicação responsiva.
*/

#pragma once // Garante inclusão única deste cabeçalho durante a compilação
#include <Arduino.h> // millis(), random(), tipos Arduino
#include <WiFi.h> // API Wi‑Fi do ESP32
#include <functional> // std::function para callbacks
#include <algorithm> // std::max (se necessário)
#include "Log.h" // Macros de log (INFO/DEBUG/ERROR)
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

// Gerenciador da conectividade Wi‑Fi (reconexão com backoff e callbacks)
class NetManager { // Início da definição da classe NetManager
public: // Seção pública: API exposta a outros módulos
    // Construtor: configura tempos de backoff e estado inicial
    NetManager(unsigned long baseRetryMs = 2000, unsigned long maxRetryMs = 30000) // Construtor com tempos padrão
        : _baseRetry(baseRetryMs), // Tempo base para backoff exponencial (ms)
        _maxRetry(maxRetryMs), // Tempo máximo entre tentativas (ms)
        _currentWait(baseRetryMs), // Janela atual de espera para próxima tentativa (ms)
        _lastAttempt(0), // Timestamp (ms) da última tentativa
        _onConnect(nullptr), // Callback para evento de conexão (opcional)
        _onDisconnect(nullptr), // Callback para evento de desconexão (opcional)
        _wasConnected(false) {} // Memória do estado anterior (conectado?)

    // Inicia o Wi‑Fi em modo estação e dispara a primeira tentativa de conexão
    void begin() { // Configura Wi‑Fi e inicia a primeira tentativa
        WiFi.mode(WIFI_STA); // Define modo estação (cliente)
        WiFi.setAutoReconnect(false); // Desliga auto-reconexão para controlar backoff manualmente
        WiFi.persistent(false); // Evita gravar credenciais na flash
        attemptConnect(); // Primeira tentativa imediata de conexão
    }

    // Deve ser chamado no loop principal; gerencia transições e reconexão com backoff
    void loop() { // Chamar frequentemente no loop principal
        bool connected = isConnected(); // Captura estado atual da conexão
        if (connected && !_wasConnected) { // Transição: desconectado -> conectado
            LOG_INFO("Wi-Fi conectado: %s", WiFi.localIP().toString().c_str()); // Loga IP local
            _currentWait = _baseRetry; // Reseta janela de backoff ao básico
            if (_onConnect) _onConnect(); // Dispara callback de conexão (se definido)
        } else if (!connected && _wasConnected) { // Transição: conectado -> desconectado
            LOG_ERROR("Wi-Fi desconectado"); // Loga perda de conexão
            if (_onDisconnect) _onDisconnect(); // Dispara callback de desconexão (se definido)
        }
        _wasConnected = connected; // Atualiza memória do estado anterior
        if (connected) return; // Se conectado, não tenta reconectar
        unsigned long now = millis(); // Leitura do tempo atual (ms desde boot)
        if (now - _lastAttempt >= _currentWait) { // Se já passou a janela de espera
            attemptConnect(); // Faz nova tentativa de conexão
            backoffGrow(); // Cresce janela com backoff e jitter
        }
    }

    // Informa se o Wi‑Fi está conectado (status = WL_CONNECTED)
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; } // Retorna true se Wi‑Fi está conectado

    // Registra callback a ser chamado quando a conexão for estabelecida
    void onConnect(const std::function<void()> &cb) { _onConnect = cb; } // Registra callback de conexão
    // Registra callback a ser chamado quando a conexão for perdida
    void onDisconnect(const std::function<void()> &cb) { _onDisconnect = cb; } // Registra callback de desconexão

private: // Seção privada: detalhes internos não expostos
    unsigned long _baseRetry; // Tempo base para primeira/menores esperas (ms)
    unsigned long _maxRetry; // Teto de espera entre tentativas (ms)
    unsigned long _currentWait; // Espera atual antes da próxima tentativa (ms)
    unsigned long _lastAttempt; // Momento (ms) da última chamada de conexão
    std::function<void()> _onConnect; // Callback para quando conectar
    std::function<void()> _onDisconnect; // Callback para quando desconectar
    bool _wasConnected; // Estado anterior de conexão

    // Executa uma tentativa de conexão e atualiza o timestamp da última tentativa
    void attemptConnect() { // Realiza uma tentativa de conexão
        _lastAttempt = millis(); // Marca hora da tentativa (ms)
        LOG_DEBUG("Tentando conectar Wi‑Fi (espera=%lums)", _currentWait); // Log de tentativa
        if (WiFi.status() != WL_CONNECTED) { // Evita begin() redundante quando já conectado
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Inicia/repete conexão com credenciais
        }
    }

    // Atualiza a janela de espera usando backoff exponencial com jitter e limites
    void backoffGrow() { // Exponencial com jitter (±10%), limitado por _maxRetry
        unsigned long next = _currentWait * 2; // Dobra a janela atual
        if (next > _maxRetry) next = _maxRetry; // Aplica teto de espera
        long jitter = (long)(next * 0.1f); // Calcula 10% de jitter (aleatoriedade)
        long delta = (long)random(-jitter, jitter); // Offset aleatório no intervalo [−jitter, +jitter)
        long candidate = (long)next + delta; // Soma jitter à janela dobrada
        if (candidate < (long)_baseRetry) candidate = (long)_baseRetry; // Garante piso no baseRetry
        _currentWait = (unsigned long)candidate; // Atualiza a janela resultante
    }
}; // Fim da classe NetManager
