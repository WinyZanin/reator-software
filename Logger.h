#ifndef LOGGER_H
#define LOGGER_H

#include "SDCard.h"

class Logger {
public:
  enum Level {
    DEBUG,
    INFO,
    ERROR
  };

  // Construtor da classe
  Logger(Level logLevel, SDCard* sdCard, byte* stage) {
    this->logLevel = logLevel;   // Nível de log que será utilizado
    this->sdCard = sdCard;       // Referência para a classe SDCard
    this->PointerStage = stage;  // Referência para a variável de estágio
  }

  // Método para registrar mensagens informativas
  void info(const char* message) {
    if (logLevel <= INFO) {
      log("[INFO]", message);
    }
  }

  // Método para registrar mensagens de debug
  void debug(const char* message) {
    if (logLevel <= DEBUG) {
      log("[DEBUG]", message);
    }
  }

  // Método para registrar mensagens de erro
  void error(const char* message) {
    if (logLevel <= ERROR) {
      log("[ERROR]", message);
    }
  }

private:
  Level logLevel;      // Nível de log que será utilizado
  SDCard* sdCard;      // Ponteiro para o SDCard
  byte* PointerStage;  // Ponteiro para a variável de estágio

  // Método privado para registrar log no Monitor Serial e no cartão SD
  void log(const char* level, const char* message) {
    // Monta a mensagem de log, incluindo o tempo em milissegundos e o estágio atual
    // Exemplo: [INFO] 4235 ms Stage:1 - Iniciando o processo de anaerobico
    String logMessage = String(level) + " " + String(millis()) + " ms Stage:" + *PointerStage + " - " + message;

    // Imprime no Monitor Serial
    Serial.println(logMessage);

    // Grava no arquivo de log no cartão SD
    sdCard->writeLog(logMessage.c_str());
  }
};

#endif