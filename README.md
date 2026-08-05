# Sensor de distância com LEDs

Trabalho de sistemas embarcados. Um Arduino lê a distância de um sensor ultrassônico e acende um LED de acordo com a faixa em que a distância se encontra. A distância também aparece num display LCD.

Os limites entre as faixas são ajustados por um potenciômetro, então dá para mudar o alcance sem recompilar o código.

## Ligações

| Componente | Pino |
|---|---|
| HC-SR04 trig | 9 |
| HC-SR04 echo | 10 |
| Potenciômetro | A0 |
| LED vermelho | 13 |
| LED amarelo | 12 |
| LED verde | 11 |
| LCD I2C (endereço 0x3F) | SDA / SCL |

## Como funciona

O potenciômetro gera uma tensão de 0 a 5 V. A partir dela saem dois limites:

- limite 1 = 5 × tensão
- limite 2 = 10 × tensão

E os LEDs seguem:

- verde: distância abaixo do limite 1
- amarelo: distância entre o limite 1 e o limite 2
- vermelho: distância igual ou acima do limite 2

## Para usar

Instale a biblioteca `LiquidCrystal_I2C` na IDE do Arduino, abra `Projeto_definitivo_final_acabado_chega.ino` e envie para a placa. O monitor serial roda a 9600 baud e mostra os limites atuais.

Se o LCD ficar apagado, tente trocar o endereço `0x3F` por `0x27` na linha 12.
