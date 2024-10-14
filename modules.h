#ifndef MODULES_H
#define MODULES_H

// Classe dos modulo do reator, aqui onde vai ficar todo o controle dos dispositivos do reator

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

//biblioteca do sensor de temperatura
#include <DS18B20.h>   //necessita instalar a biblioteca <OneWire.h> para funcionar
#include <OneWire.h>   //biblioteca para comunicação 1-Wire
DS18B20 ds(PIN_TEMP);  //Cria uma instância do sensor DS18B20

#include "logger.h"  //inclui a biblioteca de log

class modules {
public:
  // construtor da classe
  modules(Logger* logger) {
    this->logger = logger;  // ponteiro para a classe Logger

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

    //inicialização dos pinos de saída, estagio 0 (HIGH = relé desligado)
    digitalWrite(RELAY_VAP1, HIGH);
    digitalWrite(RELAY_VAP1_SEL, HIGH);
    digitalWrite(RELAY_VAP2, HIGH);
    digitalWrite(RELAY_VAP2_SEL, HIGH);
    digitalWrite(RELAY_PUMP, HIGH);
    digitalWrite(RELAY_VS, HIGH);
  }

  // metodos publicos

  //função para abrir a valvula de entrada de efluentes brutos
  void openVAP1() {
    digitalWrite(RELAY_VAP1_SEL, LOW);  // seleciona abertura da valvula
    digitalWrite(RELAY_VAP1, LOW);      // aciona a valvula
    logInfo("Abrindo a valvula de entrada...");
    delay(TIME_VAP1);                // tempo de espera do acionamento da valvula
    digitalWrite(RELAY_VAP1, HIGH);  // desliga a valvula
    logInfo("valvuva de entrada aberta");
  }

  //função para fechar a valvula de entrada de efluentes brutos
  void closeVAP1() {
    digitalWrite(RELAY_VAP1_SEL, HIGH);  // seleciona fechamento da valvula
    digitalWrite(RELAY_VAP1, LOW);       // aciona a valvula
    logInfo("Fechando a valvula de entrada...");
    delay(TIME_VAP1);                // tempo de espera do acionamento da valvula
    digitalWrite(RELAY_VAP1, HIGH);  // desliga a valvula
    logInfo("valvuva de entrada fechada");
  }

  //função para acionar a bomba de entrada
  void activePump() {
    digitalWrite(RELAY_PUMP, LOW);  // aciona a bomba
    logInfo("Acionando a bomba de entrada");
  }

  //função para desligar a bomba de entrada
  void stopPump() {
    digitalWrite(RELAY_PUMP, HIGH);  // desliga a bomba
    logInfo("Bomba de entrada desligada");
  }

  //função para abrir a valvula de entrada de ar comprimido
  void openVS() {
    digitalWrite(RELAY_VS, LOW);  // aciona a valvula de entrada de ar comprimido
    logInfo("Valvula de entrada de ar comprimido aberta");
  }

  //função para fechar a valvula de entrada de ar comprimido
  void closeVS() {
    digitalWrite(RELAY_VS, HIGH);  // desliga a valvula de entrada de ar comprimido
    logInfo("Valvula de entrada de ar comprimido fechada");
  }

  //função para abrir a valvula de saida de efluentes tratados
  void openVAP2() {
    digitalWrite(RELAY_VAP2_SEL, LOW);  // seleciona abertura da valvula
    digitalWrite(RELAY_VAP2, LOW);      // aciona a valvula
    logInfo("Abrindo a valvula de saída...");
    delay(TIME_VAP2);                // tempo de espera do acionamento da valvula
    digitalWrite(RELAY_VAP2, HIGH);  // desliga a valvula
    logInfo("valvuva de saída aberta");
  }

  //função para fechar a valvula de saida de efluentes tratados
  void closeVAP2() {
    digitalWrite(RELAY_VAP2_SEL, HIGH);  // seleciona fechamento da valvula
    digitalWrite(RELAY_VAP2, LOW);       // aciona a valvula
    logInfo("Fechando a valvula de saída...");
    delay(TIME_VAP2);                // tempo de espera do acionamento da valvula
    digitalWrite(RELAY_VAP2, HIGH);  // desliga a valvula
    logInfo("valvuva de saída fechada");
  }

  //função para obter o valor do sensor de temperatura
  float getTemperature() {
    if (ds.selectNext()) {
      // Captura a temperatura em graus Celsius
      return ds.getTempC();
    } else {
      logError("Erro ao ler a temperatura");
    }
    return 0;
  }

private:
  // variaveis privadas
  Logger* logger;  // ponteiro para a classe Logger

  // metodos privados
  //função para registrar mensagens informativas
  void logInfo(const char* message) {
    String completeMessage = "[MODULES] " + String(message);
    logger->info(completeMessage.c_str());
  }
  //função para registrar mensagens de erro
  void logError(const char* message) {
    String completeMessage = "[MODULES] " + String(message);
    logger->error(completeMessage.c_str());
  }
};

#endif