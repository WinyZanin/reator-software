// arduino mega 2560

/*
falta:
- debug pelo SD Card (OK)
- definir pinagem dos sensores (parcialmente OK)
- criar rotinas de debug (falta definir algumas coisas)
- fazer função de testes de reles e sensores (sera para acessar no futuro front-end)
- criar função para pegar parametros de um arquivo de configuração no SD Card(OK)
- certificar que todas as valvulas estão fechadas no inicio do programa (OK)
- dividir arquivos de log por ciclos de funcionamento
*/

/* relação tempo(minutos) x estágio x atuadores(ligados/desligados)
*
* |-------------|-----------|-------|-------|-------|-------|---------------|
* | tempo(min)  | estágio   | VAP1  | PUMP  | VS    | VAP2  | ação          |
* |-------------|-----------|-------|-------|-------|-------|---------------|
* | t0          | 0         | L     | L     | D     | D     | enchimento    |
* | t20         | 1         | D     | D     | D     | D     | anaerobico    |
* | t30         | 2         | D     | D     | L     | D     | reação        |
* | t225        | 3         | D     | D     | D     | D     | sedimentação  |
* | t230        | 4         | D     | D     | D     | L     | esvaziamento  |
* | t240        | 5         | D     | D     | D     | D     | finalização   |
* |-------------|-----------|-------|-------|-------|-------|---------------|
*
*/

#include "config.h"  //inclui a biblioteca de configuração, onde contem a struct IniConfig

//inicializa a struct de configuração com valores padrão
IniConfig iniConfig = {
  10000,    //tempo do estagio 0 (enchimento)
  10000,    //tempo do estagio 1 (anaerobico)
  10000,    //tempo do estagio 2 (reação)
  10000,    //tempo do estagio 3 (sedimentação)
  10000,    //tempo do estagio 4 (esvaizamento/retirada)
  10000,    //tempo do estagio 5 (finalizado)
  15000,    //tempo de acionamento da valvula 1 de entrada (VAP1)
  15000,    //tempo de acionamento da valvula 2 de saída (VAP2)
  "DEBUG",  //nível de log (DEBUG, INFO, ERROR)
  10000     //tempo de log dos sensores
};

byte stage = 0;  //estágio de funcionamento do reator

//classe do cartão SD
#define SD_CS 5  // Pino CS do cartão SD
#include "SDCard.h"
#include <SPI.h>
#include <SD.h>
SDCard sdCard(SD_CS);  // Instancia o cartão SD, passando o pino CS

//classe de log
#include "Logger.h"
Logger logger(&iniConfig, &sdCard, &stage);  // Instancia o logger, passando a struct de configuração, o cartão SD e o estágio

//classe de modulos
#include "modules.h"
modules modules(&logger, &iniConfig);  // Instancia os módulos, passando o logger

unsigned long time = 0;       // tempo de funcionamento do reator
unsigned long time_sens = 0;  // tempo de log dos sensores

void logSensor() {
  const float temperature = modules.getTemperature();
  const int od = modules.getOD();
  const int ph = modules.getPH();
  const bool nivel = modules.getNivel();
  logger.debug("Temperatura:" + String(temperature) + "Cº OD:" + String(od) + " PH:" + String(ph) + " Nivel:" + String(nivel) + " Stage:" + String(stage));
  if (millis() - time_sens >= iniConfig.time_sens_log) {
    time_sens = millis();
    sdCard.writeCSV(millis(), temperature, od, ph, nivel);
  }
}

void setup() {
  Serial.begin(9600);  // inicialização da comunicação serial

  // inicialização do cartão SD
  if (!sdCard.begin()) {
    logger.error("Falha ao inicializar o cartão SD!");
  } else {
    logger.info("Cartão SD inicializado com sucesso.");
    sdCard.setupConfig(&iniConfig);  // Configura o arquivo de configuração
    if (!sdCard.openFiles()) {       // cria os arquivos de log e CSV
      logger.error("Erro ao abrir os arquivos.");
    }
  }
  logger.init();        // Inicializa o logger
  logger.showConfig();  // mostra as configurações no Log

  logSensor();  // primeiro log dos sensores
  // certificando que as valvulas estão fechadas
  logger.info("Certificando que as valvulas estão fechadas...");
  modules.closeVAP1();
  modules.closeVAP2();
  modules.closeVS();
  logger.info("Valvulas fechadas");
  logger.info("Reator iniciado");
}

void loop() {
  // log dos sensores
  logSensor();

  // controle do estágio de funcionamento do reator
  switch (stage) {
    case 0:  // fase de enchimento
      if (time == 0) {
        modules.openVAP1();    // abre a valvula de entrada
        modules.activePump();  // aciona a bomba
        time = millis();       // inicia a contagem do tempo
        logger.info("Iniciando o processo de enchimento do reator...");
      } else {
        // verifica se o tempo de enchimento foi atingido ou se o nível foi alcançado
        if (millis() - time >= iniConfig.time_stage_0 || modules.getNivel()) {
          modules.stopPump();   // desliga a bomba
          modules.closeVAP1();  // fecha a valvula de entrada
          logger.info("Processo de enchimento do reator concluido");
          stage = 1;
          time = 0;
        }
      }
      break;
    case 1:
      if (time == 0) {
        time = millis();
        logger.info("Iniciando o processo de anaerobico...");
      } else {
        if (millis() - time >= iniConfig.time_stage_1) {
          logger.info("Processo de anaerobico concluido");
          stage = 2;
          time = 0;
        }
      }
      break;
    case 2:
      if (time == 0) {
        time = millis();  //inicia a contagem do tempo
        logger.info("Iniciando o processo de reação...");
        modules.openVS();  // abre a valvula de entrada de ar comprimido
      } else {
        if (millis() - time >= iniConfig.time_stage_2) {
          modules.closeVS();  // fecha a valvula de entrada de ar comprimido
          logger.info("Processo de reação concluido");
          stage = 3;
          time = 0;
        }
      }
      break;
    case 3:
      if (time == 0) {
        time = millis();  // inicia a contagem do tempo
        logger.info("Iniciando o processo de sedimentação...");
      } else {
        if (millis() - time >= iniConfig.time_stage_3) {
          logger.info("Processo de sedimentação concluido");
          stage = 4;
          time = 0;
        }
      }
      break;
    case 4:
      if (time == 0) {
        time = millis();  // inicia a contagem do tempo
        logger.info("Iniciando o processo de esvaziamento...");
        modules.openVAP2();  // abre a valvula de saída
        time = millis();     // inicia a contagem do tempo
      } else {
        if (millis() - time >= iniConfig.time_stage_4) {
          modules.closeVAP2();  // fecha a valvula de saída
          logger.info("Processo de esvaziamento concluido");
          stage = 5;
          time = 0;
        }
      }
      break;
    case 5:  // finalização do processo
      logger.info("Processo finalizado");
      stage = 0;  // reinicia o processo
      break;
    default:
      break;
  }

  delay(1000);  // tempo de espera entre ciclos
}