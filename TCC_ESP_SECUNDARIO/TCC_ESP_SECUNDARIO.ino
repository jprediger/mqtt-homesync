#include <WiFi.h> // Carrega a biblioteca WiFi
#include <PubSubClient.h> // biblioteca para a conexão MQTT
#include <DHT.h> //biblioteca para o sensor DHT22
#include <WiFiManager.h> // biblioteca para gerencimaneto da WIFI https://github.com/tzapu/WiFiManager
#include <Wire.h>
#include <Preferences.h>
#include <stdlib.h>
#include <ESP32Servo.h>

// Insere os dados para conexão com o broker
const char* brokerUser = "";
const char* brokerPass = "";
const char* broker = "public.mqtthq.com";

// Cria instancia da biblioteca preferences
Preferences preferences;

const int gasDigitalPin = 35;
const int gasAnalogPin = 34;
const int chamasDigitalPin = 33;
const int chamasAnalogPin = 32;
const int pinoBuzzer = 21;
const int fanPwm = 22;
const int dimmerPwm = 23;
const int dimmerRele = 25; 

WiFiClient espClient;
PubSubClient client(espClient);

// Atribuições servo motor
#define PIN_SG90 16
Servo sg90;
int persianaVal = 0;

// Variavel do modo de controle da fan
bool fanModeVarSala = false;
bool fanStatusSala = false;
int fanIntSala = 0;
int fanAlvoSala = 0;
float temperaturaSalaGlobal = 0;

const int fanPwmChan = 2;
const int fanPwmFreq = 1000;
const int fanPwmRes = 8;

// Variavel do modo de controle da fita de led
bool dimmerModeVarQuarto = false;
bool dimmerStatusQuarto = false;
int dimmerIntQuarto = 0;

int dimmerPwmChan = 1;
int dimmerPwmFreq = 1000;
int dimmerPwmRes = 8;

// Variaveis sensor de chamas
int chamasDigitalVal;
int chamasAnalogVal; 

// Variaveis sensor de gas
int gasDigitalVal;
int gasAnalogVal;

//define as funcoes
bool connectMQTT();
void reconnect();
void callback(char *topic, byte * payload, unsigned int length);

void wifiManager();
void clima();
void dimmer();
void chamas();
void gas();



void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  preferences.begin("prefs", false);
  fanStatusSala = preferences.getBool("fanStatusSala", false);
  preferences.end();

  // INICIALIZA OS PINOS UTILIZADOS
  pinMode(pinoBuzzer, OUTPUT);
  pinMode(gasAnalogPin, INPUT);
  pinMode(gasDigitalPin, INPUT);
  pinMode(fanPwm, OUTPUT);
  pinMode(dimmerPwm, OUTPUT);
  pinMode(dimmerRele, OUTPUT);
  pinMode(chamasDigitalPin, INPUT);

  // RELÉ (LÓGICA INVERSA)
  digitalWrite(dimmerRele, HIGH);

  // Configurações servo motor
  ESP32PWM::allocateTimer(3);
  sg90.setPeriodHertz(50); // PWM frequency for SG90
  sg90.attach(PIN_SG90, 500, 2400); // Minimum and maximum pulse width (in µs) to go from 0° to 180

  
  // CONFIGURAÇÃO PWM FAN
  ledcSetup(fanPwmChan, fanPwmFreq, fanPwmRes);
  ledcAttachPin(fanPwm, fanPwmChan);

  // CONFIGURAÇÃO PWM DIMMER
  ledcSetup(dimmerPwmChan, dimmerPwmFreq, dimmerPwmRes);
  ledcAttachPin(dimmerPwm, dimmerPwmChan);

  wifiManager(); // FUNÇÃO PARA CONEXÃO WIFI

  client.setServer(broker, 1883); // INICIA CONEXÃO MQTT COM O BROKER
  client.setCallback(callback); // INICIA A FUNÇÃO CALLBACK MQTT (RECEBIMENTO DE MENSAGENS)

  Serial.println("Setup concluído com sucesso.");
}

void loop() {

  delay(250);

  clima();
  dimmer();
  chamas();
  gas();
  
  if (!client.connected()){
    reconnect();
  }
  client.loop();
}

// ---------------------------- FUNÇÕES ----------------------------

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

// Função para se reconectar em caso de desconexão mqtt
void reconnect(){
  while(!client.connected()){
    Serial.print("\nConectando-se a ");
    Serial.println(broker);
      if (client.connect("ESP32_SECUNDARIO")) {
      Serial.println("Conectado");
      // Inscricao nos topicos
      client.subscribe("64c2c909ce81b/esp32/casa/sala/fanstatussala");
      client.subscribe("64c2c909ce81b/esp32/casa/sala/fanalvosala");
      client.subscribe("64c2c909ce81b/esp32/casa/sala/fanintsala");
      client.subscribe("64c2c909ce81b/esp32/casa/quarto1/dimmerstatusquarto");
      client.subscribe("64c2c909ce81b/esp32/casa/quarto1/dimmerintquarto");
      client.subscribe("64c2c909ce81b/esp32/casa/sala/tempsala");
      client.subscribe("64c2c909ce81b/esp32/casa/quarto2/persianaquarto2");

    } else {
      Serial.print("erro, rc=");
      Serial.print(client.state());
      Serial.println("Tente novamente em 5 segundos");
      // aguarde 5 segundos ate a proxima tentativa de reconexao
      delay(5000);
    }
  }
}

// Funcao de callback para receber as strings via MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];

  Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);

    if ( strcmp(topic, "64c2c909ce81b/esp32/casa/sala/fanstatussala") == 0) {
    if (incommingMessage.equals("0")) {
      fanStatusSala = false;
      preferences.begin("prefs", false);
      preferences.putBool("fanStatusSala", false);
      preferences.end();
    }
    else if (incommingMessage.equals("1")) {
      fanStatusSala = true;
      preferences.begin("prefs", false);
      preferences.putBool("fanStatusSala", true);
      preferences.end();
    }
    }

    if ( strcmp(topic, "64c2c909ce81b/esp32/casa/sala/fanintsala") == 0) {
    const char *fanIntSalaChar = incommingMessage.c_str();
    fanIntSala = atoi(fanIntSalaChar);
    }

    if ( strcmp(topic, "64c2c909ce81b/esp32/casa/sala/fanalvosala") == 0) {
    const char *fanAlvoSalaChar = incommingMessage.c_str();
    fanAlvoSala = atoi(fanAlvoSalaChar);
    }

    if ( strcmp(topic, "64c2c909ce81b/esp32/casa/sala/tempsala") == 0) {
    const char *tempSalaChar = incommingMessage.c_str();
    temperaturaSalaGlobal = atoi(tempSalaChar);
    }

    if ( strcmp(topic, "64c2c909ce81b/esp32/casa/quarto1/dimmerstatusquarto") == 0) {
    if (incommingMessage.equals("0")) {
      dimmerStatusQuarto = false;
      digitalWrite(dimmerRele, HIGH);
    }
    else if (incommingMessage.equals("1")) {
      dimmerStatusQuarto = true;
      digitalWrite(dimmerRele, LOW);
    }
    }

    if ( strcmp(topic, "64c2c909ce81b/esp32/casa/quarto1/dimmerintquarto") == 0) {
    const char *dimmerIntQuartoChar = incommingMessage.c_str();
    dimmerIntQuarto = atoi(dimmerIntQuartoChar);
    dimmerIntQuarto = 255 - dimmerIntQuarto;
    }

    if ( strcmp(topic, "64c2c909ce81b/esp32/casa/quarto2/persianaquarto2") == 0) {
    const char *persianaValChar = incommingMessage.c_str();
    persianaVal = atoi(persianaValChar);
    sg90.write(persianaVal);
    }
}

// Função para controle da ventoinha
void clima(){
  if (fanStatusSala == true && temperaturaSalaGlobal >= fanAlvoSala) {
      if(fanIntSala == 0) {
        ledcWrite(fanPwmChan, 255);
      }
      else {
      ledcWrite(fanPwmChan, fanIntSala);
      }
    }

  else {
    ledcWrite(fanPwmChan, 0);
  }
}

// Função para controle dimmer da fita de led
void dimmer(){
  if (dimmerStatusQuarto == true) {
      if(dimmerIntQuarto == 0) {
        ledcWrite(dimmerPwmChan, 0);
      }
      else {
      ledcWrite(dimmerPwmChan, dimmerIntQuarto);
      }
    }
}

// Função para controle do sensor de chamas
void chamas(){
  // Read the digital interface
  chamasDigitalVal = digitalRead(chamasDigitalPin);
  chamasAnalogVal = analogRead(chamasAnalogPin);

  // Variável para ser feito o publish 
  char chamasString[8];

  // Converte os valores digitais em string, mensagem no serial
  if (chamasDigitalVal == LOW){
    strcpy(chamasString, "1");
    Serial.println("Alerta! Chamas detectadas");
    tone(pinoBuzzer, 2300);
  }else{
    strcpy(chamasString, "0");
    Serial.println("Chamas não detectadas");
    noTone(pinoBuzzer);
  }

  client.publish("64c2c909ce81b/esp32/casa/cozinha/chamas", chamasString); // publica o valor booleano no tópico mqtt
}

// Função para controle do sensor de gás
void gas(){

  // Lê o pino analógico do sensor
  gasAnalogVal = analogRead(gasAnalogPin);
  // Lê o pino digital do sensor
  gasDigitalVal = digitalRead(gasDigitalPin);

  // Variável para ser feito o publish 
  char gasString[8];

  // Converte os valores digitais em string, mensagem no serial
  if(gasDigitalVal == LOW){
    strcpy(gasString, "1");
    Serial.println("Alerta! Gás tóxico detectado");
    tone(pinoBuzzer, 2300);
  }else{  
    strcpy(gasString, "0");
    Serial.println("Gás tóxico não detectado");
    noTone(pinoBuzzer);
  }

  client.publish("64c2c909ce81b/esp32/casa/cozinha/gas", gasString); // publica o valor booleano no tópico mqtt

}





