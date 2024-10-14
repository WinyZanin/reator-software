// arduino mega 2560

/*
falta:
- debug pelo SD Card (OK)
- definir pinagem dos sensores (parcialmente OK)
- criar rotinas de debug (falta definir algumas coisas)
- fazer função de testes de reles e sensores (sera para acessar no futuro front-end)
- criar função para pegar parametros de um arquivo de configuração no SD Card
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

//variáveis de tempo
#define TIME_STAGE_0 10000  //tempo do estagio 0 (enchimento)
#define TIME_STAGE_1 10000  //tempo do estagio 1 (anaerobico)
#define TIME_STAGE_2 10000  //tempo do estagio 2 (reação)
#define TIME_STAGE_3 10000  //tempo do estagio 3 (sedimentação)
#define TIME_STAGE_4 10000  //tempo do estagio 4 (esvaizamento/retirada)
#define TIME_STAGE_5 10000  //tempo do estagio 5

byte stage = 0;  //estágio de funcionamento do reator

//classe do cartão SD
#define SD_CS 10  // Pino CS do cartão SD
#include "SDCard.h"
#include <SPI.h>
#include <SD.h>
SDCard sdCard(SD_CS);  // Instancia o cartão SD

//classe de log
#include "Logger.h"
Logger logger(Logger::DEBUG, &sdCard, &stage);  // Instancia o Logger no nível DEBUG, INFO, ERROR, o cartão SD e a variável de estágio

//classe de modulos
#include "modules.h"
modules modules(&logger);  // Instancia os módulos, passando o logger

unsigned long time = 0;  //tempo de funcionamento do reator

void setup() {
  Serial.begin(9600);  //inicialização da comunicação serial

  //inicialização do cartão SD
  if (!sdCard.begin()) {
    logger.error("Falha ao inicializar o cartão SD!");
  } else {
    logger.info("Cartão SD inicializado com sucesso.");
    if (!sdCard.openFiles()) {
      logger.error("Erro ao abrir os arquivos.");
    }
  }

  logger.info("Reator iniciado");
}

void loop() {
  switch (stage) {
    case 0:  // fase de enchimento
      if (time == 0) {
        modules.openVAP1();    // abre a valvula de entrada
        modules.activePump();  // aciona a bomba
        time = millis();       //inicia a contagem do tempo
        logger.info("Iniciando o processo de enchimento do reator");
      } else {
        if (millis() - time >= TIME_STAGE_0) {
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
        logger.info("Iniciando o processo de anaerobico");
      } else {
        if (millis() - time >= TIME_STAGE_1) {
          logger.info("Processo de anaerobico concluido");
          stage = 2;
          time = 0;
        }
      }
      break;
    case 2:
      if (time == 0) {
        time = millis();  //inicia a contagem do tempo
        logger.info("Iniciando o processo de reação");
        modules.openVS();  // abre a valvula de entrada de ar comprimido
      } else {
        if (millis() - time >= TIME_STAGE_2) {
          modules.closeVS();  // fecha a valvula de entrada de ar comprimido
          logger.info("Processo de reação concluido");
          stage = 3;
          time = 0;
        }
      }
      break;
    case 3:
      if (time == 0) {
        time = millis();  //inicia a contagem do tempo
        logger.info("Iniciando o processo de sedimentação");
      } else {
        if (millis() - time >= TIME_STAGE_3) {
          logger.info("Processo de sedimentação concluido");
          stage = 4;
          time = 0;
        }
      }
      break;
    case 4:
      if (time == 0) {
        time = millis();  //inicia a contagem do tempo
        logger.info("Iniciando o processo de esvaziamento");
        modules.openVAP2();  // abre a valvula de saída
        time = millis();     //inicia a contagem do tempo
      } else {
        if (millis() - time >= TIME_STAGE_4) {
          modules.closeVAP2();  // fecha a valvula de saída
          logger.info("Processo de esvaziamento concluido");
          stage = 5;
          time = 0;
        }
      }
      break;
    case 5:  //finalização do processo
      logger.info("Processo finalizado");
      stage = 0;  //reinicia o processo
      break;
    default:
      break;
  }
  delay(1000);  //tempo de espera entre ciclos
}