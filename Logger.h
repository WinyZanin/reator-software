#ifndef LOGGER_H
#define LOGGER_H

#include "SDCard.h"
#include "config.h"

class Logger {
public:
  enum Level {
    DEBUG,
    INFO,
    ERROR
  };

  // Construtor da classe
  Logger(IniConfig* iniConfig, SDCard* sdCard, byte* stage) {
    this->pointerIniConfig = iniConfig;  // Referência para a struct de configuração
    this->sdCard = sdCard;               // Referência para a classe SDCard
    this->PointerStage = stage;          // Referência para a variável de estágio
  }

  // Método para inicializar o logger
  void init() {
    // Converte a string de configuração para o enum Level
    // ou seja, pega o nível de log do arquivo de configuração
    logLevel = stringToEnum(pointerIniConfig->log_level);
  }

  // Método para registrar mensagens informativas
  void info(const String message) {
    if (logLevel <= INFO) {
      log("[INFO]", message);
    }
  }

  // Método para registrar mensagens de debug
  void debug(const String message) {
    if (logLevel <= DEBUG) {
      log("[DEBUG]", message);
    }
  }

  // Método para registrar mensagens de erro
  void error(const String message) {
    if (logLevel <= ERROR) {
      log("[ERROR]", message);
    }
  }

  // Método para mostrar as configurações do arquivo de configuração no log
  void showConfig() {
    log("[INFO]", "Configurações do arquivo de configuração:");
    log("[INFO]", "TIME_STAGE_0: " + String(pointerIniConfig->time_stage_0));
    log("[INFO]", "TIME_STAGE_1: " + String(pointerIniConfig->time_stage_1));
    log("[INFO]", "TIME_STAGE_2: " + String(pointerIniConfig->time_stage_2));
    log("[INFO]", "TIME_STAGE_3: " + String(pointerIniConfig->time_stage_3));
    log("[INFO]", "TIME_STAGE_4: " + String(pointerIniConfig->time_stage_4));
    log("[INFO]", "TIME_STAGE_5: " + String(pointerIniConfig->time_stage_5));
    log("[INFO]", "TIME_VAP1: " + String(pointerIniConfig->time_vap1));
    log("[INFO]", "TIME_VAP2: " + String(pointerIniConfig->time_vap2));
    log("[INFO]", "LOG_LEVEL: " + String(pointerIniConfig->log_level));
    // importante para mostrar o nível de log ativo
    if (logLevel == DEBUG) {
      log("[INFO]", "LOG_LEVEL ATIVO: DEBUG");
    } else if (logLevel == INFO) {
      log("[INFO]", "LOG_LEVEL ATIVO: INFO");
    } else if (logLevel == ERROR) {
      log("[INFO]", "LOG_LEVEL ATIVO: ERROR");
    }
    log("[INFO]", "TIME_SENS_LOG: " + String(pointerIniConfig->time_sens_log));
  }

private:
  Level logLevel;               // Nível de log que será utilizado
  IniConfig* pointerIniConfig;  // Ponteiro para a struct de configuração
  SDCard* sdCard;               // Ponteiro para o SDCard
  byte* PointerStage;           // Ponteiro para a variável de estágio

  // Método privado para registrar log no Monitor Serial e no cartão SD
  void log(const char* level, const String message) {
    // Monta a mensagem de log, incluindo o tempo em milissegundos e o estágio atual
    // Exemplo: [INFO] 4235 ms Stage:1 - Iniciando o processo de anaerobico
    String logMessage = String(level) + " " + String(millis()) + " ms Stage:" + *PointerStage + " - " + message;

    // Imprime no Monitor Serial
    Serial.println(logMessage);

    // Grava no arquivo de log no cartão SD
    sdCard->writeLog(logMessage.c_str());
  }

  // Função que converte uma string para o enum Level
  Level stringToEnum(char* str) {
    //log("[TESTE1]", str);
    trim(str);  // Remove espaços em branco no início e no final
    //log("[TESTE2]", str);
    toUpperCase(str);  // Converte para maiúsculas
    //log("[TESTE3]", str);

    if (strcmp(str, "DEBUG") == 0) {
      return DEBUG;
    } else if (strcmp(str, "INFO") == 0) {
      return INFO;
    } else if (strcmp(str, "ERROR") == 0) {
      return ERROR;
    } else {
      // Retorna INFO como padrão
      return INFO;
    }
  }

  // Função para remover espaços em branco no início e no final da string
  void trim(char* str) {
    // Remove espaços do final
    int end = strlen(str) - 1;
    while (end >= 0 && isspace(str[end])) {
      str[end] = '\0';
      end--;
    }

    // Remove espaços do início
    int start = 0;
    while (str[start] && isspace(str[start])) {
      start++;
    }

    // Move a string ajustada para o início
    if (start > 0) {
      memmove(str, str + start, strlen(str + start) + 1);
    }
  }

  // Função para converter um caractere para maiúsculas
  char toUpperCaseChar(char c) {
    if (c >= 'a' && c <= 'z') {
      return c - ('a' - 'A');
    }
    return c;
  }

  // Função para converter uma string para maiúsculas, para evitar erros
  void toUpperCase(char* str) {
    for (int i = 0; str[i]; i++) {
      str[i] = toUpperCaseChar(str[i]);
    }
  }
};

#endif