/*
    Arquivo: include/UidBuffer.h
    Propósito: Define um buffer circular (ring buffer) estático para armazenar
    UIDs lidas do RFID junto com o timestamp (millis) de captura, evitando
    alocações dinâmicas para maior robustez.
*/
#pragma once // Evita múltiplas inclusões do cabeçalho
#include <Arduino.h> // Tipos básicos e String (usada em toJson)
#include <cstring> // strncpy, memset

#ifndef UID_BUFFER_CAPACITY // Pode ser definido via build_flags em platformio.ini
#define UID_BUFFER_CAPACITY 64 // Capacidade padrão do ring buffer
#endif // UID_BUFFER_CAPACITY

// Estrutura fixa para armazenar UID + timestamp de captura.
struct UidEntry { // Estrutura do item armazenado no buffer
    char uid[32]; // UID em hexadecimal (até 28 bytes típico, margem extra)
    uint32_t capture_ms; // millis() no momento da leitura
}; // Fim da struct UidEntry

// Buffer circular estático sem alocação dinâmica.
// Política de overwrite: quando cheio, descarta o elemento mais antigo abrindo espaço.
class UidBuffer { // Início da definição da classe UidBuffer
public: // Seção pública: API do buffer
    // Construtor: zera índices e tamanho inicial do buffer
    UidBuffer() : _size(0), _head(0), _tail(0) {} // Inicializa membros com zero

    // Enfileira uma entrada (UID + timestamp); sobrescreve o mais antigo se cheio
    bool push(const char* uidHex, uint32_t captureMs) { // Insere elemento no head
        if (!uidHex || uidHex[0] == '\0') return false; // Rejeita UID nulo ou vazio
        if (_size == UID_BUFFER_CAPACITY) { // Detecta buffer cheio
            _tail = (_tail + 1) % UID_BUFFER_CAPACITY; // Avança tail para descartar o mais antigo
            _size--; // Ajusta tamanho após overwrite
        } // fim: tratamento de buffer cheio
        strncpy(_data[_head].uid, uidHex, sizeof(_data[_head].uid) - 1); // Copia UID com limite
        _data[_head].uid[sizeof(_data[_head].uid) - 1] = '\0'; // Garante terminação NUL
        _data[_head].capture_ms = captureMs; // Armazena timestamp de captura
        _head = (_head + 1) % UID_BUFFER_CAPACITY; // Avança head circularmente
        _size++; // Incrementa contagem de itens válidos
        return true; // Indica sucesso
    } // fim: push

    // Lê o elemento mais antigo sem remover
    bool peek(UidEntry &out) const { // Leitura não-destrutiva do tail
        if (_size == 0) return false; // Falha se vazio
        out = _data[_tail]; // Copia elemento mais antigo para out
        return true; // Sucesso
    } // fim: peek

    // Remove e devolve o elemento mais antigo (comportamento FIFO)
    bool pop(UidEntry &out) { // Deque do elemento no tail
        if (_size == 0) return false; // Falha se vazio
        out = _data[_tail]; // Copia elemento mais antigo
        _tail = (_tail + 1) % UID_BUFFER_CAPACITY; // Avança tail circularmente
        _size--; // Decrementa contagem
        return true; // Sucesso
    } // fim: pop

    // Verdadeiro se o buffer não contém elementos
    bool isEmpty() const { return _size == 0; } // Checa se tamanho é zero

    // Quantidade de elementos atualmente armazenados
    size_t size() const { return _size; } // Retorna tamanho atual

    // Capacidade máxima configurada em tempo de compilação
    size_t capacity() const { return UID_BUFFER_CAPACITY; } // Retorna capacidade

    // Acessa elemento pelo índice relativo ao mais antigo (0 = elemento em _tail)
    bool getAt(size_t indexFromOldest, UidEntry &out) const { // Acesso relativo ao tail
        if (indexFromOldest >= _size) return false; // Fora do intervalo
        size_t idx = (_tail + indexFromOldest) % UID_BUFFER_CAPACITY; // Converte índice lógico em físico
        out = _data[idx]; // Copia elemento para out
        return true; // Sucesso
    } // fim: getAt

    // Serializa a entrada para JSON parcial (sem metadados de device)
    String toJson(const UidEntry &e) const { // Monta JSON minimalista
        String s = "{"; // Abre objeto JSON
        s += "\"uid\":\""; s += e.uid; s += "\","; // Campo uid (com aspas ao redor do valor)
        s += "\"capture_timestamp_ms\":"; s += e.capture_ms; s += '}'; // Campo timestamp e fecha objeto
        return s; // Retorna JSON gerado
    } // fim: toJson

private: // Seção privada: armazenamento e índices
    UidEntry _data[UID_BUFFER_CAPACITY]; // Área estática de armazenamento
    size_t _size; // Número de elementos válidos
    size_t _head; // Próxima posição de escrita (incrementa circularmente)
    size_t _tail; // Próxima posição de leitura (elemento mais antigo)
}; // Fim da classe UidBuffer
