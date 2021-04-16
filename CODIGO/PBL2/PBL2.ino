#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>


#define BUTTON D3
// nome da rede WIFI:
const char* ssid = "Fazendinha";
//senha da rede WIFI:
const char* password = "15253545";

//instancia um objeto do tipo WiFiUDP:
WiFiUDP ntpUDP;
//instancia um objeto do tipo NTPClient:
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// o end point da thing criada no AWS:
const char* awsEndPoint = "a2jm8vp5br1x7j-ats.iot.us-east-1.amazonaws.com";

// declaracoes de procedimentos:
void callback(char* topic, byte* payload, unsigned int length);
void setupWifi();
void reconnect();
void carregarArquivos();
int * capturarHora();
int calcularTempo(int * data_hora_inicio,int * data_hora_fim);
int calcularSegundos(int * data_hora);

//variaveis globais:
int * hora_inicio = 0;
int * hora_fim = 0;
int tempo_ativo = 0;
int tempo_desligar = 0;
int timer = 0;

//instancia um objeto do tipo WiFiClientSecure:
WiFiClientSecure espClient;
//Configuração padrão do MQTT:
PubSubClient client(awsEndPoint, 8883, callback, espClient);
char msg[50];

void setup() {
  Serial.begin(115200); // inicia o display serial
  pinMode(LED_BUILTIN, OUTPUT); //inicializa o pino do LED da placa como saída.
  pinMode(BUTTON, INPUT_PULLUP); // inicializa o pino do botao da placa como entrada.
  
  // Conexao com wifi:
  setupWifi();
  delay(1000);
  carregarArquivos();
}

void loop() {
  if(!client.connected())
    reconnect();
  client.loop();

  /*
  if(!digitalRead(BUTTON)){
    if(!digitalRead(LED_BUILTIN)){
      digitalWrite(LED_BUILTIN, HIGH);
      hora_fim = capturarHora();
      tempo_ativo = calcularTempo(hora_inicio, hora_fim);
      char string[50];
      Serial.println(tempo_ativo);
      //sprintf(string, "%d", tempo_ativo);
      Serial.println(string);
      //client.publish("TEMPO_ATIVO", string);
      
      hora_inicio = 0;
      hora_fim = 0;
      tempo_ativo = 0;
      timer = 0;
      tempo_desligar = 0;
    }
    else{
      digitalWrite(LED_BUILTIN, LOW);
      hora_inicio = capturarHora();
    }
  } */

  
  if(timer){
    int * aux_hora = capturarHora();
    if(calcularSegundos(aux_hora) == tempo_desligar){
      digitalWrite(LED_BUILTIN, HIGH);
      hora_fim = aux_hora;
      tempo_ativo = calcularTempo(hora_inicio, hora_fim);
      char string[50];
      Serial.println(tempo_ativo);
      sprintf(string, "%d", tempo_ativo);
      Serial.println(string);
      client.publish("TEMPO_ATIVO", string);
      
      tempo_ativo = 0;
      timer = 0;
      tempo_desligar = 0;
    }
  }
  
  delay(500);
}


//A função que é chamada sempre que algum tópico assinado é atualizado
void callback(char * topic, byte * payload, unsigned int length){
  // mostra no serial o que foi recebido
  Serial.print("Mensagem recebida: ");
  Serial.println(topic);
  
  //Captura a mensagem recebida no topico topic:
  char text[length];
  for(int i = 0; i<length; i++){
    text[i] = (char) payload[i];
  }
  //printa o conteudo da mensagem:
  Serial.println(text);

  if(!strcmp(topic, "ON_OFF")){
    if(!digitalRead(LED_BUILTIN)){
      digitalWrite(LED_BUILTIN, HIGH);
      hora_fim = capturarHora();
      tempo_ativo = calcularTempo(hora_inicio, hora_fim);
      char string[50];
      //Serial.println(tempo_ativo);
      sprintf(string, "%d", tempo_ativo);
      //Serial.println(string);
      client.publish("TEMPO_ATIVO", string);

      hora_inicio = 0;
      hora_fim = 0;
      tempo_ativo = 0;
      timer = 0;
      tempo_desligar = 0;
    }
    else{
      digitalWrite(LED_BUILTIN, LOW);
      hora_inicio = capturarHora();
    }
  }
  else if(!strcmp(topic, "TEMPORIZADOR")){
    if(digitalRead(LED_BUILTIN)){
      digitalWrite(LED_BUILTIN, LOW);
      hora_inicio = capturarHora();
      timer = 1;
    }
    
    int aux = atoi(text);
    Serial.println(aux);
    tempo_desligar =(aux*60) + calcularSegundos(hora_inicio);
  }
}


//procedimento que faz a conexao com o WiFi
void setupWifi(){
  //acende o led para indicar que a placa esta se conectando ao WiFi
  digitalWrite(LED_BUILTIN, LOW);
  delay(10);

  //Setta o buffer
  espClient.setBufferSizes(512, 512);

  //Mostra as informacoes do WiFi
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  //Inicia conexao com WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(". ");
  }

  // Amostra da rede WiFi que esta conectado
  Serial.println();
  Serial.println("Conectado no WiFi!");
  Serial.print("Endereço de IP: ");
  Serial.println(WiFi.localIP());
  
  //Inicia a instancia timeClient para se conectar ao servido ntp
  timeClient.begin();
  while (!timeClient.update())
    timeClient.forceUpdate();

  espClient.setX509Time(timeClient.getEpochTime());
  
  //apaga o led quando a conexao for bem sucedida:
  digitalWrite(LED_BUILTIN, HIGH);
}// end_setupWifi

// procedimento que faz a conexao com o MQTT
void reconnect(){

  //variavel para contagem de erros (se houver):
  int contagem_erro = 0;
  
  //Fica em loop enquanto o cliente nao estiver conectado
  while(!client.connected()){

    //Informa:  
    Serial.print("TENTATIVA DE CONEXAO COM MQTT #");
    Serial.print(contagem_erro);
    Serial.println("...");

    // se o cliente se conectar ao "ESPthing", a conexao eh bem sucedida 
    if(client.connect("ESPthing")){

      //Informa piscando o Led uma vez:
      Serial.println("CONECTADO");
      digitalWrite(LED_BUILTIN, LOW);
      delay(2000);
      digitalWrite(LED_BUILTIN, HIGH);

      //Publica para informar que a placa esta conectada:
      client.publish("CONEXAO","Placa conectada!");

      //Faz a inscrição em tópicos:  
      client.subscribe("TEMPORIZADOR");
      client.subscribe("ON_OFF");
    } //end_if
    else{
      
      contagem_erro++;
      char buf[256];
      
      //Captura o erro decorrente ta tentativa de conexao:
      espClient.getLastSSLError(buf, 256);

      //informa o erro:
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);

      Serial.println("Aguarde 5 segundos para uma nova tentativa.");
      delay(5000);
    } //end_else
  } //end_while
} //end_reconect

//Procedimento que carrega os certificados do AWS
void carregarArquivos(){
  
  //inicia o SPIFFS, ferramenta para acessar a memoria flash da placa para manipulacao de arquivos:
  if(!SPIFFS.begin()){
    //Se nao conseguir, avisa e da um return para o restante do código nao executar:
    Serial.println("FALHA NO SETUP");
    return;
  }
  
  //Mostra o endereço do ponteiro HEAP:
  Serial.print("Heap: ");
  Serial.println(ESP.getFreeHeap());
  
  //Carrega o arquivo de certificado:
  File cert = SPIFFS.open("/cert.der", "r");
  if(!cert)
    Serial.println("Arquivo de certificado de conexão não teve êxito na abertura.");
  else{
    Serial.println("Arquivo de certificado de conexão teve êxito na abertura.");
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
  
  if(espClient.loadCertificate(cert)){
    Serial.println("Arquivo de certificado carregado.");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
  }
  else
    Serial.println("Falha ao carregar o arquivo de certificado.");
  
  //Carrega o arquivo de chave privada:
  File private_key = SPIFFS.open("/private.der", "r");
  if(!private_key)
    Serial.println("Arquivo de chave privada não teve êxito na abertura.");
  else{
    Serial.println("Arquivo de chave privada teve êxito na abertura.");
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
  
  if(espClient.loadPrivateKey(private_key)){
    Serial.println("Arquivo de chave privada carregado.");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
  }
  else
    Serial.println("Falha ao carregar o arquivo de chave privada.");
  
  //Carrega o arquivo do AWS Root:
  File ca = SPIFFS.open("/ca.der", "r");
  if(!ca)
    Serial.println("Arquivo do AWS Root não teve êxito na abertura.");
  else{
    Serial.println("Arquivo do AWS Root teve êxito na abertura.");
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
  
  if(espClient.loadCACert(ca)){
    Serial.println("Arquivo do AWS Root carregado.");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
  }
  else
    Serial.println("Falha ao carregar o arquivo do AWS Root.");
} // end_carregarArquivos

//Captura a data e hora e armazena em um vetor:
int * capturarHora(){
  int * vetor_data = (int*) malloc(sizeof(int)*4);

  vetor_data[0] = timeClient.getDay();
  vetor_data[1] = timeClient.getHours();
  vetor_data[2] = timeClient.getMinutes();
  vetor_data[3] = timeClient.getSeconds();

  Serial.print(vetor_data[1]);
  Serial.print(":");
  Serial.print(vetor_data[2]);
  Serial.print(":");
  Serial.println(vetor_data[3]);
  return vetor_data;
}

//Calcula o tempo, em segundos, do inicio até o fim de um determinado evento:
int calcularTempo(int * data_hora_inicio,int * data_hora_fim){
  int segundo_fim = 0, segundo_inicio = 0;
  
  //captura os segundos do dia em que a led foi ligada:
  segundo_fim = calcularSegundos(data_hora_fim);
    
  //captura os segundos do dia em que a led foi desligada:
  segundo_inicio = calcularSegundos(data_hora_inicio);
  
  //verifica se esta no mesmo dia e retorna o periodo que a led ficou ativa:
  if(data_hora_inicio[0] == data_hora_fim[0])
    return segundo_fim - segundo_inicio;
  else{
    if(data_hora_fim[0] - data_hora_inicio[0] > 0)
      return segundo_fim + (86400 - segundo_inicio) + (86400*(data_hora_fim[0] - data_hora_inicio[0]- 1));
    else
      return segundo_fim + (86400 - segundo_inicio) + (86400*(data_hora_fim[0] - data_hora_inicio[0] + 7));
  }
}

//Calcula o tempo, em segundos, do dia atual
int calcularSegundos(int * data_hora){
  return (data_hora[1]*3600) + (data_hora[2]*60) + data_hora[3];
}
