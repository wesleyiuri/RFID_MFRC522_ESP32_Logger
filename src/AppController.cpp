/*
    Arquivo: src/AppController.cpp
    Propósito: Implementa a classe AppController, que concentra a Máquina de
    Estados do firmware e a integração entre RFID, Wi‑Fi, envio HTTP e buffer.
    O código segue princípios de não‑bloqueio e tolerância a falhas.
*/

#include "AppController.h" // Declaração da classe AppController
#include "PersistentStore.h" // Interface de persistência do buffer (NVS)
// Configuração: tenta usar include/ProjectConfig.h (local, ignorado no Git), ou fallback para include/ProjectConfig.example.h
#if defined(__has_include)
#   if __has_include("ProjectConfig.h")
#       include "ProjectConfig.h" // Constantes de configuração definidas pelo usuário
#   else
#       include "ProjectConfig.example.h" // Fallback para CI e builds sem segredos
#  endif
#else
#  include "ProjectConfig.h" // Constantes de configuração definidas pelo usuário
#endif

#ifndef STATUS_LED_PIN // Se não definido via build_flags ou outro header
#define STATUS_LED_PIN -1 // Pino de LED opcional; -1 indica desativado
#endif // fim: STATUS_LED_PIN default

#ifndef QUEUE_DRAIN_INTERVAL_MS // Se não definido externamente
#define QUEUE_DRAIN_INTERVAL_MS 100 // Cadência mínima entre tentativas de envio (ms)
#endif // fim: QUEUE_DRAIN_INTERVAL_MS default

// Construtor: inicializa subcomponentes e estado interno padrão
AppController::AppController() // Construtor da classe AppController
    : _rfid(PIN_SDA, PIN_RST), // Inicializa o leitor MFRC522 com pinos do config.h
        _net(2000, 30000), // NetManager com backoff: base 2s, máximo 30s
        _http(HTTP_TIMEOUT_MS), // HttpSender com timeout configurável
        _state(State::INIT), // Começa em INIT para decidir o próximo estado
        _lastQueueAttempt(0), // Zera controle de cadência do envio
        _timeInitialized(false) {} // NTP ainda não inicializado

// begin(): chamada uma vez no boot para preparar todos os serviços
void AppController::begin() { // Inicializa os subsistemas e o estado inicial
    Serial.begin(115200); // Inicializa porta serial para logs e debug
    delay(50); // Pequeno atraso para estabilizar a serial
    Serial.println(); // Linha em branco para separar boots
    LOG_INFO("ESP32 RFID Logger iniciando..."); // Mensagem de início

    if (STATUS_LED_PIN >= 0) { // Se LED estiver habilitado
        pinMode(STATUS_LED_PIN, OUTPUT); // Configura pino como saída
        digitalWrite(STATUS_LED_PIN, LOW); // Indica estado inicial (desconectado)
    }

    _persist.begin(); // Abre namespace no NVS para persistência
    if (PERSIST_BUFFER) { // Se persistência ativada via flag
        _persist.load(_buffer); // Recupera snapshot anterior do buffer
        LOG_INFO("Buffer restaurado: %u entradas", (unsigned)_buffer.size());
    } // fim: restauração condicional do buffer persistido

    _rfid.begin(); // Inicializa o leitor MFRC522 (SPI + PCD_Init)

    // Registra callback chamado quando a rede conecta pela primeira vez
    _net.onConnect([this]() { // Registra lambda chamada quando conectar Wi‑Fi
        if (!_timeInitialized) { // Configura NTP apenas uma vez
            configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // NTP servidores
            _timeInitialized = true; // Marca NTP configurado
        } // fim: configuração única de NTP
        if (STATUS_LED_PIN >= 0) digitalWrite(STATUS_LED_PIN, HIGH); // LED ON indica conectado
    }); // fim: callback onConnect

    _net.begin(); // Inicia o Wi‑Fi (modo STA) e primeira tentativa de conexão
    _state = _net.isConnected() ? State::IDLE : State::CONNECTING; // Decide estado inicial
} // fim: begin()

// serviceRfid(): tenta ler uma UID (não‑bloqueante) e enfileirar
void AppController::serviceRfid() { // Lê RFID e enfileira, sem bloquear
    char uid[32]; // Buffer para UID em hexadecimal
    uint32_t capMs = 0; // Timestamp em millis() no momento da captura
    if (_rfid.read(uid, sizeof(uid), capMs)) { // Somente entra se uma nova UID foi aceita
        LOG_INFO("UID: %s", uid); // Loga a UID capturada
        _buffer.push(uid, capMs); // Enfileira UID + tempo de captura
        if (PERSIST_BUFFER) _persist.saveSnapshot(_buffer); // Persiste snapshot do buffer
    } // fim: bloco se houve nova UID
} // fim: serviceRfid()

// serviceQueueSend(): envia o item mais antigo do buffer, se conectado e respeitando cadência
void AppController::serviceQueueSend() { // Envia item mais antigo da fila, se possível
    if (!_net.isConnected()) return; // Sem Wi‑Fi não há envio
    if (_buffer.isEmpty()) return; // Sem dados para enviar
    unsigned long now = millis(); // Tempo atual do sistema (desde boot)
    if (now - _lastQueueAttempt < QUEUE_DRAIN_INTERVAL_MS) return; // Respeita intervalo
    _lastQueueAttempt = now; // Atualiza marca do último envio
    UidEntry e; // Estrutura para obter o item da frente
    if (!_buffer.peek(e)) return; // Leitura não destrutiva; aborta se falhar
    bool ok = _http.postUid(e); // Faz POST com a entrada atual
    if (ok) { // Se o envio foi bem-sucedido
        _buffer.pop(e); // Remove definitivamente da fila
        LOG_INFO("UID enviada: %s", e.uid); // Loga UID enviada
        if (PERSIST_BUFFER) _persist.saveSnapshot(_buffer); // Atualiza persistência
    } // fim: remoção e persistência após envio OK
} // fim: serviceQueueSend()

// loop(): uma iteração da FSM e serviços não‑bloqueantes
void AppController::loop() { // Executa uma iteração da FSM e serviços
    serviceRfid(); // Lê RFID com prioridade para não perder eventos
    _net.loop(); // Mantém a reconexão Wi‑Fi com backoff
    switch (_state) { // Máquina de estados de alto nível
        case State::INIT: // Estado transitório inicial
            _state = _net.isConnected() ? State::IDLE : State::CONNECTING; // Decide proximo
            break; // Sai do switch
        case State::CONNECTING: // Tentando conectar ao Wi‑Fi
            if (_net.isConnected()) { // Ao conectar
                LOG_INFO("Wi-Fi conectado."); // Sinaliza conexão estabelecida
                _state = State::SENDING_QUEUE; // Próximo: drenar fila acumulada
            } // fim: transição CONNECTING->SENDING_QUEUE
            break; // Permanece tentando caso contrário
        case State::SENDING_QUEUE: // Drenagem de fila quando há conectividade
            serviceQueueSend(); // Tenta enviar um item conforme cadência
            if (_buffer.isEmpty()) _state = State::IDLE; // Sem pendências -> IDLE
            if (!_net.isConnected()) _state = State::CONNECTING; // Queda de rede -> CONNECTING
            break; // Fim do caso SENDING_QUEUE
        case State::IDLE: // Conectado e sem pendências
            serviceQueueSend(); // Se chegar item novo, tentar enviar
            if (!_net.isConnected()) _state = State::CONNECTING; // Perdeu rede
            else if (!_buffer.isEmpty()) _state = State::SENDING_QUEUE; // Há itens -> drenar
            break; // Fim do caso IDLE
    } // fim: switch(_state)
} // fim: loop()
