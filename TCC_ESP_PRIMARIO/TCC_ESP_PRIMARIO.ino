#include <WiFi.h> // Carrega a biblioteca WiFi
#include <PubSubClient.h> // biblioteca para a conexão MQTT
#include <DHT.h> //biblioteca para o sensor DHT22
#include <WiFiManager.h> // biblioteca para gerencimaneto da WIFI https://github.com/tzapu/WiFiManager
#include <Preferences.h>
#include <stdlib.h>

// Insere os dados para conexão com o broker
const char* brokerUser = "";
const char* brokerPass = "";
const char* broker = "public.mqtthq.com";

WiFiClient espClient;
PubSubClient client(espClient);

// Atribui output para os pinos GPIO dos RELES
const int ledSala = 16;
const int ledCozinha = 4;
const int ledBanheiro = 19;
const int ledQuarto1 = 17;
const int ledQuarto2 = 18;

// ATRIBUICOES DHT

// DHT SALA
const int dhtSalaPin = 25;
DHT dhtSala(dhtSalaPin, DHT11); //objeto para o sensor DHT11
float tempSala; // variavel para temperatura
float tempSalaGlobal;
float humSala; // variavel para humidade

// DHT COZINHA
const int dhtCozinhaPin = 21;
DHT dhtCozinha(dhtCozinhaPin, DHT11);
float tempCozinha;
float humCozinha;

// DHT BANHEIRO
const int dhtBanheiroPin = 32;
DHT dhtBanheiro(dhtBanheiroPin, DHT11);
float tempBanheiro;
float humBanheiro;

// DHT QUARTO1
const int dhtQuarto1Pin = 33;
DHT dhtQuarto1(dhtQuarto1Pin, DHT11);
float tempQuarto1;
float humQuarto1;

// DHT QUARTO2
const int dhtQuarto2Pin = 26;
DHT dhtQuarto2(dhtQuarto2Pin, DHT11);
float tempQuarto2;
float humQuarto2;


// DECLARAÇÃO DE FUNÇÕES
bool connectMQTT();
void reconnect();
void callback(char *topic, byte * payload, unsigned int length);
void wifiManager();

void dhtSalaPub();
void dhtCozinhaPub();
void dhtBanheiroPub();
void dhtQuarto1Pub();
void dhtQuarto2Pub();

void setup() {

  Serial.begin(115200);

  // INICIALIZA OS PINOS LED COMO OUTPUT
  pinMode(ledSala, OUTPUT);
  pinMode(ledCozinha, OUTPUT);
  pinMode(ledBanheiro, OUTPUT);
  pinMode(ledQuarto1, OUTPUT);
  pinMode(ledQuarto2, OUTPUT);

  // SETA OUTPUTS PARA LOW (NECESSÁRIO POR CONTA DO USO DE RELÉS)
  digitalWrite(ledSala, HIGH);
  digitalWrite(ledCozinha, HIGH);
  digitalWrite(ledBanheiro, HIGH);
  digitalWrite(ledQuarto1, HIGH);
  digitalWrite(ledQuarto2, HIGH);

  // INICIALIZA OS DHTS
  dhtSala.begin();
  dhtCozinha.begin();
  dhtBanheiro.begin();
  dhtQuarto1.begin();
  dhtQuarto2.begin();

  wifiManager(); // FUNÇÃO PARA CONEXÃO WIFI

  client.setServer(broker, 1883); // INICIA CONEXÃO MQTT COM O BROKER
  client.setCallback(callback); // INICIA A FUNÇÃO CALLBACK MQTT (RECEBIMENTO DE MENSAGENS)
  
  Serial.println("Setup concluído com sucesso.");
}


void loop() {

  delay(250);

  dhtSalaPub();
  dhtCozinhaPub();
  dhtBanheiroPub();
  dhtQuarto1Pub();
  dhtQuarto2Pub();

  if (!client.connected()){
    reconnect();
  }
  client.loop();
}

// Configuracao WiFi
void wifiManager() {
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("ESP32","goldomessi123"); // Dados para a rede ESP32 criada em caso de não conexão

  if(!res) {
    Serial.println("Failed to connect");
    ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
  }
}

// FUNÇÃO DE RECONEXÃO MQTT
void reconnect(){
  while(!client.connected()){
    Serial.print("\nConectando-se a ");
    Serial.println(broker);
      if (client.connect("ESP32_PRIMARIO")) {
      Serial.println("Conectado");
      // Inscricao nos topicos
      client.subscribe("64c2c909ce81b/esp32/casa/sala/ledsala");
      client.subscribe("64c2c909ce81b/esp32/casa/cozinha/ledcozinha");
      client.subscribe("64c2c909ce81b/esp32/casa/banheiro/ledbanheiro");
      client.subscribe("64c2c909ce81b/esp32/casa/quarto1/ledquarto1");
      client.subscribe("64c2c909ce81b/esp32/casa/quarto2/ledquarto2");
    } else {
      Serial.print("erro, rc=");
      Serial.print(client.state());
      Serial.println("Tente novamente em 5 segundos");
      // aguarde 5 segundos ate a proxima tentativa de reconexao
      delay(5000);
    }
  }
}

// FUNÇÃO DE CALLBACK PARA RECEBER MENSAGENS VIA MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];

  Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);

  // Parte do codigo para mudanca de estado das GPIOS atraves da leitura das strings recebidas nos topicos inscritos
  if ( strcmp(topic, "64c2c909ce81b/esp32/casa/sala/ledsala") == 0) {
      if (incommingMessage.equals("1")) digitalWrite(ledSala, LOW);
      else if (incommingMessage.equals("0")) digitalWrite(ledSala, HIGH);
  }

    if ( strcmp(topic, "64c2c909ce81b/esp32/casa/cozinha/ledcozinha") == 0) {
      if (incommingMessage.equals("1")) digitalWrite(ledCozinha, LOW);
      else if (incommingMessage.equals("0")) digitalWrite(ledCozinha, HIGH);
  }

    if ( strcmp(topic, "64c2c909ce81b/esp32/casa/banheiro/ledbanheiro") == 0) {
      if (incommingMessage.equals("1")) digitalWrite(ledBanheiro, LOW);
      else if (incommingMessage.equals("0")) digitalWrite(ledBanheiro, HIGH);
  }

    if ( strcmp(topic, "64c2c909ce81b/esp32/casa/quarto1/ledquarto1") == 0) {
      if (incommingMessage.equals("1")) digitalWrite(ledQuarto1, LOW);
      else if (incommingMessage.equals("0")) digitalWrite(ledQuarto1, HIGH);
  }

    if ( strcmp(topic, "64c2c909ce81b/esp32/casa/quarto2/ledquarto2") == 0) {
      if (incommingMessage.equals("1")) digitalWrite(ledQuarto2, LOW);
      else if (incommingMessage.equals("0")) digitalWrite(ledQuarto2, HIGH);
  }
}

// FUNÇÃO PARA PUBLICAÇÃO DAS VARIAVEIS DO DHT SALA
void dhtSalaPub() {

  tempSala = dhtSala.readTemperature(); // LÊ A TEMPERATURA
  tempSalaGlobal = tempSala;

  // CONVERTE O VALOR LIDO PARA STRING (CHAR[])
  char tempSalaString[8];
  dtostrf(tempSala, 1, 2, tempSalaString);
  Serial.print("Temperatura Sala: ");
  Serial.println(tempSalaString);

  client.publish("64c2c909ce81b/esp32/casa/sala/tempsala", tempSalaString); // PUBLICA O VALOR DA TEMPERATURA LIDO

  humSala = dhtSala.readHumidity(); // LÊ A UMIDADE

  // CONVERTE O VALOR LIDO PARA STRING (CHAR[])
  char humSalaString[8];
  dtostrf(humSala, 1, 2, humSalaString);
  Serial.print("Umidade Sala: ");
  Serial.println(humSalaString);

  client.publish("64c2c909ce81b/esp32/casa/sala/humsala", humSalaString); // PUBLICA O VALOR DA TEMPERATURA LIDO
}

// FUNÇÃO PARA PUBLICAÇÃO DAS VARIAVEIS DO DHT COZINHA
void dhtCozinhaPub() {

  tempCozinha = dhtCozinha.readTemperature(); // LÊ A TEMPERATURA

  // CONVERTE O VALOR LIDO PARA STRING (CHAR[])
  char tempCozinhaString[8];
  dtostrf(tempCozinha, 1, 2, tempCozinhaString);
  Serial.print("Temperatura Cozinha: ");
  Serial.println(tempCozinhaString);

  client.publish("64c2c909ce81b/esp32/casa/cozinha/tempcozinha", tempCozinhaString); // PUBLICA O VALOR DA TEMPERATURA LIDO

  humCozinha = dhtCozinha.readHumidity(); // LÊ A UMIDADE

  // CONVERTE O VALOR LIDO PARA STRING (CHAR[])
  char humCozinhaString[8];
  dtostrf(humCozinha, 1, 2, humCozinhaString);
  Serial.print("Umidade Cozinha: ");
  Serial.println(humCozinhaString);

  client.publish("64c2c909ce81b/esp32/casa/cozinha/humcozinha", humCozinhaString); // PUBLICA O VALOR DA TEMPERATURA LIDO
}

// FUNÇÃO PARA PUBLICAÇÃO DAS VARIAVEIS DO DHT BANHEIRO
void dhtBanheiroPub() {

  tempBanheiro = dhtBanheiro.readTemperature(); // LÊ A TEMPERATURA

  // CONVERTE O VALOR LIDO PARA STRING (CHAR[])
  char tempBanheiroString[8];
  dtostrf(tempBanheiro, 1, 2, tempBanheiroString);
  Serial.print("Temperatura Banheiro: ");
  Serial.println(tempBanheiroString);

  client.publish("64c2c909ce81b/esp32/casa/banheiro/tempbanheiro", tempBanheiroString); // PUBLICA O VALOR DA TEMPERATURA LIDO

  humBanheiro = dhtBanheiro.readHumidity(); // LÊ A UMIDADE

  // CONVERTE O VALOR LIDO PARA STRING (CHAR[])
  char humBanheiroString[8];
  dtostrf(humBanheiro, 1, 2, humBanheiroString);
  Serial.print("Umidade Banheiro: ");
  Serial.println(humBanheiroString);

  client.publish("64c2c909ce81b/esp32/casa/banheiro/humbanheiro", humBanheiroString); // PUBLICA O VALOR DA TEMPERATURA LIDO
}

// FUNÇÃO PARA PUBLICAÇÃO DAS VARIAVEIS DO DHT QUARTO1
void dhtQuarto1Pub() {

  tempQuarto1 = dhtQuarto1.readTemperature(); // LÊ A TEMPERATURA

  // CONVERTE O VALOR LIDO PARA STRING (CHAR[])
  char tempQuarto1String[8];
  dtostrf(tempQuarto1, 1, 2, tempQuarto1String);
  Serial.print("Temperatura Quarto1: ");
  Serial.println(tempQuarto1String);

  client.publish("64c2c909ce81b/esp32/casa/quarto1/tempquarto1", tempQuarto1String); // PUBLICA O VALOR DA TEMPERATURA LIDO

  humQuarto1 = dhtQuarto1.readHumidity(); // LÊ A UMIDADE

  // CONVERTE O VALOR LIDO PARA STRING (CHAR[])
  char humQuarto1String[8];
  dtostrf(humQuarto1, 1, 2, humQuarto1String);
  Serial.print("Umidade Quarto1: ");
  Serial.println(humQuarto1String);

  client.publish("64c2c909ce81b/esp32/casa/quarto1/humquarto1", humQuarto1String); // PUBLICA O VALOR DA TEMPERATURA LIDO
}

// FUNÇÃO PARA PUBLICAÇÃO DAS VARIAVEIS DO DHT QUARTO2
void dhtQuarto2Pub() {

  tempQuarto2 = dhtQuarto2.readTemperature(); // LÊ A TEMPERATURA

  // CONVERTE O VALOR LIDO PARA STRING (CHAR[])
  char tempQuarto2String[8];
  dtostrf(tempQuarto2, 1, 2, tempQuarto2String);
  Serial.print("Temperatura Quarto2: ");
  Serial.println(tempQuarto2String);

  client.publish("64c2c909ce81b/esp32/casa/quarto2/tempquarto2", tempQuarto2String); // PUBLICA O VALOR DA TEMPERATURA LIDO

  humQuarto2 = dhtQuarto2.readHumidity(); // LÊ A UMIDADE

  // CONVERTE O VALOR LIDO PARA STRING (CHAR[])
  char humQuarto2String[8];
  dtostrf(humQuarto2, 1, 2, humQuarto2String);
  Serial.print("Umidade Quarto2: ");
  Serial.println(humQuarto2String);

  client.publish("64c2c909ce81b/esp32/casa/quarto2/humquarto2", humQuarto2String); // PUBLICA O VALOR DA TEMPERATURA LIDO
}



