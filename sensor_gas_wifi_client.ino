#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <string>

// Dados para autenticação a rede WIFI
char ssid[] = "GMCF";
char pass[] = "NET1920key";

// IP do servidor para o qual queremos estabelecer a comunicação
IPAddress server(192, 168, 1, 200);

// Para garantir que o IP esteja na mesma faixa do servidor
IPAddress ip(192, 168, 1, 250);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Define a porta para as requisições http
const int HTTP_PORT = 80;

// Argumentos utilizadas para envio de notificações ao servidor
const String ACTION_ALARM_ON = "on";
const String ACTION_ALARM_OFF = "off";

// Define os pinos para o RGB LED
const int LED_RED_PIN = D1;
const int LED_GREEN_PIN = D2;
const int LED_BLUE_PIN = D3;

// Define o pino para o buzzer
const int PIN_BUZZER = D8;

// Definicoes dos pinos ligados ao sensor
const int PIN_AO = A0;
const int PIN_D0 = D0;

// Nível mínimo para ser considerado normal
const int SENSOR_NORMAL_LEVEL = 250;

// Recebe o estado do sensor (false = Normal, true = Gas Detectado)
bool gasDetected = false;

// Recebe o valor analógico do sensor de gás, (Pino A0)
int valor_analogico = 0;

// Recebe o valor digital do sensor de gás, (Pino D0)
int valor_digital = 0;

void setup() {
  // inicializa o comunicador serial para monitoramento
  Serial.begin(9600);

  Serial.println("Cliente de para checagem de vazamento de gás");

  // Define os pinos de leitura do sensor como entrada
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_AO, INPUT);

  // Start de RGB PINs
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);

  // Define o aviso sonoro como saída
  pinMode(PIN_BUZZER, OUTPUT);

  // inicia os sinalizadores como o sistema de alarme para situação normal(Gás não detectado)
  runVisualFlags(ACTION_ALARM_OFF);

  // Definir explicitamente o ESP8266 para ser um cliente WiFi, evitando assim problemas de rede com os outros dispositivos na rede
  WiFi.mode(WIFI_STA);
  WiFi.begin ( ssid, pass );
  Serial.println ( "" );

  // Aguardando conexão
  setColor(0, 255, 255);
  Serial.print("Conectando a rede");
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  WiFi.config(ip, gateway, subnet);
  setColor(0, 255, 0);
  Serial.println("[OK]");

  Serial.println ( "" );
  Serial.print ( "Conectado a " );
  Serial.println ( ssid );
  Serial.print ( "IP: " );
  Serial.println ( WiFi.localIP() );
}

void loop()
{
  // Le os dados do pino digital D0 do sensor
  valor_digital = digitalRead(PIN_D0);

  // Le os dados do pino analogico A0 do sensor
  valor_analogico = analogRead(PIN_A0);

  // Verifica o nivel de gas/fumaca detectado
  bool prevRead = gasDetected;
  gasDetected = valor_analogico > SENSOR_NORMAL_LEVEL;

  // verifica se houve mudança de estado
  if (prevRead != gasDetected) {
    String action = gasDetected ? ACTION_ALARM_ON : ACTION_ALARM_OFF;
    sendAlert(action);
    runVisualFlags(action);
  }

  delay(100);
}

// Ativa/Desativa os sinalizadores do sistema (LED, Buzzer, etc)
void runVisualFlags(String action) {
  if (action == ACTION_ALARM_ON) {
    alarmSystemOn();
  }
  else if (action == ACTION_ALARM_OFF) {
    alarmSystemOff();
  }
}

// Liga o sistema de alarme
void alarmSystemOn() {
  digitalWrite(PIN_BUZZER, HIGH);
  setColor(255, 0, 0);
}

// Desliga o sistema de alarme
void alarmSystemOff() {
  digitalWrite(PIN_BUZZER, LOW);
  setColor(0, 255, 0);
}

// Permite definir as cores do RGB LED
void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(LED_RED_PIN, redValue);
  analogWrite(LED_GREEN_PIN, greenValue);
  analogWrite(LED_BLUE_PIN, blueValue);
}

// Envia um request para o servidor a fim de habilitar/desabilitar o sistema de alarme
void sendAlert(String action)
{
  WiFiClient client;
  if (!client.connect(server, HTTP_PORT)) {
    setColor(255, 157, 0);
    Serial.println("Não foi possível estabelecer uma conexão com o servidor");
    return;
  }

  char nivel[20];
  itoa(valor_analogico, nivel, 10);
  
  client.println("GET /?alarm=" + action + "&sensor_level=" + nivel + " HTTP/1.1");
  client.println("Accept: text/html, text/plain");
  client.println("Connection: close");

  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}
