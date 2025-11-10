/*
    Arquivo: src/NetManager.cpp
    Propósito: Unidade de compilação associada a NetManager.h. A maior parte da
    lógica está inline no header por simplicidade/eficiência; este arquivo existe
    para permitir mover implementações mais pesadas para cá no futuro sem quebrar
    pontos de inclusão.
*/

#include "NetManager.h" // Declarações da classe NetManager
#include "Log.h" // Macros de log (consistência com o header)

// Nota: Hoje a implementação de NetManager está no header (leve/inline)
// Este arquivo existe como ponto de extensão: se a lógica crescer, mova métodos
// para cá para manter o header enxuto e reduzir recompilações
