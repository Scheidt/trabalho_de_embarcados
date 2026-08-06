# Sensor de distância com LEDs

Trabalho de sistemas embarcados de 2023. Um Arduino lê a distância de um sensor ultrassônico e acende um LED de acordo com a faixa em que a distância se encontra. A distância também aparece num display LCD.

Os limites entre as faixas são ajustados por um potenciômetro, então dá para mudar o alcance sem recompilar o código.

Este repositório é um arquivo. O código como ele foi entregue em 2023 está na branch `original` e não recebe commit novo. A `main` é o mesmo projeto consertado, em Arduino e com o mesmo hardware. O diff está em [compare/original...main](../../compare/original...main).

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
- vermelho piscando a 2 Hz: o sensor não recebeu eco

A distância usada na comparação é a mediana das últimas 5 leituras. Quando faltam 3 leituras válidas nessa janela, o sistema passa a "sem eco" em vez de inventar um número. Para trocar de faixa, a distância precisa ultrapassar o limite por pelo menos 2 cm, senão a faixa atual continua valendo.

O display mostra a distância na primeira linha e a faixa na segunda. O sensor mede a cada 60 ms, o display atualiza a cada 250 ms e a serial imprime a cada 1000 ms, cada um no seu ritmo e sem `delay()`.

## Para usar

Instale a biblioteca `LiquidCrystal I2C` na IDE do Arduino, abra `Projeto_definitivo_final_acabado_chega.ino` e envie para a placa. O monitor serial roda a 9600 baud e mostra os limites atuais, a leitura bruta, a leitura filtrada e a faixa.

Se o LCD ficar apagado, tente trocar o endereço `0x3F` por `0x27` na linha do `LiquidCrystal_I2C lcd(...)`.

A versão da branch `original` usava `lcd.begin()` sem argumentos, que existia na biblioteca instalada em 2023 e não existe na versão publicada hoje. Na `main` a chamada é `lcd.init()`.
