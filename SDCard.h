#ifndef SDCARD_H
#define SDCARD_H

#include <SPI.h>
#include <SD.h>

#include <IniFile.h>  //inclui a biblioteca de manipulação de arquivos .ini

class SDCard {
public:
  // Construtor, aceita o pino CS (Chip Select) como parâmetro
  SDCard(int chipSelectPin) {
    this->chipSelectPin = chipSelectPin;  // Inicializa o pino CS
    this->directoryCount = 0;             // Inicializa a contagem de diretórios
  }

  // Inicializa o cartão SD
  bool begin() {
    if (!SD.begin(chipSelectPin)) {
      logSD("Falha ao inicializar o cartão SD!");
      return false;
    }
    logSD("Cartão SD inicializado com sucesso.");

    // Determina o próximo diretório sequencial a ser criado
    // lê o diretorio mais alto e incrementa
    directoryCount = findNextDirectoryNumber();
    createDirectory(directoryCount);  // Cria o diretório de trabalho, que vai ser usado para armazenar os arquivos

    return true;
  }

  // Cria o arquivo de log e o arquivo CSV no novo diretório
  bool openFiles() {
    String directoryName = "/log_" + String(directoryCount);                         // Nome do diretório de trabalho
    String logFilePath = directoryName + "/log-" + String(directoryCount) + ".log";  // Caminho do arquivo de log
    String csvFilePath = directoryName + "/sen-" + String(directoryCount) + ".csv";  // Caminho do arquivo CSV

    // Abre o arquivo de log (cria se não existir)
    logFile = SD.open(logFilePath.c_str(), FILE_WRITE);
    if (!logFile) {
      logSD("[ERROR] ao abrir o arquivo de log.");
      return false;
    }
    logSD("Arquivo de log aberto: " + logFilePath);
    // Abre o arquivo CSV (cria se não existir)
    csvFile = SD.open(csvFilePath.c_str(), FILE_WRITE);
    if (!csvFile) {
      logSD("[ERROR] ao abrir o arquivo CSV.");
      return false;
    }
    logSD("Arquivo CSV aberto: " + csvFilePath);
    // Escreve o cabeçalho no arquivo CSV
    csvFile.println("TIME_MS,TEMPERATURE,OD,PH,NIVEL");
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
  void writeCSV(unsigned long timeMillis, float temperature, int od, int ph, bool nivel) {
    if (csvFile) {
      csvFile.print(timeMillis);
      csvFile.print(",");
      csvFile.print(temperature);
      csvFile.print(",");
      csvFile.print(od);
      csvFile.print(",");
      csvFile.print(ph);
      csvFile.print(",");
      csvFile.println(nivel);
      csvFile.flush();  // Grava os dados imediatamente
    }
  }

  // Fecha os arquivos, importante para evitar perda de dados
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

  // Função para configurar o arquivo de configuração
  // Recebe um ponteiro para a struct IniConfig,
  // que será preenchida com os valores lidos do arquivo de configuração,
  // ou com valores padrão caso o arquivo não exista.
  void setupConfig(IniConfig* config) {
    const size_t bufferLen = 80;
    char buffer[bufferLen];
    const char* configFilename = "/config.ini";  // Nome do arquivo de configuração

    IniFile ini(configFilename);  // Instancia o objeto IniFile

    // Abre o arquivo de configuração
    if (!ini.open()) {
      // caso o arquivo não exista, cria um novo com valores padrão
      logSD("Arquivo de configuração " + String(configFilename) + " não encontrado.");
      logSD("Criando arquivo de configuração com valores padrão.");
      //função para criar o arquivo de configuração com valores padrão
      File configFile = SD.open(configFilename, FILE_WRITE);  // Abre o arquivo para escrita
      // Checa se o arquivo foi aberto corretamente e escreve os valores padrão
      if (configFile) {
        configFile.println("[Time]");
        sprintf(buffer, "time_stage_0=%lu", iniConfig.time_stage_0);
        configFile.println(buffer);
        sprintf(buffer, "time_stage_1=%lu", iniConfig.time_stage_1);
        configFile.println(buffer);
        sprintf(buffer, "time_stage_2=%lu", iniConfig.time_stage_2);
        configFile.println(buffer);
        sprintf(buffer, "time_stage_3=%lu", iniConfig.time_stage_3);
        configFile.println(buffer);
        sprintf(buffer, "time_stage_4=%lu", iniConfig.time_stage_4);
        configFile.println(buffer);
        sprintf(buffer, "time_stage_5=%lu", iniConfig.time_stage_5);
        configFile.println(buffer);
        sprintf(buffer, "time_vap1=%lu", iniConfig.time_vap1);
        configFile.println(buffer);
        sprintf(buffer, "time_vap2=%lu", iniConfig.time_vap2);
        configFile.println(buffer);
        configFile.println("[Log]");
        sprintf(buffer, "log_level=%s", iniConfig.log_level);
        configFile.println(buffer);
        sprintf(buffer, "time_sens_log=%lu", iniConfig.time_sens_log);
        configFile.println(buffer);
        configFile.close();
        logSD("Arquivo de configuração criado com sucesso.");
      } else {
        logSD("[ERROR] ao criar o arquivo de configuração.");
      }
      configFile.close();
    } else {
      // Caso o arquivo exista, lê os valores
      logSD("Arquivo de configuração encontrado.");

      // Checa se o arquivo é válido. Isso pode ser usado para avisar se alguma linha
      // é maior que o buffer.
      if (!ini.validate(buffer, bufferLen)) {
        logSD("Arquivo de configuração " + String(configFilename) + " inválido: ");
        printErrorMessage(ini.getError());
        return;
      }

      // Lê os valores do arquivo de configuração
      if (ini.getValue("Time", "TIME_STAGE_0", buffer, bufferLen)) {
        //logSD("TIME_STAGE_0=" + String(buffer));
        iniConfig.time_stage_0 = atol(buffer);
      } else
        logSD("[ERROR] Não encontrado TIME_STAGE_0");

      if (ini.getValue("Time", "TIME_STAGE_1", buffer, bufferLen)) {
        //Serial.print("TIME_STAGE_1=" + String(buffer));
        iniConfig.time_stage_1 = atol(buffer);
      } else
        logSD("[ERROR] Não encontrado TIME_STAGE_1");

      if (ini.getValue("Time", "TIME_STAGE_2", buffer, bufferLen)) {
        //Serial.print("TIME_STAGE_2=" + String(buffer));
        iniConfig.time_stage_2 = atol(buffer);
      } else
        logSD("[ERROR] Não encontrado TIME_STAGE_2");

      if (ini.getValue("Time", "TIME_STAGE_3", buffer, bufferLen)) {
        //Serial.print("TIME_STAGE_3=" + String(buffer));
        iniConfig.time_stage_3 = atol(buffer);
      } else
        logSD("[ERROR] Não encontrado TIME_STAGE_3");

      if (ini.getValue("Time", "TIME_STAGE_4", buffer, bufferLen)) {
        //Serial.print("TIME_STAGE_4=" + String(buffer));
        iniConfig.time_stage_4 = atol(buffer);
      } else
        logSD("[ERROR] Não encontrado TIME_STAGE_4");

      if (ini.getValue("Time", "TIME_STAGE_5", buffer, bufferLen)) {
        //Serial.print("TIME_STAGE_5=" + String(buffer));
        iniConfig.time_stage_5 = atol(buffer);
      } else
        logSD("[ERROR] Não encontrado TIME_STAGE_5");

      if (ini.getValue("Time", "TIME_VAP1", buffer, bufferLen)) {
        //Serial.print("TIME_VAP1=" + String(buffer));
        iniConfig.time_vap1 = atol(buffer);
      } else
        logSD("[ERROR] Não encontrado TIME_VAP1");

      if (ini.getValue("Time", "TIME_VAP2", buffer, bufferLen)) {
        //Serial.print("TIME_VAP2=" + String(buffer));
        iniConfig.time_vap2 = atol(buffer);
      } else
        logSD("[ERROR] Não encontrado TIME_VAP2");

      if (ini.getValue("Log", "LOG_LEVEL", buffer, bufferLen)) {
        //Serial.print("LOG_LEVEL=" + String(buffer));
        strncpy(iniConfig.log_level, buffer, sizeof(iniConfig.log_level));
      } else
        logSD("[ERROR] Não encontrado LOG_LEVEL");

      if (ini.getValue("Log", "TIME_SENS_LOG", buffer, bufferLen)) {
        //Serial.print("TIME_SENS_LOG=" + String(buffer));
        iniConfig.time_sens_log = atol(buffer);
      } else
        logSD("[ERROR] Não encontrado TIME_SENS_LOG");

      printErrorMessage(ini.getError());  // Imprime mensagens de erro, se houver
    }
  }


private:
  int chipSelectPin;   // Pino CS do cartão SD
  File logFile;        // Arquivo de log
  File csvFile;        // Arquivo CSV
  int directoryCount;  // Contador de diretórios

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
      logSD("[ERROR] ao criar o diretório.");
    }
  }

  // Registra mensagens no Monitor Serial
  void logSD(String message) {
    //String logMessage = "[SDcard] " + String(millis()) + " ms - " + message;
    Serial.println("[SDcard] " + String(millis()) + " ms - " + message);
  }

  // Registra mensagens de erro do IniFile
  void printErrorMessage(uint8_t e, bool eol = true) {
    switch (e) {
      case IniFile::errorNoError:
        logSD("no error");
        break;
      case IniFile::errorFileNotFound:
        logSD("file not found");
        break;
      case IniFile::errorFileNotOpen:
        logSD("file not open");
        break;
      case IniFile::errorBufferTooSmall:
        logSD("buffer too small");
        break;
      case IniFile::errorSeekError:
        logSD("seek error");
        break;
      case IniFile::errorSectionNotFound:
        logSD("section not found");
        break;
      case IniFile::errorKeyNotFound:
        logSD("key not found");
        break;
      case IniFile::errorEndOfFile:
        logSD("end of file");
        break;
      case IniFile::errorUnknownError:
        logSD("unknown error");
        break;
      default:
        logSD("unknown error value");
        break;
    }
    //if (eol)
    //Serial.println();
  }
};

#endif
