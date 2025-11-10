/*
    Arquivo: include/RfidReader.h
    Propósito: Declara a classe RfidReader que encapsula o acesso ao leitor
    MFRC522 via SPI, com deduplicação temporal por UID (janela + cache) para
    evitar reenvio da mesma tag dentro de um período, mesmo alternando com
    outras tags. Implementação em src/RfidReader.cpp.
*/

#pragma once // Garante inclusão única deste cabeçalho durante a compilação
#include <Arduino.h> // Tipos/utilidades Arduino (uint8_t, size_t, millis, etc.)
#include <MFRC522.h> // Biblioteca oficial do leitor RFID MFRC522
#include "RfidDedupCache.h" // Cache de deduplicação por UID (testável)

// Janela de deduplicação (ms): mesmo UID lido dentro da janela é descartado
#ifndef DEDUP_INTERVAL_MS // Permite sobreposição via build flags/ProjectConfig.h
#define DEDUP_INTERVAL_MS 1000 // Valor padrão: 1 segundo (janela de deduplicação)
#endif // DEDUP_INTERVAL_MS (proteção da macro)

// Tamanho do cache de deduplicação por UID (entradas recentes)
#ifndef DEDUP_CACHE_SIZE
#define DEDUP_CACHE_SIZE 16 // Número de UIDs distintos rastreados na janela
#endif // DEDUP_CACHE_SIZE

// Leitor RFID MFRC522 com deduplicação temporal por UID (cache + janela)
class RfidReader { // Início da definição da classe RfidReader
public: // Seção pública: API do leitor
    // Construtor: define pinos SDA/SS (chip select) e RST do MFRC522
    RfidReader(uint8_t sda, uint8_t rst); // Construtor do leitor RFID

    // Inicializa o barramento SPI e o chip MFRC522 (deve ser chamada no setup)
    void begin(); // Inicialização de hardware do RFID

    // Lê um UID, converte para HEX e aplica deduplicação temporal por UID (cache)
    // Retorna true se uma nova leitura válida foi obtida; outHex recebe o UID em HEX
    bool read(char *outHex, size_t outLen, uint32_t &captureMs); // Leitura não-bloqueante com dedup

private: // Seção privada: detalhes internos
    MFRC522 _mfrc522; // Instância do driver MFRC522
    char _lastUid[32]; // Mantido por compatibilidade (não é usado para dedup global)
    uint32_t _lastUidCapture; // Mantido por compatibilidade

    RfidDedupCache _dedup; // Componente de deduplicação por UID (cache + janela)

    // Verifica se o UID em HEX é duplicado dentro da janela DEDUP_INTERVAL_MS
    bool isDuplicate(const char *hex, uint32_t now); // Retorna true quando for duplicado (delegado ao cache)

    // Converte a estrutura MFRC522::Uid (bytes) para string HEX segura em out
    void uidToHex(const MFRC522::Uid &uid, char *out, size_t outLen); // Conversão UID->HEX

    // Converte um nibble (0..15) para seu caractere hexadecimal ('0'..'F')
    char nibbleHex(uint8_t n); // Conversão nibble para char HEX
}; // Fim da classe RfidReader
