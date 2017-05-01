#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <string.h>

/**
   Authentication
*/
char ssid[] = "GMCF";
char pass[] = "NET1920key";

// the IP address for the shield:
IPAddress ip(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

/**
   LED RGB PINs
*/
const int LED_RED_PIN = D1;
const int LED_GREEN_PIN = D2;
const int LED_BLUE_PIN = D3;
const int PIN_BUZZER = D8;

// Definicoes dos pinos ligados ao sensor
const int PIN_AO = A0;
const int PIN_D0 = D0;

int SENSOR_LEVEL = 250;

// Recebe o estado do sensor (false = Normal, true = Gas Detectado)
bool gasDetected = false;

// Recebe o valor analógico do sensor de gás, (Pino A0)
int valor_analogico = 0;

// Recebe o valor digital do sensor de gás, (Pino D0)
int valor_digital = 0;

// Flag para determinar se o vazamento é local ou externo (Local = Cozinha, Externo = Porão)
bool internal_location = true;

// Recebe o nivel de vazamento de um sensor externo (Porão)
String external_level = "Não informado";

/**
   Port default to webserver
*/
ESP8266WebServer server(80);

void setup()
{
  // Define os pinos de leitura do sensor como entrada
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_AO, INPUT);

  // Start de RGB PINs
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);

  // Define o aviso sonoro como saída
  pinMode(PIN_BUZZER, OUTPUT);

  // Start frequency
  Serial.begin(9600);
  delay(10);

  // Connect to WiFi network
  WiFi.begin ( ssid, pass );
  Serial.println ( "" );

  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  WiFi.config(ip, gateway, subnet);

  Serial.println ( "" );
  Serial.print ( "Conectado a " );
  Serial.println ( ssid );
  Serial.print ( "IP: " );
  Serial.println ( WiFi.localIP() );

  if ( MDNS.begin ( "esp8266" ) ) {
    Serial.println ( "MDNS iniciado" );
  }

  //Controller/Dispacher
  server.on ( "/", handleMain );
  server.onNotFound ( notFound );
  server.begin();
  Serial.println ( "Servidor HTTP Iniciado" );

  // inicia o alarme como desligado (Luz verde, buzzer desligado)
  alarmSystemOff();
}

void loop() {
  // Le os dados do pino digital D0 do sensor
  valor_digital = digitalRead(PIN_D0);

  // Le os dados do pino analogico A0 do sensor
  valor_analogico = analogRead(PIN_A0);

  // Verifica o nivel de gas/fumaca detectado
  bool prevRead = gasDetected;
  gasDetected = valor_analogico > SENSOR_LEVEL;

  // verifica se houve mudança de estado
  if (prevRead != gasDetected) {
    if (gasDetected) {
      internal_location = true;
      alarmSystemOn();
    } else {
      alarmSystemOff();
    }
  }

  // Ativa os clients
  server.handleClient();
  delay(500);
}

// Liga o sistema de alarme
void alarmSystemOn() {
  digitalWrite(PIN_BUZZER, HIGH);
  if (internal_location) {
    setColor(104, 3, 175);
  }
  else {
    setColor(255, 0, 0);
  }
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

/**
   Method main, manager channel and values
*/
void handleMain() {
  String param = server.argName( 0 ) ;
  String value = server.arg( 0 ) ;

  // Rota padrão, exibe a index (informações de estado do sensor)
  if ( server.args() == 0 ) {
    drawMainPage();
    return;
  }

  // Recupera o nível de leitura do sensor externo
  if (server.argName(1) == "sensor_level") {
    external_level = server.arg(1);
  }

  // Rota para ativar o alarme
  if ( param == "alarm" && value == "on" ) {
    internal_location = false;
    alarmSystemOn();
    acceptedResponse();
    return;
  }

  // Rota para desativar o alarme
  if (param == "alarm" && value == "off" ) {
    internal_location = true;
    alarmSystemOff();
    acceptedResponse();
    return;
  }

  // Rota para parâmetros desconhecidos
  if ( server.args() > 0 ) {
    badRequest();
  }
}

/**
   Main Page
*/
void drawMainPage() {
  char nivelInfo[20];
  itoa(valor_analogico, nivelInfo, 10);

  bool leaking = (gasDetected || internal_location == false);
  String stateInfo = leaking ? "Vazamento detectado" : "Normal";
  String stateIcon = leaking ? "down" : "up";
  String stateCss = leaking ? "danger" : "success";
  String local = internal_location ? "Cozinha" : "Porão";

  String message = "<!DOCTYPE html>\
<html>\
<head>\
<title>MQ2 - Sensor de Gas<\/title>\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\
<link href=\"http://getbootstrap.com/dist/css/bootstrap.min.css\" rel=\"stylesheet\" >\
<link href=\"http://getbootstrap.com/assets/css/docs.min.css\" rel=\"stylesheet\" >\
</head>\
<body>";
  message += "<div class=\"row\">";
  message += "<a href=\"#\" class=\"v4-tease\" style=\"background:#302a2a\"><img src=\"https://www.arduino.cc/en/favicon.png\"/> Arduino Project</a>";
  message += "<main class=\"bs-docs-masthead\" id=\"content\" tabindex=\"-1\">";
  message += "<div class=\"container\">";
  message += "<div class=\"container-fluid\">";
  message += "<h1><span class=\"glyphicon glyphicon-fire\" aria-hidden=\"true\"></span> Sensor de gás</h1>";
  message += "<h3>Exibe informações sobre os sensores de gás instalados na residência.</h3><p>&nbsp;</p>";
  message += "<p><a style=\"margin-right:10px;margin-bottom:10px\" class=\"btn btn-info btn-lg\" href=\"#\" role=\"button\"><span class=\"glyphicon glyphicon-home\" aria-hidden=\"true\"></span> Local: ";
  message += local;
  message += "</a>";
  message += "<a style=\"margin-right:10px;margin-bottom:10px\" class=\"btn btn-primary btn-lg\" href=\"#\" role=\"button\"><span class=\"glyphicon glyphicon-signal\" aria-hidden=\"true\"></span> Leitura atual: ";
  message += internal_location ? nivelInfo : external_level;
  message += "</a>";
  message += "<a style=\"margin-right:10px;margin-bottom:10px\"class=\"btn btn-" + stateCss + " btn-lg float-left\" href=\"#\" role=\"button\">";
  message += "<span class=\"glyphicon glyphicon glyphicon-thumbs-" + stateIcon + "\" aria-hidden=\"true\"></span> Situação: ";
  message += stateInfo;
  message += "</a>";

  // se os sinalizadores foram disparados, exibe o botão para silenciá-lo em situação especial
  if (leaking) {
    message += "<a style=\"margin-right:10px;margin-bottom:10px\"class=\"btn btn-warning btn-lg float-left\" href=\"http://192.168.1.200/?alarm=off\" role=\"button\"><span class=\"glyphicon glyphicon-volume-off\" aria-hidden=\"true\"></span> Silenciar sensor</a>";
  }

  message += "</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>";
  message += "</div></div><p>&nbsp;</p><p>&nbsp;</p></div></div>";
  message += "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script>";
  message += "<script src=\"http://getbootstrap.com/dist/js/bootstrap.min.js\"></script>";
  message += "<script>window.jQuery||document.write('<script src=\"http://getbootstrap.com/assets/js/vendor/jquery.min.js\"><\/script>')</script>";
  message += "</body>\
</html>";

  server.send( 200, "text/html", message );
}

/**
   Resposta para 'Page Not Found' - HTTP 404
*/
void notFound() {
  httpSimpleResponse(404, "File Not Found");
}

/**
   Resposta para 'Bad Request' - HTTP 400
*/
void badRequest() {
  httpSimpleResponse(400, "400 Bad Request");
}

/**
   Resposta para 'Accepted' - HTTP 202
*/
void acceptedResponse() {
  httpSimpleResponse(202, "202 Accepted");
}

// envia uma messagem http simples como resposta para uma determinada requisição
void httpSimpleResponse(int httpCode, String message) {
  message += "\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( httpCode, "text/plain", message );
}

