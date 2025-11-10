/*
    Arquivo: include/Log.h
    Propósito: Fornece macros simples de logging com níveis configuráveis via
    build_flags (LOG_LEVEL). Essas macros imprimem mensagens no Serial de forma
    leve e evitam custo quando o nível configurado não habilita o log.
*/

#pragma once // Garante inclusão única deste cabeçalho durante a compilação
#include <Arduino.h> // Serial.printf_P, PSTR e tipos Arduino

#ifndef LOG_LEVEL // Se não definido pelo build, define padrão
#define LOG_LEVEL 2 // 0=OFF, 1=ERROR, 2=INFO, 3=DEBUG
#endif // fim: LOG_LEVEL default

// Macros condicionais por nível. Quando o nível é menor, a macro vira no-op.
#if LOG_LEVEL >= 1 // Habilita logs de erro
#define LOG_ERROR(fmt, ...) Serial.printf_P(PSTR("[E] " fmt "\n"), ##__VA_ARGS__) // Macro de erro (nível E)
#else // LOG_LEVEL < 1
#define LOG_ERROR(fmt, ...) // Macro vazia quando nível não habilita erro
#endif // fim: bloco LOG_ERROR

#if LOG_LEVEL >= 2 // Habilita logs informativos
#define LOG_INFO(fmt, ...) Serial.printf_P(PSTR("[I] " fmt "\n"), ##__VA_ARGS__) // Macro de info (nível I)
#else // LOG_LEVEL < 2
#define LOG_INFO(fmt, ...) // Macro vazia quando nível não habilita info
#endif // fim: bloco LOG_INFO

#if LOG_LEVEL >= 3 // Habilita logs de debug
#define LOG_DEBUG(fmt, ...) Serial.printf_P(PSTR("[D] " fmt "\n"), ##__VA_ARGS__) // Macro de debug (nível D)
#else // LOG_LEVEL < 3
#define LOG_DEBUG(fmt, ...) // Macro vazia quando nível não habilita debug
#endif // fim: bloco LOG_DEBUG
