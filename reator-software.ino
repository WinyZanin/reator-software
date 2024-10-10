// arduino mega 2560

/*
falta:
- debug pelo SD Card
- definir pinagem dos sensores
- criar rotinas de debug
- fazer função de testes de reles e sensores
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

//configuração de pinagem dos Reles
#define RELAY_VAP1 38      // Válvula de acionamento pneumático - ENTRADA de efluentes brutos
#define RELAY_VAP1_SEL 39  // Seletor de acionamento pneumático - ENTRADA de efluentes brutos
#define RELAY_VAP2 40      // Válvula de acionamento pneumático - SAIDA de efluentes tratados
#define RELAY_VAP2_SEL 41  // Seletor de acionamento pneumático - SAIDA de efluentes tratados
#define RELAY_PUMP 42      // Bomba de ENTRADA de efluentes brutos
#define RELAY_VS 43        // valvula solenoide de entrada de ar comprimido

//configuração de pinagem dos Sensores
#define PIN_OD A1    //sensor de oxigênio dissolvido
#define PIN_PH A2    //sensor de pH
#define PIN_TEMP 5   //sensor de temperatura do reator
#define PIN_NIVEL 4  //sensor de nível do reator
//#define PIN_REDOX 6          //sensor de potencial redox
//#define PIN_CONDUTIVIDADE 7  //sensor de condutividade

//variáveis de tempo
#define TIME_STAGE_0 10000  //tempo do estagio 0 (enchimento)
#define TIME_STAGE_1 10000  //tempo do estagio 1 (anaerobico)
#define TIME_STAGE_2 10000  //tempo do estagio 2 (reação)
#define TIME_STAGE_3 10000  //tempo do estagio 3 (sedimentação)
#define TIME_STAGE_4 10000  //tempo do estagio 4 (esvaizamento/retirada)
#define TIME_STAGE_5 10000  //tempo do estagio 5
#define TIME_VAP1 15000     //tempo de acionamento da valvula 1 de entrada (VAP1)
#define TIME_VAP2 15000     //tempo de acionamento da valvula 2 de saída (VAP2)

byte stage = 0;  //estágio de funcionamento do reator

//classe do cartão SD
#define SD_CS 10  // Pino CS do cartão SD
#include "SDCard.h"
#include <SPI.h>
#include <SD.h>

SDCard sdCard(SD_CS);  // Instancia o cartão SD

//biblioteca do sensor de temperatura
#include <DS18B20.h>   //necessita instalar a biblioteca <OneWire.h> para funcionar
DS18B20 ds(PIN_TEMP);  //Cria uma instância do sensor DS18B20

//classe de log
#include "Logger.h"
Logger logger(Logger::DEBUG, &sdCard, &stage);  // Instancia o Logger no nível DEBUG, INFO, ERROR, o cartão SD e a variável de estágio


unsigned long time = 0;  //tempo de funcionamento do reator

//funções
//void logger(String msg) {  //função para log
//  Serial.println("[" + String(millis()) + " ms -" + " Stage: " + String(stage) + "] " + msg);
//}

float getTemperature() {  //função para obter a temperatura
  if (ds.selectNext()) {
    // Captura a temperatura em graus Celsius
    return ds.getTempC();
  } else {
    logger.error("Erro ao ler a temperatura");
  }
  return 0;
}

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

  //configuração dos pinos de saída
  pinMode(RELAY_VAP1, OUTPUT);
  pinMode(RELAY_VAP1_SEL, OUTPUT);
  pinMode(RELAY_VAP2, OUTPUT);
  pinMode(RELAY_VAP2_SEL, OUTPUT);
  pinMode(RELAY_PUMP, OUTPUT);
  pinMode(RELAY_VS, OUTPUT);

  //configuração dos pinos de entrada
  pinMode(PIN_OD, INPUT);
  pinMode(PIN_PH, INPUT);
  pinMode(PIN_NIVEL, INPUT);
  //pinMode(TEMPERATURA, INPUT);

  //inicialização dos pinos de saída, estagio 0 (HIGH= relé desligado)
  digitalWrite(RELAY_VAP1, HIGH);
  digitalWrite(RELAY_VAP1_SEL, HIGH);
  digitalWrite(RELAY_VAP2, HIGH);
  digitalWrite(RELAY_VAP2_SEL, HIGH);
  digitalWrite(RELAY_PUMP, HIGH);
  digitalWrite(RELAY_VS, HIGH);

  logger.info("Reator iniciado");
}

void loop() {
  switch (stage) {
    case 0:  // fase de enchimento
      if (time == 0) {
        digitalWrite(RELAY_VAP1_SEL, LOW);  // seleciona abertura da valvula
        digitalWrite(RELAY_VAP1, LOW);      // aciona a valvula
        logger.info("Abrindo a valvula de entrada");
        delay(TIME_VAP1);                // tempo de espera do acionamento da valvula
        digitalWrite(RELAY_VAP1, HIGH);  // desliga a valvula
        logger.info("valvuva de entrada aberta");
        digitalWrite(RELAY_PUMP, LOW);  // aciona a bomba
        logger.info("Acionando a bomba de entrada");
        time = millis();  //inicia a contagem do tempo
        logger.info("Iniciando o processo de enchimento do reator");
      } else {
        if (millis() - time >= TIME_STAGE_0) {
          digitalWrite(RELAY_PUMP, HIGH);  // desliga a bomba
          logger.info("Bomba de entrada desligada");
          digitalWrite(RELAY_VAP1_SEL, HIGH);  // seleciona fechamento da valvula
          digitalWrite(RELAY_VAP1, LOW);       // aciona a valvula
          logger.info("Fechando a valvula de entrada");
          delay(TIME_VAP1);                // tempo de espera do acionamento da valvula
          digitalWrite(RELAY_VAP1, HIGH);  // desliga a valvula
          logger.info("valvuva de entrada fechada");
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
        digitalWrite(RELAY_VS, LOW);  // aciona a valvula de entrada de ar comprimido
        logger.info("Acionando a valvula de entrada de ar comprimido");
      } else {
        if (millis() - time >= TIME_STAGE_2) {
          digitalWrite(RELAY_VS, HIGH);  // desliga a valvula de entrada de ar comprimido
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
        digitalWrite(RELAY_VAP2_SEL, LOW);  // seleciona abertura da valvula
        digitalWrite(RELAY_VAP2, LOW);      // aciona a valvula
        logger.info("Abrindo a valvula de saída");
        delay(TIME_VAP2);                // tempo de espera do acionamento da valvula
        digitalWrite(RELAY_VAP2, HIGH);  // desliga a valvula
        logger.info("valvuva de saída aberta");
        time = millis();  //inicia a contagem do tempo
      } else {
        if (millis() - time >= TIME_STAGE_4) {
          digitalWrite(RELAY_VAP2_SEL, HIGH);  // seleciona fechamento da valvula
          digitalWrite(RELAY_VAP2, LOW);       // aciona a valvula
          logger.info("Fechando a valvula de saída");
          delay(TIME_VAP2);                // tempo de espera do acionamento da valvula
          digitalWrite(RELAY_VAP2, HIGH);  // desliga a valvula
          logger.info("valvuva de saída fechada");
          logger.info("Processo de esvaziamento concluido");
          stage = 5;
          time = 0;
        }
      }
      break;
    case 5:  //finalização do processo
      logger.info("Processo finalizado");
      stage = 0;
      break;
    default:
      break;
  }
  delay(1000);  //tempo de espera entre ciclos
}