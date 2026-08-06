#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <math.h>


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

LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Ou a leitura tem distancia, ou ela nao tem. Nenhum valor de "cm" significa
// falha: quem usa a leitura precisa olhar para "valida" primeiro. Quando a
// leitura nao vale, cm fica NAN, que nao passa por nenhuma comparacao de faixa.
struct Leitura {
  bool valida;
  float cm;
};

Leitura medirDistancia();

void setup() {
  // put your setup code here, to run once:
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  Serial.begin(9600);
  Serial.print("Hello World!");
  lcd.init();
  lcd.backlight();

}
void loop() {
  // put your main code here, to run repeatedly:
  Leitura leitura = medirDistancia();

  lcd.init();
  lcd.backlight();
  if (leitura.valida) {
    lcd.print("Distancia: ");
    lcd.print(leitura.cm);
  } else {
    lcd.print("Sem eco");
  }

  float value = analogRead(potencPin);
  float volt = value * 5.0 / 1023.0;

  float incremento1 = 5 *volt;
  float incremento2 = 10 *volt;

  Serial.print("Verde < ");
  Serial.print(incremento1);
  Serial.print("|  ");
  Serial.print(incremento1);
  Serial.print(" <= amarelo < ");
  Serial.print(incremento2);
  Serial.print("|  ");
  Serial.print(incremento2);
  Serial.println(" >= vermelho");

  if (!leitura.valida) {
    // Sem eco nao e distancia zero. Nenhum LED acende dizendo que esta perto.
    Serial.println("Sem eco: o sensor nao esta enxergando nada.");
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVermelho, LOW);
  }else if(leitura.cm < incremento1) {
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVermelho, LOW);
  }else if ((incremento1 <= leitura.cm) && (leitura.cm < incremento2)) {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAmarelo, HIGH);
    digitalWrite(ledVermelho, LOW);
  }else if (incremento2 <= leitura.cm) {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVermelho, HIGH);
  }


  delay(50);
}

Leitura medirDistancia(){
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
