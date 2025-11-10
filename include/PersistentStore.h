/*
    Arquivo: include/PersistentStore.h
    Propósito: Persistir um snapshot simples do UidBuffer em NVS (Preferences)
    do ESP32 quando PERSIST_BUFFER=1. Estratégia O(n) adequada a baixa escala
    para recuperar o estado do buffer após reinicializações.
*/

#pragma once // Garante inclusão única deste cabeçalho durante a compilação
#include <Arduino.h> // Tipos/utilidades Arduino (String, uint32_t, etc.)
#include "UidBuffer.h" // Declarações de UidBuffer e UidEntry
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

// Habilita a persistência apenas quando definido em ProjectConfig.h (evita custo quando desativado)
#if PERSIST_BUFFER // Compila bloco real de persistência quando PERSIST_BUFFER=1
#include <Preferences.h> // API de NVS/Preferences do ESP32

// Componente de persistência baseado em NVS/Preferences
class PersistentStore { // Início da definição da classe PersistentStore
public: // Seção pública: API exposta a outros módulos
    // Abre namespace "rfidbuf" na NVS para leitura/escrita
    void begin() { _prefs.begin("rfidbuf", false); } // Inicia Preferences no namespace rfidbuf (rw)

    // Salva snapshot completo do buffer (ordem do mais antigo ao mais novo)
    void saveSnapshot(const UidBuffer &buf) { // Serializa tamanho e pares (uid, timestamp)
        _prefs.putUInt("count", (uint32_t)buf.size()); // Persiste quantidade de itens
        UidEntry e; // Estrutura temporária para leitura de cada entrada
        for (size_t i = 0; i < buf.size(); ++i) { // Itera do item mais antigo ao mais novo
            buf.getAt(i, e); // Lê a i-ésima entrada do buffer para 'e'
            char keyUid[16]; snprintf(keyUid, sizeof(keyUid), "uid%u", (unsigned)i); // Monta chave UID
            char keyTs[16]; snprintf(keyTs, sizeof(keyTs), "ts%u", (unsigned)i); // Monta chave timestamp
            _prefs.putString(keyUid, e.uid); // Grava UID como String na NVS
            _prefs.putUInt(keyTs, e.capture_ms); // Grava timestamp (ms) na NVS
        } // fim do for
    } // fim: saveSnapshot

    // Carrega snapshot no buffer (respeita a capacidade e ignora entradas vazias)
    void load(UidBuffer &buf) { // Reconstrói o conteúdo do buffer a partir da NVS
        uint32_t count = _prefs.getUInt("count", 0); // Lê quantidade de itens
        if (count > UID_BUFFER_CAPACITY) count = UID_BUFFER_CAPACITY; // Limita à capacidade
        for (uint32_t i = 0; i < count; ++i) { // Itera sobre as posições salvas
            char keyUid[16]; snprintf(keyUid, sizeof(keyUid), "uid%u", i); // Chave UID
            char keyTs[16]; snprintf(keyTs, sizeof(keyTs), "ts%u", i); // Chave timestamp
            String uid = _prefs.getString(keyUid, ""); // Lê UID (ou vazio)
            uint32_t ts = _prefs.getUInt(keyTs, 0); // Lê timestamp (ms) (ou 0)
            if (uid.length() > 0) { // Ignora slots vazios
                buf.push(uid.c_str(), ts); // Reinsere no buffer na ordem correta
            }
        } // fim do for
    } // fim: load
private: // Seção privada: estado interno
    Preferences _prefs; // Handler da NVS/Preferences
}; // Fim da classe PersistentStore
#else // PERSIST_BUFFER desabilitado: fornecer stubs sem efeito
// Stubs no-op quando a persistência estiver desativada por build flag
class PersistentStore { // Início da classe stub (sem persistência real)
public: // API compatível, implementações vazias
    // Inicialização stub: não faz nada quando persistência está desabilitada
    void begin() {} // no-op
    // Salvamento stub: ignorado quando persistência está desabilitada
    void saveSnapshot(const UidBuffer &) {} // no-op
    // Carregamento stub: ignorado quando persistência está desabilitada
    void load(UidBuffer &) {} // no-op
}; // Fim da classe PersistentStore (stub)
#endif // Fim do controle condicional de PERSIST_BUFFER
