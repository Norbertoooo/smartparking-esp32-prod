#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Ultrasonic.h>
#include <ArduinoJson.h>

#define TRIGGER_PIN  12
#define ECHO_PIN     13
#define ledVermelho  33
#define ledVerde     25
#define ledAzul      26 

// certificado do heroku
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
"ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n" \
"MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \
"LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n" \
"RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n" \
"+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n" \
"PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n" \
"xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n" \
"Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n" \
"hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n" \
"EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n" \
"MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n" \
"FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n" \
"nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n" \
"eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n" \
"hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n" \
"Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n" \
"vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n" \
"+OkuE6N36B9K\n" \
"-----END CERTIFICATE-----\n";

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);
HTTPClient http;
WiFiMulti WiFiMulti;

const String nome = "vaga-01";
const String disponivel = "DISPONIVEL";
const String ocupada = "OCUPADA";
const int portaSerial = 115200;
bool estadoVaga;
int intervalo = 1000;
const String url = "https://reactive-smartparking-api.herokuapp.com/api/vagas";
const char* nomeDaRede = "EMG_NORTE 2.4G";
const char* senhaDaRede = "96427744"; 
const float distanciaLimite = 30;
const uint8_t fingerprint[20] = {0x08, 0x3b, 0x71, 0x72, 0x02, 0x43, 0x6e, 0xca, 0xed, 0x42, 0x86, 0x93, 0xba, 0x7e, 0xdf, 0x81, 0xc4, 0xbc, 0x62, 0x30};

void setup() {
  Serial.begin(portaSerial);
  pinMode(ledAzul, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(nomeDaRede, senhaDaRede);
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print("Conectando ao wifi: ");
    Serial.println(nomeDaRede);
  }
  Serial.println("Conexão com wifi estabelecida");
}

void loop() {
  float distanciaEmCM;
  long microsec = ultrasonic.timing();
  distanciaEmCM = ultrasonic.convert(microsec, Ultrasonic::CM);
  Serial.print("CM: ");
  Serial.println(distanciaEmCM);
  
  if(distanciaEmCM <= distanciaLimite){
    acendeLedVermelho();
    if(!estadoVaga){
      enviarEstadoDaVaga(ocupada);
      }
    estadoVaga = true;
  }
  
  if(distanciaEmCM > distanciaLimite){
    acendeLedVerde();
    if(estadoVaga){
      enviarEstadoDaVaga(disponivel);
      }
    estadoVaga = false;
  }
  delay(intervalo);
}

void enviarEstadoDaVaga(String estado){

  WiFiClientSecure *client = new WiFiClientSecure;
  if(client){
    client -> setCACert(rootCACertificate); 
    {
      StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
      JsonObject& JSONencoder = JSONbuffer.createObject();
  
      JSONencoder["nome"] = nome;
      JSONencoder["status"] = estado;
      char JSONmessageBuffer[300];
      JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
      
      http.begin(url);      //Specify request destination
      http.addHeader("Content-Type", "application/json");  //Specify content-type header
      int httpCode = http.POST(JSONmessageBuffer);   //Send the request
      String payload = http.getString();                  //Get the response payload
      
      Serial.print("Codigo de retorno da requisicao: ");
      Serial.println(httpCode);   //Print HTTP return code
      Serial.print("Corpo da resposta da requisicao: ");
      Serial.println(payload);    //Print request response payload
      Serial.print("Json enviado: ");
      Serial.println(JSONmessageBuffer);
      http.end();  //Close connection     
    }
    delete client;
  } 
}
  
void acendeLedVermelho(){
  digitalWrite(ledAzul, LOW);
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledVermelho, HIGH);
}

void acendeLedVerde(){
  digitalWrite(ledAzul, LOW);
  digitalWrite(ledVerde, HIGH);
  digitalWrite(ledVermelho, LOW);
}
