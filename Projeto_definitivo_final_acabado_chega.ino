#include <LiquidCrystal_I2C.h>
#include <Wire.h>


const unsigned char trigPin = 9;
const unsigned char echoPin = 10;
const unsigned char potencPin = A0;
const unsigned int ledVermelho = 13;
const unsigned int ledAmarelo = 12;
const unsigned int ledVerde = 11;

LiquidCrystal_I2C lcd(0x3F, 16, 2);

float distance();

void setup() {
  // put your setup code here, to run once:
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  Serial.begin(9600);
  Serial.print("Hello World!");
  lcd.begin();
  lcd.backlight();

}
void loop() {
  // put your main code here, to run repeatedly:
  float distancia = distance();
  //Serial.print("Distance: ");
  //Serial.print(distancia);
  //Serial.println(" cm");

  
  lcd.begin();
  lcd.backlight();
  lcd.print("Distancia: ");
  lcd.print(distancia);

  float value = analogRead(potencPin);
  float volt = value * 5.0 / 1023.0;

  //Serial.print( "Value: " );
  //Serial.print( value );
  //Serial.print( "  Volt: " );
  //Serial.println( volt );

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

  if(distancia < incremento1) {
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVermelho, LOW);
  }else if ((incremento1 <= distancia) && (distancia < incremento2)) {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAmarelo, HIGH);
    digitalWrite(ledVermelho, LOW);
  }else if (incremento2 <= distancia) {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVermelho, HIGH);
  }


  delay(50);
}

float distance(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  float duration = pulseIn(echoPin, HIGH);
  return (duration*.0343)/2;
}