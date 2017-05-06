# Sensor de gás com WiFi (Server e Client)

O objetivo deste projeto é a criação de um ou mais sensores de gás integrados por meio de uma rede WiFi para prover um monitoramento de ambientes distintos.
Entenda como ambiente o local onde o sensor é instalado, como por exemplo um cômodo de uma casa (cozinha) ou um apartamento de um determinado prédio.
A ideia é que o server seja a central onde o alerta será disparado permitindo a detecção imediata do vazamento de gás.

### Circuito elétrico
![Esquema](https://github.com/professorgilmagro/arduino-sensor-gas/blob/master/screenshots/sensor-gas-esquema.png)

#### Componentes necessários
Para o desenvolvimento do sistema, seja para o server ou para o client, as peças necessárias são as mesmas, mudando apenas o script (código) que será utilizado nos respectivos pontos de verificação.

- 1	×	Sensor de Gás MQ-2 Inflamável e Fumaça
- 1	×	LED RGB 4 Pinos Catodo 5mm
- 1	×	D1 mini V2.2.0 WEMOS-ESP8266 WIFI
- 1	×	Buzzer
- 1	×	Resistor 150 ohm
- 2	×	Resistor 100 ohms
- 1	×	Resistor 330 ohms

#### Acesso ao monitor de gás via browser
Caso sejam mantidas as configurações conforme expressas no arquivo *sensor_gas_wifi_server.ino*, o acesso pode ser realizado a partir da url: *http://gas.local*, conforme telas abaixo:

##### Visualização do painel

![Desktop Normal](https://github.com/professorgilmagro/arduino-sensor-gas/blob/master/screenshots/webmonitor-normal.jpg)

![Desktop Vazamento](https://github.com/professorgilmagro/arduino-sensor-gas/blob/master/screenshots/webmonitor-vazamento.jpg)

![Mobile](https://github.com/professorgilmagro/arduino-sensor-gas/blob/master/screenshots/webmonitor-mobile.jpg?raw=true)

#### Melhorias adicionais (futuras)
Melhorias podem ser implementadas neste projeto, tais como alerta via SMS, email ou por meio de serviços (API) de mensageiros como WhatsApp, Telegram e outros.
Opcionamente, como recurso de prover um monitoramento mais eficiente, com histórico de alertas, níveis de detecção,  administração de usuários por ponto instalado, será necessário o desenvolvimento de um servidor com banco de dados remoto.

Por ora, o sistema funciona internamente (rede local). O **sensor client** dispara os sinalizadores (led, aviso sonoro) no local onde está instalado e envia um alerta ao **sensor server** que funciona como uma central, que por sua vez, ativa os sinalizadores visuais e sonoros correspondentes, além de atualizar a página com as informações enviadas pelo client.

