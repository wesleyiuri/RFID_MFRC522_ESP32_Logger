/*
    Arquivo: src/RfidReader.cpp
    Propósito: Implementação da classe RfidReader. Inicializa o barramento SPI,
    conversa com o MFRC522 e aplica deduplicação temporal por UID (cache + janela)
    via RfidDedupCache para evitar relatórios repetidos do mesmo cartão dentro do
    intervalo configurado, inclusive quando alterna entre diferentes tags.
*/

#include "RfidReader.h" // Declarações da classe RfidReader e tipos associados
#include <SPI.h> // Controle do barramento SPI (usado pelo MFRC522)
#include "Log.h" // Macros de log (INFO/DEBUG/ERROR)
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

// RfidReader::RfidReader(): cria o objeto MFRC522 com os pinos SDA(SS) e RST
RfidReader::RfidReader(uint8_t sda, uint8_t rst) // Início: construtor
    : _mfrc522(sda, rst), // Inicializa driver MFRC522 com pinos informados
        _lastUidCapture(0) { // Zera timestamp do último UID capturado
    _lastUid[0] = '\0'; // Limpa último UID (string vazia)
    _dedup.clear(); // Limpa cache de deduplicação por UID
} // fim: RfidReader::RfidReader()

// RfidReader::begin(): inicializa o barramento SPI e o MFRC522
void RfidReader::begin() { // Início: begin()
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_SDA); // Inicializa o SPI com pinos definidos
    _mfrc522.PCD_Init(); // Inicializa o leitor MFRC522
    LOG_INFO("MFRC522 inicializado"); // Loga inicialização bem-sucedida
} // fim: begin()

// RfidReader::read(): tenta ler um novo cartão; true se UID válido e não duplicado (por UID na janela)
bool RfidReader::read(char *outHex, size_t outLen, uint32_t &captureMs) { // Início: read()
    if (!_mfrc522.PICC_IsNewCardPresent()) return false; // Sem novo cartão presente
    if (!_mfrc522.PICC_ReadCardSerial()) return false; // Falha ao ler o serial do cartão

    char hex[32]; // Buffer local para UID em hexadecimal
    uidToHex(_mfrc522.uid, hex, sizeof(hex)); // Converte bytes do UID para string HEX
    uint32_t now = millis(); // Timestamp atual (ms desde o boot)

    // Se esta UID já foi enviada dentro da janela (cache por UID), ignora
    if (isDuplicate(hex, now)) { // Deduplicação temporal por UID usando cache
        LOG_DEBUG("RFID ignorado (duplicado na janela) UID=%s", hex); // Loga duplicata suprimida (visível em LOG_LEVEL>=3)
        _mfrc522.PICC_HaltA(); // Encerra comunicação com o cartão
        _mfrc522.PCD_StopCrypto1(); // Finaliza criptografia no leitor
        return false; // Ignora leitura duplicada
    }

    // Armazena como último UID para deduplicação futura
    strncpy(_lastUid, hex, sizeof(_lastUid)-1); // Copia UID para buffer interno
    _lastUid[sizeof(_lastUid)-1] = '\0'; // Garante terminação NUL
    _lastUidCapture = now; // Atualiza timestamp da última captura
    _dedup.remember(hex, now); // Atualiza/insere no cache de deduplicação por UID

    // Entrega ao chamador
    captureMs = now; // Timestamp de captura para o chamador
    strncpy(outHex, hex, outLen-1); // Copia UID em HEX para o buffer externo
    outHex[outLen-1] = '\0'; // Garante terminação do buffer externo

    // Finaliza comunicação com o cartão atual
    LOG_DEBUG("RFID aceito UID=%s t=%lu", outHex, (unsigned long)captureMs); // Loga leitura aceita (debug)
    _mfrc522.PICC_HaltA(); // Encerra comunicação com o cartão
    _mfrc522.PCD_StopCrypto1(); // Finaliza criptografia
    return true; // Sinaliza sucesso da leitura
} // fim: read()

// RfidReader::isDuplicate(): verifica se o UID é repetido dentro da janela (per-UID)
bool RfidReader::isDuplicate(const char *hex, uint32_t now) { // Início: isDuplicate()
    return _dedup.isDuplicate(hex, now); // Delega ao componente de cache
} // fim: isDuplicate()

// RfidReader::uidToHex(): bytes do UID -> string hexadecimal (maiúscula)
void RfidReader::uidToHex(const MFRC522::Uid &uid, char *out, size_t outLen) { // Início: uidToHex()
    size_t pos = 0; // Posição atual de escrita no buffer
    for (byte i = 0; i < uid.size && pos + 2 < outLen; i++) { // Itera bytes, garantindo espaço
        uint8_t b = uid.uidByte[i]; // Lê byte do UID
        char hi = nibbleHex((b >> 4) & 0x0F); // Converte nibble alto
        char lo = nibbleHex(b & 0x0F); // Converte nibble baixo
        out[pos++] = hi; // Escreve caractere alto
        out[pos++] = lo; // Escreve caractere baixo
    }
    out[pos] = '\0'; // Finaliza string com NUL
} // fim: uidToHex()

// RfidReader::nibbleHex(): retorna o char HEX correspondente ao nibble (0..15)
char RfidReader::nibbleHex(uint8_t n) { // Início: nibbleHex()
    return (n < 10) ? ('0' + n) : ('A' + (n - 10)); // '0'..'9' ou 'A'..'F'
} // fim: nibbleHex()
