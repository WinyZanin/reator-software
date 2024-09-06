// arduino mega 2560

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
#define VAP1 22      // Válvula de acionamento pneumático - ENTRADA de efluentes brutos
#define VAP1_SEL 23  // Seletor de acionamento pneumático - ENTRADA de efluentes brutos
#define VAP2 24      // Válvula de acionamento pneumático - SAIDA de efluentes tratados
#define VAP2_SEL 25  // Seletor de acionamento pneumático - SAIDA de efluentes tratados
#define PUMP 26      // Bomba de ENTRADA de efluentes brutos
#define VS 28        // valvula solenoide de entrada de ar comprimido

//configuração de pinagem dos Sensores
#define OD 2           //sensor de oxigênio dissolvido
#define PH 3           //sensor de pH
#define NIVEL 4        //sensor de nível do reator
#define TEMPERATURA 5  //sensor de temperatura do reator

//variáveis
#define TIME_STAGE_0 10000  //tempo do estagio 0 (enchimento)
#define TIME_STAGE_1 10000  //tempo do estagio 1 (anaerobico)
#define TIME_STAGE_2 10000  //tempo do estagio 2 (reação)
#define TIME_STAGE_3 10000  //tempo do estagio 3 (sedimentação)
#define TIME_STAGE_4 10000  //tempo do estagio 4 (esvaizamento/retirada)
#define TIME_STAGE_5 10000  //tempo do estagio 5
#define TIME_VAP1 10000     //tempo de acionamento da valvula 1 de entrada (VAP1)
#define TIME_VAP2 10000     //tempo de acionamento da valvula 2 de saída (VAP2)

int stage = 0;  //estágio de funcionamento do reator

unsigned long time = 0;  //tempo de funcionamento do reator

//funções
void logger(String msg) {  //função para log
  Serial.println("[" + String(millis()) + " ms -" + " Stage: " + String(stage) + "] " + msg);
}

void setup() {
  //configuração dos pinos de saída
  pinMode(VAP1, OUTPUT);
  pinMode(VAP1_SEL, OUTPUT);
  pinMode(VAP2, OUTPUT);
  pinMode(VAP2_SEL, OUTPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(VS, OUTPUT);

  //configuração dos pinos de entrada
  pinMode(OD, INPUT);
  pinMode(PH, INPUT);
  pinMode(NIVEL, INPUT);
  pinMode(TEMPERATURA, INPUT);

  //inicialização dos pinos de saída, estagio 0
  digitalWrite(VAP1, LOW);
  digitalWrite(VAP1_SEL, LOW);
  digitalWrite(VAP2, LOW);
  digitalWrite(VAP2_SEL, LOW);
  digitalWrite(PUMP, LOW);
  digitalWrite(VS, LOW);

  Serial.begin(9600);  //inicialização da comunicação serial
  logger("Reator iniciado");
}

void loop() {
  switch (stage) {
    case 0:  // fase de enchimento
      if (time == 0) {
        digitalWrite(VAP1_SEL, LOW);  // seleciona abertura da valvula
        digitalWrite(VAP1, HIGH);     // aciona a valvula
        logger("Abrindo a valvula de entrada");
        delay(TIME_VAP1);         // tempo de espera do acionamento da valvula
        digitalWrite(VAP1, LOW);  // desliga a valvula
        logger("valvuva de entrada aberta");
        digitalWrite(PUMP, HIGH);  // aciona a bomba
        logger("Acionando a bomba de entrada");
        time = millis();  //inicia a contagem do tempo
        logger("Iniciando o processo de enchimento do reator");
      } else {
        if (millis() - time >= TIME_STAGE_0) {
          digitalWrite(PUMP, LOW);  // desliga a bomba
          logger("Bomba de entrada desligada");
          digitalWrite(VAP1_SEL, HIGH);  // seleciona fechamento da valvula
          digitalWrite(VAP1, HIGH);      // aciona a valvula
          logger("Fechando a valvula de entrada");
          delay(TIME_VAP1);         // tempo de espera do acionamento da valvula
          digitalWrite(VAP1, LOW);  // desliga a valvula
          logger("valvuva de entrada fechada");
          logger("Processo de enchimento do reator concluido");
          stage = 1;
          time = 0;
        }
      }
      break;
    case 1:
      if (time == 0) {
        time = millis();
        logger("Iniciando o processo de anaerobico");
      } else {
        if (millis() - time >= TIME_STAGE_1) {
          logger("Processo de anaerobico concluido");
          stage = 2;
          time = 0;
        }
      }
      break;
    case 2:
      if (time == 0) {
        time = millis();  //inicia a contagem do tempo
        logger("Iniciando o processo de reação");
        digitalWrite(VS, HIGH);  // aciona a valvula de entrada de ar comprimido
        logger("Acionando a valvula de entrada de ar comprimido");
      } else {
        if (millis() - time >= TIME_STAGE_2) {
          digitalWrite(VS, LOW);  // desliga a valvula de entrada de ar comprimido
          logger("Processo de reação concluido");
          stage = 3;
          time = 0;
        }
      }
      break;
    case 3:
      if (time == 0) {
        time = millis();  //inicia a contagem do tempo
        logger("Iniciando o processo de sedimentação");
      } else {
        if (millis() - time >= TIME_STAGE_3) {
          logger("Processo de sedimentação concluido");
          stage = 4;
          time = 0;
        }
      }
      break;
    case 4:
      if (time == 0) {
        time = millis();  //inicia a contagem do tempo
        logger("Iniciando o processo de esvaziamento");
        digitalWrite(VAP2_SEL, LOW);  // seleciona abertura da valvula
        digitalWrite(VAP2, HIGH);     // aciona a valvula
        logger("Abrindo a valvula de saída");
        delay(TIME_VAP2);         // tempo de espera do acionamento da valvula
        digitalWrite(VAP2, LOW);  // desliga a valvula
        logger("valvuva de saída aberta");
        time = millis();  //inicia a contagem do tempo
      } else {
        if (millis() - time >= TIME_STAGE_4) {
          digitalWrite(VAP2_SEL, HIGH);  // seleciona fechamento da valvula
          digitalWrite(VAP2, HIGH);      // aciona a valvula
          logger("Fechando a valvula de saída");
          delay(TIME_VAP2);         // tempo de espera do acionamento da valvula
          digitalWrite(VAP2, LOW);  // desliga a valvula
          logger("valvuva de saída fechada");
          logger("Processo de esvaziamento concluido");
          stage = 5;
          time = 0;
        }
      }
      break;
    case 5:  //finalização do processo
      logger("Processo finalizado");
      stage = 0;
      break;
    default:
      break;
  }
  delay(1000);  //tempo de espera entre ciclos
}