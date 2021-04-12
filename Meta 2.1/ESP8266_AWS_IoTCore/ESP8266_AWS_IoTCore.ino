#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "x"; //colocar o nome da rede wifi
const char* password = "x"; //colocar a senha da rede wifi

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const char* AWS_endpoint = "x"; //colocar o MQTT broker ip (Endpoint da thing criada)

int compara(char s1[], char s2[]) {
  for (int i = 0; s1[i] != '\0' && s2[i] != '\0'; i++) {
    if (s1[i] < s2[i])
      return 1;
    else if (s1[i] > s2[i])
      return 1;
  }
  //strings iguais
  return 0;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived: ");
  Serial.println(topic);
  char* text = (char*)payload;
  char* flagLedON = "1";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  if ( compara(flagLedON, text) == 0 ) {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  Serial.println();

}
WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, callback, espClient); //Configuração padrão do MQTT
char msg[50];

void setup_wifi() {
  //Setup para conexão com o WiFi
  delay(10);
  espClient.setBufferSizes(512, 512);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(". ");
  }

  Serial.println();
  Serial.println("Conectado no WiFi!");
  Serial.print("Endereço de IP: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  espClient.setX509Time(timeClient.getEpochTime());

}

void reconnect() {
  digitalWrite(LED_BUILTIN, HIGH);
  int attempt = 0;
  while (!client.connected()) {//até que não esteja conectado.

    Serial.print("Tentativa de conexão com o MQTT... #");
    Serial.println(attempt);

    if (client.connect("ESPthing")) {//tentativa de conexão
      Serial.println("Conectado!");
      digitalWrite(LED_BUILTIN, LOW);
      delay(2000);
      digitalWrite(LED_BUILTIN, HIGH);
      //client.publish("outTopic", "a vida é boa, sim. A vida é boa.");//Publica em um tópico assim que a conexão é bem sucedida.
      client.subscribe("inTopic"); //Inscreve em um tópico.
    }
    else {
      attempt++;

      Serial.println(" Tenta de novo em 5 segundos");
      char buf[256];
      espClient.getLastSSLError(buf, 256); //Captura o erro decorrente ta tentativa de conexão.
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {

  Serial.begin(115200); //Define a velocidade do serial.
  Serial.setDebugOutput(true);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  setup_wifi();
  delay(1000);
  if (!SPIFFS.begin()) {
    Serial.println("Falha no Setup!");
    return;
  }

  Serial.print("Heap: ");
  Serial.println(ESP.getFreeHeap());

  //Carrega arquivo de certificado de conexão.
  File cert = SPIFFS.open("/cert.der", "r");
  if (!cert) {
    Serial.println("Arquivo de certificado de conexão não teve êxito na abertura.");
  }
  else
    Serial.println("Arquivo de certificado de conexão foi aberto.");

  delay(1000);

  if (espClient.loadCertificate(cert))
    Serial.println("Certificado de conexão foi carregado.");
  else
    Serial.println("Certificado de conexão não foi carregado.");

  //Carrega arquivo de chave privada.
  File private_key = SPIFFS.open("/private.der", "r");
  if (!private_key) {
    Serial.println("Arquivo de certificado de chave privada não teve êxito na abertura.");
  }
  else
    Serial.println("Arquivo de certificado de chave privada foi aberto.");

  delay(1000);

  if (espClient.loadPrivateKey(private_key))
    Serial.println("Certificado de chave privada foi carregado.");
  else
    Serial.println("Certificado de chave privada não foi carregado.");

  //Carrega arquivo de certificado do AWS Root.
  File ca = SPIFFS.open("/ca.der", "r");
  if (!ca) {
    Serial.println("Arquivo de certificado do AWS Root não teve êxito na abertura.");
  }
  else
    Serial.println("Arquivo de certificado do AWS Root foi aberto.");

  delay(1000);

  if (espClient.loadCACert(ca))
    Serial.println("Certificado do AWS root foi carregado.");
  else
    Serial.println("Certificado do AWS root não foi carregado.");

}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(3000);
}
