#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <math.h>
#include <stdlib.h>


const unsigned char trigPin = 9;
const unsigned char echoPin = 10;
const unsigned char potencPin = A0;
const unsigned int ledVermelho = 13;
const unsigned int ledAmarelo = 12;
const unsigned int ledVerde = 11;

// Alcance util do HC-SR04 e o tempo de ida e volta do som correspondente.
// Sem esse limite, pulseIn() espera o padrao de 1 segundo antes de desistir.
const float ALCANCE_MAXIMO_CM = 400.0;
const unsigned long TIMEOUT_ECO_US = (unsigned long)(2.0 * ALCANCE_MAXIMO_CM / 0.0343);

// Cada tarefa tem a sua propria frequencia. O sensor mede rapido porque o
// objeto se move, o display atualiza devagar porque olho humano nao le a 20 Hz,
// e a serial fala uma vez por segundo para nao virar cachoeira de texto.
const unsigned long INTERVALO_MEDIDA_MS = 60;
const unsigned long INTERVALO_DISPLAY_MS = 250;
const unsigned long INTERVALO_SERIAL_MS = 1000;
const unsigned long INTERVALO_PISCA_MS = 250;

// A medida do sensor oscila alguns centimetros entre leituras consecutivas.
// A mediana das ultimas AMOSTRAS descarta o pico isolado sem atrasar a
// resposta como faria uma media. Uma leitura perdida no meio de leituras boas
// tambem e ruido: so vira "sem eco" quando faltam validas demais na janela.
const unsigned char AMOSTRAS = 5;
const unsigned char MINIMO_VALIDAS = 3;

// Folga exigida para trocar de faixa. Sem ela, uma distancia parada em cima
// do limite alternava as cores dos LEDs sozinha.
const float MARGEM_CM = 2.0;

const unsigned char COLUNAS_LCD = 16;

LiquidCrystal_I2C lcd(0x3F, COLUNAS_LCD, 2);

// Ou a leitura tem distancia, ou ela nao tem. Nenhum valor de "cm" significa
// falha: quem usa a leitura precisa olhar para "valida" primeiro. Quando a
// leitura nao vale, cm fica NAN, que nao passa por nenhuma comparacao de faixa.
struct Leitura {
  bool valida;
  float cm;
};

// Sem eco e uma faixa como as outras, nao a ausencia de faixa.
enum Faixa {
  FAIXA_VERDE,
  FAIXA_AMARELO,
  FAIXA_VERMELHO,
  FAIXA_SEM_ECO
};

Leitura janela[AMOSTRAS];
unsigned char proximaAmostra = 0;

Leitura leituraBruta = { false, NAN };
Leitura leituraAtual = { false, NAN };
Faixa faixaAtual = FAIXA_SEM_ECO;
float limite1 = 0.0;
float limite2 = 0.0;

unsigned long ultimaMedida = 0;
unsigned long ultimoDisplay = 0;
unsigned long ultimaSerial = 0;

Leitura medirDistancia();
Leitura filtrar();
float mediana(float *valores, unsigned char n);
Faixa classificar(Leitura leitura, float limite1, float limite2, Faixa atual);
void atualizarLimites();
void escreverLinha(unsigned char linha, const char *texto);
const char *nomeDaFaixa(Faixa faixa);

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  Serial.begin(9600);
  Serial.print("Hello World!");

  for (unsigned char i = 0; i < AMOSTRAS; i++) {
    janela[i] = Leitura{ false, NAN };
  }

  // O display se inicializa uma vez. Fazer isso a cada volta do loop era o
  // que apagava a tela inteira vinte vezes por segundo.
  lcd.init();
  lcd.backlight();
}

void loop() {
  unsigned long agora = millis();

  // Medir. A unica espera que sobrou e a do proprio pulseIn(), limitada pelo
  // timeout: no pior caso, sem eco nenhum, sao 23 ms.
  if (agora - ultimaMedida >= INTERVALO_MEDIDA_MS) {
    ultimaMedida = agora;
    atualizarLimites();

    leituraBruta = medirDistancia();
    janela[proximaAmostra] = leituraBruta;
    proximaAmostra = (proximaAmostra + 1) % AMOSTRAS;

    leituraAtual = filtrar();
    faixaAtual = classificar(leituraAtual, limite1, limite2, faixaAtual);
  }

  // Acender. Roda toda volta porque e barato e porque o pisca precisa da hora.
  bool vermelhoAceso = (faixaAtual == FAIXA_VERMELHO);
  if (faixaAtual == FAIXA_SEM_ECO) {
    vermelhoAceso = ((agora / INTERVALO_PISCA_MS) % 2) == 0;
  }
  digitalWrite(ledVerde, faixaAtual == FAIXA_VERDE ? HIGH : LOW);
  digitalWrite(ledAmarelo, faixaAtual == FAIXA_AMARELO ? HIGH : LOW);
  digitalWrite(ledVermelho, vermelhoAceso ? HIGH : LOW);

  // Escrever no display, sem limpar a tela: cada linha e reescrita completa,
  // com espacos ate o fim, entao nao sobra resto da mensagem anterior.
  if (agora - ultimoDisplay >= INTERVALO_DISPLAY_MS) {
    ultimoDisplay = agora;
    char linha[COLUNAS_LCD + 1];
    if (leituraAtual.valida) {
      char numero[10];
      dtostrf(leituraAtual.cm, 0, 1, numero);
      snprintf(linha, sizeof(linha), "Dist: %s cm", numero);
    } else {
      snprintf(linha, sizeof(linha), "Sem eco");
    }
    escreverLinha(0, linha);
    snprintf(linha, sizeof(linha), "Faixa: %s", nomeDaFaixa(faixaAtual));
    escreverLinha(1, linha);
  }

  if (agora - ultimaSerial >= INTERVALO_SERIAL_MS) {
    ultimaSerial = agora;
    Serial.print("Verde < ");
    Serial.print(limite1);
    Serial.print(" <= amarelo < ");
    Serial.print(limite2);
    Serial.print(" <= vermelho  |  bruta: ");
    if (leituraBruta.valida) {
      Serial.print(leituraBruta.cm);
    } else {
      Serial.print("sem eco");
    }
    Serial.print("  |  filtrada: ");
    if (leituraAtual.valida) {
      Serial.print(leituraAtual.cm);
    } else {
      Serial.print("sem eco");
    }
    Serial.print("  |  faixa: ");
    Serial.println(nomeDaFaixa(faixaAtual));
  }
}

Leitura medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // pulseIn() devolve 0 quando desiste de esperar. O zero nao sai daqui:
  // vira uma leitura invalida, que e coisa diferente de 0 cm.
  unsigned long duracao = pulseIn(echoPin, HIGH, TIMEOUT_ECO_US);
  if (duracao == 0) {
    return Leitura{ false, NAN };
  }
  return Leitura{ true, (float)((duracao * 0.0343) / 2.0) };
}

Leitura filtrar() {
  float validas[AMOSTRAS];
  unsigned char n = 0;
  for (unsigned char i = 0; i < AMOSTRAS; i++) {
    if (janela[i].valida) {
      validas[n] = janela[i].cm;
      n++;
    }
  }
  if (n < MINIMO_VALIDAS) {
    return Leitura{ false, NAN };
  }
  return Leitura{ true, mediana(validas, n) };
}

float mediana(float *valores, unsigned char n) {
  for (unsigned char i = 1; i < n; i++) {
    float chave = valores[i];
    signed char j = i - 1;
    while (j >= 0 && valores[j] > chave) {
      valores[j + 1] = valores[j];
      j--;
    }
    valores[j + 1] = chave;
  }
  return valores[n / 2];
}

void atualizarLimites() {
  float volt = analogRead(potencPin) * 5.0 / 1023.0;
  limite1 = 5 * volt;
  limite2 = 10 * volt;
}

Faixa classificar(Leitura leitura, float limite1, float limite2, Faixa atual) {
  if (!leitura.valida) {
    return FAIXA_SEM_ECO;
  }
  // Voltando de "sem eco" nao existe faixa anterior para segurar, entao os
  // limites valem crus.
  if (atual == FAIXA_SEM_ECO) {
    if (leitura.cm < limite1) return FAIXA_VERDE;
    if (leitura.cm < limite2) return FAIXA_AMARELO;
    return FAIXA_VERMELHO;
  }
  // Nas demais, cada limite se desloca para o lado que dificulta a saida da
  // faixa em que o sistema ja esta.
  float corte1 = (atual == FAIXA_VERDE) ? limite1 + MARGEM_CM : limite1 - MARGEM_CM;
  float corte2 = (atual == FAIXA_VERMELHO) ? limite2 - MARGEM_CM : limite2 + MARGEM_CM;
  if (leitura.cm < corte1) {
    return FAIXA_VERDE;
  }
  if (leitura.cm < corte2) {
    return FAIXA_AMARELO;
  }
  return FAIXA_VERMELHO;
}

const char *nomeDaFaixa(Faixa faixa) {
  switch (faixa) {
    case FAIXA_VERDE: return "verde";
    case FAIXA_AMARELO: return "amarelo";
    case FAIXA_VERMELHO: return "vermelho";
    default: return "sem eco";
  }
}

void escreverLinha(unsigned char linha, const char *texto) {
  char preenchida[COLUNAS_LCD + 1];
  snprintf(preenchida, sizeof(preenchida), "%-16s", texto);
  lcd.setCursor(0, linha);
  lcd.print(preenchida);
}
