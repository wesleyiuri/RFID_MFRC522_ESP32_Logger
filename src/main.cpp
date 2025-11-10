/*
    Arquivo: src/main.cpp
    Propósito: Ponto de entrada do firmware Arduino/ESP32. Este arquivo mantém a
    função setup() e loop() extremamente mínimas e delega toda a lógica de
    negócio (FSM, leitura RFID, rede, envio HTTP e buffer) para a classe
    AppController. Isso melhora a testabilidade e a organização do projeto.

    Autor: Wesley Santos — https://github.com/wesleyiuri/
*/

#include <Arduino.h> // Cabeçalho principal do framework Arduino (setup/loop, tipos básicos)
#include "AppController.h" // Declara a classe AppController que orquestra o sistema

// Instância global do controlador da aplicação (mantida em escopo estático do arquivo)
static AppController app; // 'static' restringe a visibilidade ao arquivo atual

// setup() é chamado uma única vez no boot/reset do microcontrolador
void setup() {
    app.begin(); // Inicializa todos os subsistemas via AppController (RFID, Wi‑Fi, etc.)
} // fim: setup()

// loop() é chamado de forma contínua pelo runtime do Arduino (semelhante a um scheduler cooperativo)
void loop() {
    app.loop(); // Executa uma iteração da FSM e serviços (não‑bloqueante)
} // fim: loop()