/*
    Arquivo: include/RfidDedupCache.h
    Propósito: Encapsula a lógica de deduplicação temporal por UID usando um
    cache fixo (tamanho definido por DEDUP_CACHE_SIZE) e uma janela de tempo
    (DEDUP_INTERVAL_MS). Facilita testes unitários sem depender do MFRC522.
*/

#pragma once // Garante inclusão única deste cabeçalho durante a compilação
#include <Arduino.h> // Tipos/utilidades do Arduino (uint32_t, etc.)
#include <cstring> // strcmp, strncpy

#ifndef DEDUP_INTERVAL_MS // Permite sobrescrever via build_flags
#define DEDUP_INTERVAL_MS 1000 // Janela de deduplicação padrão (ms)
#endif // fim: DEDUP_INTERVAL_MS default
#ifndef DEDUP_CACHE_SIZE // Permite sobrescrever via build_flags
#define DEDUP_CACHE_SIZE 16 // Entradas distintas acompanhadas no cache
#endif // fim: DEDUP_CACHE_SIZE default

// Componente de deduplicação temporal por UID (cache + janela)
class RfidDedupCache { // Início da definição da classe RfidDedupCache
public: // Seção pública: API exposta a outros módulos
    // Construtor: inicializa o estado do cache
    RfidDedupCache() { clear(); } // Constrói e limpa o cache imediatamente

    // clear(): zera todas as entradas do cache (estado inicial)
    void clear() { // Início: clear()
        for (int i = 0; i < DEDUP_CACHE_SIZE; ++i) { // Varre todos os slots do cache
            _entries[i].used = false; // Marca o slot como livre (não usado)
            _entries[i].uid[0] = '\0'; // Limpa o UID armazenado (string vazia)
            _entries[i].lastMs = 0; // Zera o timestamp da última leitura aceita
        } // fim: laço de limpeza
    } // fim: clear()

    // isDuplicate(): retorna true se UID foi visto dentro da janela de deduplicação
    bool isDuplicate(const char* uidHex, uint32_t now) const { // Início: isDuplicate()
        int idx = findIndex(uidHex); // Procura índice do UID no cache
        if (idx < 0) return false; // UID não está no cache: não é duplicado
        return (now - _entries[idx].lastMs) < DEDUP_INTERVAL_MS; // true se ainda dentro da janela
    } // fim: isDuplicate()

    // remember(): registra/atualiza o UID no cache com timestamp 'now'
    void remember(const char* uidHex, uint32_t now) { // Início: remember()
        int idx = findIndex(uidHex); // Procura UID já existente no cache
        if (idx >= 0) { _entries[idx].lastMs = now; return; } // Já existe: atualiza timestamp e retorna
        // Procura um slot livre para cadastrar novo UID
        for (int i = 0; i < DEDUP_CACHE_SIZE; ++i) { // Percorre slots do cache
            if (!_entries[i].used) { // Achou slot livre
                writeSlot(i, uidHex, now); // Grava o UID e o timestamp no slot
                return; // Sai após gravar em slot livre
            } // fim: condição de slot livre
        } // fim: laço de busca de slot livre
        // Se o cache estiver cheio: substituir o mais antigo (menor lastMs)
        int oldest = 0; // Índice do slot considerado mais antigo
        for (int i = 1; i < DEDUP_CACHE_SIZE; ++i) { // Varre para encontrar o menor lastMs
            if (_entries[i].lastMs < _entries[oldest].lastMs) oldest = i; // Atualiza índice do mais antigo
        } // fim: laço de seleção do mais antigo
        writeSlot(oldest, uidHex, now); // Sobrescreve o slot do mais antigo com o novo UID
    } // fim: remember()

    // contains(): utilitário para testes/diagnóstico (verifica existência no cache)
    bool contains(const char* uidHex) const { return findIndex(uidHex) >= 0; } // true se UID já foi registrado

private: // Seção privada: estrutura interna e helpers
    struct Entry { // Entrada do cache
        char uid[32]; // UID armazenado em HEX (string terminada em NUL)
        uint32_t lastMs; // Timestamp da última leitura aceita (ms)
        bool used; // Indica se o slot está ocupado
    }; // fim: struct Entry
    Entry _entries[DEDUP_CACHE_SIZE]; // Array fixo de entradas (cache associativo simples)

    // findIndex(): devolve o índice do UID no cache; -1 se não encontrado
    int findIndex(const char* uidHex) const { // Início: findIndex()
        for (int i = 0; i < DEDUP_CACHE_SIZE; ++i) { // Percorre todos os slots
            if (_entries[i].used && strcmp(_entries[i].uid, uidHex) == 0) return i; // Achou UID neste slot
        } // fim: laço de busca
        return -1; // Não encontrado
    } // fim: findIndex()

    // writeSlot(): grava UID/tempo em um slot específico
    void writeSlot(int i, const char* uidHex, uint32_t now) { // Início: writeSlot()
        strncpy(_entries[i].uid, uidHex, sizeof(_entries[i].uid) - 1); // Copia UID com limite e sem overflow
        _entries[i].uid[sizeof(_entries[i].uid) - 1] = '\0'; // Garante terminação NUL do UID
        _entries[i].lastMs = now; // Atualiza timestamp do slot
        _entries[i].used = true; // Marca slot como ocupado
    } // fim: writeSlot()
}; // Fim da classe RfidDedupCache
