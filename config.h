#ifndef CONFIG_H
#define CONFIG_H

// struct para armazenar configurações do arquivo ini
struct IniConfig {
  unsigned long time_stage_0;   //tempo do estagio 0 (enchimento)
  unsigned long time_stage_1;   //tempo do estagio 1 (anaerobico)
  unsigned long time_stage_2;   //tempo do estagio 2 (reação)
  unsigned long time_stage_3;   //tempo do estagio 3 (sedimentação)
  unsigned long time_stage_4;   //tempo do estagio 4 (esvaizamento/retirada)
  unsigned long time_stage_5;   //tempo do estagio 5 (finalizado)
  unsigned long time_vap1;      //tempo de acionamento da valvula 1 de entrada (VAP1)
  unsigned long time_vap2;      //tempo de acionamento da valvula 2 de saída (VAP2)
  char log_level[6];            //nível de log (DEBUG, INFO, ERROR)
  unsigned long time_sens_log;  //tempo de log dos sensores
};
#endif
