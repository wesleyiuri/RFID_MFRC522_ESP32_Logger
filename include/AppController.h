/*
    Arquivo: include/AppController.h
    Propósito: Declara a classe AppController, responsável por orquestrar o
    funcionamento do firmware como um todo. Ela mantém a Máquina de Estados
    Finitos (FSM), integra o leitor RFID, o gerenciador de rede, o buffer de UIDs
    e o cliente HTTP. Mantemos apenas as declarações no .h e a implementação no .cpp
    para acelerar recompilações e reduzir acoplamento.
*/

#pragma once // Garante inclusão única deste cabeçalho durante a compilação
#include <Arduino.h> // Tipos básicos/utilidades do Arduino (millis, tipos, etc.)
#include "UidBuffer.h" // Buffer circular fixo p/ armazenar UIDs lidas
#include "RfidReader.h" // Wrapper do MFRC522 com deduplicação temporal por UID (cache + janela via RfidDedupCache)
#include "NetManager.h" // Gerenciador de Wi‑Fi com backoff e callbacks
#include "HttpSender.h" // Cliente HTTP/HTTPS com política de retries
#include "Log.h" // Macros de logging por nível
#include "PersistentStore.h" // Persistência (NVS) opcional do buffer

// Controlador principal da aplicação (padrão façade/orquestrador)
class AppController { // Início da definição da classe que orquestra o firmware
public: // Seção pública: API exposta a outros módulos
    AppController(); // Construtor: inicializa membros e estado interno
    void begin(); // Inicialização: Serial, dispositivos, rede, NTP e persistência
    void loop(); // Iteração da FSM: leitura, reconexão e envio de fila
private: // Seção privada: detalhes internos não expostos
    // Estados de alto nível: Init (decisão), Connecting (Wi‑Fi), Sending (drena fila), Idle (standby conectado)
    enum class State { INIT, CONNECTING, SENDING_QUEUE, IDLE }; // Enum que modela a FSM

    UidBuffer _buffer; // Fila circular de UIDs capturadas (sem alocação dinâmica)
    RfidReader _rfid; // Leitor MFRC522 + deduplicação temporal por UID (impede reenvio < janela)
    NetManager _net; // Wi‑Fi com backoff exponencial e eventos
    HttpSender _http; // Cliente HTTP para enviar eventos ao endpoint
    State _state; // Estado atual da FSM
    unsigned long _lastQueueAttempt; // Controle de cadência de envio da fila (ms)
    bool _timeInitialized; // Indica se NTP/RTC já foi configurado (para timestamp ISO)
    PersistentStore _persist; // Persistência opcional do buffer (NVS)

    void serviceRfid(); // Lê RFID de forma não‑bloqueante e enfileira
    void serviceQueueSend(); // Tenta enviar o item mais antigo da fila (se conectado)
}; // Fim da classe AppController
