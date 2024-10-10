#ifndef SDCARD_H
#define SDCARD_H

#include <SPI.h>
#include <SD.h>

class SDCard {
public:
  // Construtor, aceita o pino CS (Chip Select) como parâmetro
  SDCard(int chipSelectPin) {
    this->chipSelectPin = chipSelectPin;
    this->directoryCount = 0;  // Inicializa a contagem de diretórios
  }

  // Inicializa o cartão SD
  bool begin() {
    if (!SD.begin(chipSelectPin)) {
      logSD("Falha ao inicializar o cartão SD!");
      return false;
    }
    logSD("Cartão SD inicializado com sucesso.");

    // Determina o próximo diretório sequencial a ser criado
    directoryCount = findNextDirectoryNumber();
    createDirectory(directoryCount);

    return true;
  }

  // Abre o arquivo de log e o arquivo CSV no novo diretório
  bool openFiles() {
    String directoryName = "/log_" + String(directoryCount);                         // Nome do diretório de trabalho
    String logFilePath = directoryName + "/log-" + String(directoryCount) + ".log";  // Caminho do arquivo de log
    String csvFilePath = directoryName + "/sen-" + String(directoryCount) + ".csv";  // Caminho do arquivo CSV

    // Abre o arquivo de log (cria se não existir)
    logFile = SD.open(logFilePath.c_str(), FILE_WRITE);
    if (!logFile) {
      logSD("Erro ao abrir o arquivo de log.");
      return false;
    }
    logSD("Arquivo de log aberto: " + logFilePath);
    // Abre o arquivo CSV (cria se não existir)
    csvFile = SD.open(csvFilePath.c_str(), FILE_WRITE);
    if (!csvFile) {
      logSD("Erro ao abrir o arquivo CSV.");
      return false;
    }
    logSD("Arquivo CSV aberto: " + csvFilePath);
    // Escreve o cabeçalho no arquivo CSV
    csvFile.println("Tempo (ms), Valor do Sensor");
    return true;
  }

  // Escreve no arquivo de log
  void writeLog(const char* message) {
    if (logFile) {
      logFile.println(message);
      logFile.flush();  // Grava os dados imediatamente
    }
  }

  // Escreve no arquivo CSV (tempo e valor do sensor)
  void writeCSV(unsigned long timeMillis, float sensorValue) {
    if (csvFile) {
      csvFile.print(timeMillis);
      csvFile.print(",");
      csvFile.println(sensorValue);
      csvFile.flush();  // Grava os dados imediatamente
    }
  }

  // Fecha os arquivos
  void closeFiles() {
    if (logFile) {
      logFile.close();
      logSD("Arquivo de log fechado.");
    }
    if (csvFile) {
      csvFile.close();
      logSD("Arquivo CSV fechado.");
    }
  }

private:
  int chipSelectPin;
  File logFile;
  File csvFile;
  int directoryCount;

  // Busca o próximo número de diretório sequencial
  int findNextDirectoryNumber() {
    int count = 0;
    while (SD.exists("/log_" + String(count))) {
      count++;
    }
    return count;
  }

  // Cria um novo diretório com o número sequencial
  void createDirectory(int directoryNumber) {
    String directoryName = "/log_" + String(directoryNumber);
    if (SD.mkdir(directoryName.c_str())) {
      logSD("Diretório criado: " + directoryName);
    } else {
      logSD("Erro ao criar o diretório.");
    }
  }

  // Registra mensagens no Monitor Serial
  void logSD(String message) {
    //String logMessage = "[SDcard] " + String(millis()) + " ms - " + message;
    Serial.println("[SDcard] " + String(millis()) + " ms - " + message);
  }
};

#endif
