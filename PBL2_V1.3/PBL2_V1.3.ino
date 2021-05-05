/* Biblioteca para capturar o timestamp*/
#include <TimeLib.h>

/*Bibliotecas para fazer a comunicação com o MQTT usando o broker do AWS*/
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>

/*Biblioteca para utilizar a memória EEPROM da ESP8266*/
#include <EEPROM.h>

/*Bibliotecas para fazer a comunicação da placa com o banco de dados MySQL*/
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

/*Arquivo contendo usuário e senha do MySQL*/
#include "arduino_secrets.h"

/*Arquivo contendo usuário e senha do WiFi e o endPoint da coisa da AWS*/
#include "credenciais.h"

/******************************************************************************/
/*****VARIAVEIS E CONSTANTES DEFINIDAS PARA A CAPTURA DO TIMESTEMP:***********/
/*****************************************************************************/
//Servidor NTP usado para o timestemp
static const char ntpServerName[] = "us.pool.ntp.org";

//Time zone de São Paulo, Brasil
const int timeZone = -3;

//instancia de um objeto WiFiUDP
WiFiUDP Udp;

// porta local para ouvir pacotes UDP
unsigned int localPort = 8888;

//buffer para armazenar os pacotes de entrada e saída
byte packetBuffer[NTP_PACKET_SIZE];
/***************************************************************************************/
/****************************DECLARAÇÃO DE PROCEDIMENTOS********************************/
/***************************************************************************************/
void callback(char* topic, byte *payload, unsigned int length); // Procedimento chamado quando algum tópico inscrito é atualizado.
void setupWifi(); //Procedimento para conectar a placa ao WiFi.
void reconnect(); //Procedimento para conectar a placa ao MQTT.
void carregarArquivos(); // Procedimento que carrega os arquivos de certificados do AWS.
int * capturarData(); // Procedimento que captura o dia, hora, minuto e segundo do instante em que foi chamado.
int calcularTempo(int * fim, int * inicio ); //Calcula qual o período de tempo, em segundos, que se passou do inicio ao fim.
int calcularSegundos(int * instante); // Calcula qual o período de tempo, em segundos, que se passou do inicio do dia ao instante.
int * fusoHorario(int * data); // Modifica o fuso horário da data para o Br.
void ligarLed(); //Faz a ligação do led.
void desligarLed(); // Desliga o led.
void enviarLogs(String data, float energia, float custo);
void enviarEstado(String estado);
void sendNTPpacket(IPAddress &address);
time_t getNtpTime();

/***************************************************************************************/
/*******VARIAVEIS E CONSTANTES DEFINIDOS PARA A CONEXÃO COM O AWS E COM O WIFI:*********/
/***************************************************************************************/
//Nome e senha da rede WiFi:
const char * ssid = USER_WIFI;
const char * password = PASSWORD_WIFI;

//O end point da thing criada no AWS:
const char * awsEndPoint = ENDPOINT_AWS;

//instancia um objeto do tipo WiFiUDP:
WiFiUDP ntpUDP;
//instancia um objeto do tipo NTPClient:
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//instancia um objeto do tipo WiFiClientSecure:
WiFiClientSecure espClient;
//Configuração padrão do MQTT:
PubSubClient client(awsEndPoint, 8883, callback, espClient);
char msg[50];

/***************************************************************************************/
/*********VARIAVEIS, DEFINES E CONSTANTES PARA CÁLCULOS NA LÓGICA DO PROGRAMA***********/
/***************************************************************************************/

//Pino do botao da placa
#define BUTTON D3

// Para verificação da EEPROM:
#define VERIFY 18

//Variaveis globais:
int * hora_inicio = 0; // instante do dia onde a led ligou.
int * hora_fim = 0; // instante do dia onde a led desligou.
int tempo_desligar = 0; // tempo programado para a led desligar.
int tempo_ligar = 0; // tempo programado para a led ligar.
boolean flag_ligar = false; // flag para o temporizador de ligar.
boolean flag_desligar = false; // flag para o temporizador de desligar.
int contador = 0; //contador para mandar periodicamente o tópico VERIFICAR.

//Taxa de energia e potência da lampada (teoricas):
const double TAXA =  VALOR_TAXA; // reais por Wh
const double POTENCIA =  VALOR_POTENCIA; //em watts

/***************************************************************************************/
/*********************VARIAVEIS E OBJETOS PARA  A CONEXÃO COM O MySQL*******************/
/***************************************************************************************/

//Endereço IP do banco MySQL usado:
IPAddress server_addr(85,10,205,173);

//usuário e senha do banco MySQL:
char user[] = SECRET_USERDB;
char pass[] = SECRET_PASSDB;

//Comando para mandar os dados para o banco de dados:
char INSERT_SQL_LOGS[] = "INSERT INTO log_node.logs (data, energia, custo) VALUES ('%s', '%f', '%f')";
char INSERT_SQL_STATUS[] = "UPDATE log_node.status SET estado ='%s' WHERE 1";
char query[128];

//Objeto conn, com um clientSQL para a conexão com o MySQL:
WiFiClient clientSQL;
MySQL_Connection conn((Client *)&clientSQL);

/****************************************************************************************/
/****************************************************************************************/

void setup() {
  Serial.begin(115200); //inicia o display serial, para depuração.
  pinMode(LED_BUILTIN, OUTPUT); // inicializa o pino do led como saída.
  pinMode(BUTTON, INPUT_PULLUP); //inicializa o pino do botao como entrada.

  //conexão com o WiFi:
  setupWifi();
  delay(1000);

  //Realiza a conexão com o MySQL:
  while (!conn.connect(server_addr, 3306, user, pass)) {
    Serial.println("Conexão SQL falhou.");
    conn.close();
    Serial.println("passou do conn");
    delay(1000);
    Serial.println("Conectando SQL novamente.");
  }
  Serial.println("Conectado ao servidor SQL."); 

  delay(1000);
  //faz o carregamentos dos certificados no espClient:
  carregarArquivos();

  //Conecta a placa ao MQTT
  reconnect();

  //inicia o objeto Udp na portal local:
  Udp.begin(localPort);

  //Faz a sincronização do provedor para capturar o timestemp correto:
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  //Inicia a EEPROM:
  EEPROM.begin(4);

  //Verifica se tem dados Salvos:
  if(EEPROM.read(0) == VERIFY){

    //Se tiver, captura e os envia para o banco de dados:
    int tempo_ativo = EEPROM.read(1);
    double energia;
    double custo;
  
    //Envia para o banco de dados o tempo total que ficou ligada:
    energia = (tempo_ativo/3600)*POTENCIA;
    custo = energia*TAXA;
    char data[15];

    //captura a data atual e transforma em string:
    if(day()<10 && month()<10)
      sprintf(data, "0%d/0%d/%d", day(), month(), year());
    else if(day()<10 && month()>=10)
      sprintf(data, "0%d/%d/%d", day(), month(), year());
    else if(month()<10 && day()>=10)
      sprintf(data, "%d/0%d/%d", day(), month(), year());
    else
      sprintf(data, "%d/%d/%d", day(), month(), year());
      
    //Envia a data, a energia e o custo para o banco de dados:
    enviarLogs(data, energia, custo);
  
    //envia para o banco de dados o novo estado da lampada (desligado):
    enviarEstado("DESLIGADO");
  }

}

void loop() {
  if(!client.connected())
    reconnect();

  //Verifica se os tópicos inscritos foram alterados.
  //se sim, executa callback:
  client.loop();

  //Verifica se o button foi pressionado:
  if(!digitalRead(BUTTON)){
    //Verifica o estado da lâmpada:
    if(!digitalRead(LED_BUILTIN))
      desligarLed();
    else
      ligarLed();
  }

  //verifica se existe um temporizador para ligar:
  if(flag_ligar){
    //captura a hora:
    int * aux_data = capturarData();
    if(calcularSegundos(aux_data) == tempo_ligar){
      //Verifica se a led já está ligada
      if(digitalRead(LED_BUILTIN))
        ligarLed();
      
      //Reinicia variaveis
      tempo_ligar = 0;
      flag_ligar = false;
    }
    //libera o ponteiro:
    free(aux_data);
  }

  if(flag_desligar){
    //captura a hora:
    int * aux_data = capturarData();
    if(calcularSegundos(aux_data) == tempo_desligar){
      if(!digitalRead(LED_BUILTIN))
        desligarLed();

        //Reinicia variaveis
        tempo_desligar = 0;
        flag_desligar = false;
    }
    //libera o ponteiro:
    free(aux_data);
  }

  if(contador == 30){
    //Avisa, periodicamente, que a placa esta ativa:
    client.publish("VERIFICAR","{\"status\": \"PLACA ATIVA\"}");
    contador = 0;
  }

  //Verifica se o led ta ligado para armazenar os dados na memória EEPROM:
  if(!digitalRead(LED_BUILTIN)){

    //Se tiver ligado, salva o VERIFY na primeira posição da EEPROM
    EEPROM.write(0, VERIFY);

    //Captura o instante e calcula o tempo que ela está ligada:
    int * tempo = capturarData();
    int tempo_ativo = calcularTempo(tempo, hora_inicio);

    //Salva o tempo ativo da led na posição 2 da EEPROM e "commita" a mudança:
    EEPROM.write(1, tempo_ativo);
    EEPROM.commit();

    //Libera espaço:
    free(tempo);
  }
  else{
    //Se não tiver ligada, limpa a primeira posição da EEPROM e "commita" a mudança:
    EEPROM.write(0,0);
    EEPROM.commit();
  }
  
  //delay baixo para o button ser mais responsivo:
  delay(300);
  contador++;
}

/*
 * O procedimento que é chamada sempre que algum tópico assinado é atualizado
 * 
 * Parâmetros:
 *    char *topic         -> nome do tópico que foi atualizado;
 *    byte * payload      -> conteúdo do tópico que foi enviado;
 *    unsigned int length -> tamanho do conteúdo que foi enviado.
 */
void callback(char * topic, byte * payload, unsigned int length){
  //mostra no serial o que foi recebido:
  Serial.print("Mensagem recebida no tópico ");
  Serial.println(topic);

  //Verifica em qual tópico foi recebida a mensagem:
  if(!strcmp(topic, "ON_OFF")){
    //Verifica se a led esta ligada:
    if(!digitalRead(LED_BUILTIN))
      desligarLed();
    else
      ligarLed();
  }
  else if(!strcmp(topic, "TEMPORIZADOR")){

    //Captura a mensagem recebida no topico topic:
    char time1[10], time2[10], a  = (char) payload[0];
    int i = 0, j = 0;

    //enquanto não encontrar a vírgula o conteudo de payload vai para time1:
    while(a != ','){
      time1[i] = (char) payload[i];
      i++;
      a = (char) payload[i];
    }
    //encontrou a virgula, finaliza a string com '\0' e passa para a próxima posição de payload (i++):
    time1[i] = '\0';
    i++;
    //enquanto nao chegar ao fim da payload, o seu conteudo vai para time2:
    while(i<length){
      time2[j] = (char) payload[i];
      j++;
      i++;
    }
    //encontrou o fim de payload, finaliza a string com '\0':
    time2[j] = '\0';

    Serial.println(time1);
    Serial.println(time2);
    
    //capturo a data do instante atual:
    int * data = capturarData();
    
    //Verifico se time1 é 0:
    if(!strcmp(time1, "0")){
      //Se for, o led é ligado (caso nao esteja):
      flag_ligar = false;
      tempo_ligar = 0;
      if(digitalRead(LED_BUILTIN))
        ligarLed();
      
      //Reinicio a flag e tempo de desligar:
      flag_desligar = false;
      tempo_desligar = 0;
      
      //se time2 for diferente de 0, a led terá um tempo para desligar:
      if(strcmp(time2, "0")){
        
        //calculo o tempo de desligar, a partir de agora:
        int aux = atoi(time2);
        tempo_desligar = (aux*60) + calcularSegundos(data);
        
        flag_desligar = true;
        
        if(tempo_desligar > 86400)
          tempo_desligar = tempo_desligar - 86400;
      }
    }
    //Se time1 não for 0, a led terá um tempo para ligar
    else{
      flag_ligar = true;
      int aux = atoi(time1);
      tempo_ligar = (aux*60) + calcularSegundos(data);

      if(tempo_ligar > 86400)
        tempo_ligar = tempo_ligar - 86400;
      
      //Reinicio a flag e tempo de desligar:
      flag_desligar = false;
      tempo_desligar = 0;

      //Se tiver um tempo para desligar:
      if(strcmp(time2,"0")){
        aux = atoi(time2);
        tempo_desligar = (aux*60) + tempo_ligar;
        flag_desligar = true;
      }
    }
    free(data);
  }
}

/*
 * Procedimento que realiza a conexão com o WiFi
 * 
 */
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
}

/* 
 *  Procedimento que realiza a conexão da placa com o MQTT ( broker)
 *  
 */
void reconnect(){
  
  //variavel para contagem de erros (se houver):
  int contagem_erro = 0;
  
  //Fica em loop enquanto o cliente nao estiver conectado
  while(!client.connected()){

    //Informa:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("TENTATIVA DE CONEXAO COM MQTT #");
    Serial.print(contagem_erro);
    Serial.println("...");

    // se o cliente se conectar ao "ESPthing", a conexao eh bem sucedida 
    if(client.connect("ESPthing")){
      delay(1500);
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("CONECTADO");
      
      //Publica para informar que a placa esta conectada:
      client.publish("CONEXAO","{\"status\": \"PLACA CONECTADA\"}");

      //Faz a inscrição em tópicos:  
      client.subscribe("TEMPORIZADOR");
      client.subscribe("ON_OFF");
    }
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
    }
  }
}


/*
 * Procedimento que faz a leitura dos certificados do AWS armazenados na memória da placa,
 * e os carrega para o objeto espClient, que faz o tratamento de segurança do AWS.
 * 
 */
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
}


/*
 * Captura um instante de tempo, contendo dia da semana, hora, minuto e segundo.
 * 
 * return;
 *    vetor contendo dia da semana, hora, minuto e segundo.
 */
int * capturarData(){
  int * vetor_data = (int*) malloc(sizeof(int)*4);

  //capturo a data:
  vetor_data[0] = timeClient.getDay();
  vetor_data[1] = timeClient.getHours();
  vetor_data[2] = timeClient.getMinutes();
  vetor_data[3] = timeClient.getSeconds();

  return fusoHorario(vetor_data);
}


/*
 * Calcula quanto tempo se passou, em segundos, de um instante inicio até um instante fim.
 * 
 * Parâmetros
 *    int inicio[4] -> instante inicio capturado, tendo dia da semana, hora, minuto e segundo;
 *    int fim[4]    -> instante fim capturado, tendo dia da semana, hora, minuto e segundo.
 * 
 * return:
 *    o tempo que se passou do inicio para o fim (parâmetros) em segundos.
 */
int calcularTempo(int * fim, int * inicio){
  int segundos_fim = 0, segundos_inicio = 0;

  //captura os segundos do dia em que a led foi ligada:
  segundos_fim = calcularSegundos(fim);
  
  //captura os segundos do dia em que a led foi desligada:
  segundos_inicio = calcularSegundos(inicio);
  
  //Verifica se não houve um "passar de dias" e retorna o período que a led ficou ativa:
  if(inicio[0] == fim[0])
    return segundos_fim - segundos_inicio;
  else if(fim[0] - inicio[0] > 0)
    return segundos_fim + (86400 - segundos_inicio) + (86400 * (fim[0] - inicio[0] - 1));
  else
    return  segundos_fim + (86400 - segundos_inicio) + (86400*(fim[0] - inicio[0] + 7));
}

/*
 * Calcula os Segundos desde o inicio do dia até um instante capturado.
 * 
 * Parâmetros:
 *    int instante[4] -> instante capturado contendo hora, minuto e segundo.
 *
 * return:
 *    retorna o tempo, em segundos, do dia.
 */
int calcularSegundos(int * instante){
  return (instante[1] * 3600) + (instante[2] * 60) + instante[3];
}


/*
 * Faz o desligamento do led, captura a hora e verifica o tempo que a led ficou ligada.
 * Além disso, faz a postagem no MQTT do tempo que a led ficou ligada e do novo estado
 * da mesma.
 * 
 */
void desligarLed(){
  double horas;
  double energia;
  double custo;
  //Desliga o led e captura o tempo total que ficou ligado:
  digitalWrite(LED_BUILTIN, HIGH);
  hora_fim = capturarData();
  
  int tempo_ativo = calcularTempo(hora_fim, hora_inicio);
  
  //Publica o tempo total que ficou ligada:
  horas =((double) tempo_ativo)/3600.0; //pego a hora
  energia = horas*POTENCIA; //pego a energia em Wh
  custo = energia*TAXA; 
  char data[15];

  //captura a data atual e transforma em string:
  if(day()<10 && month()<10)
    sprintf(data, "0%d/0%d/%d", day(), month(), year());
  else if(day()<10 && month()>=10)
    sprintf(data, "0%d/%d/%d", day(), month(), year());
  else if(month()<10 && day()>=10)
    sprintf(data, "%d/0%d/%d", day(), month(), year());
  else
    sprintf(data, "%d/%d/%d", day(), month(), year());

  enviarLogs(data, energia, custo);
  //Publica o novo estado da lampada (desligado):
  enviarEstado("DESLIGADO");

  //Reinicia todas as variáveis
  free(hora_inicio);
  free(hora_fim);
  if(!flag_ligar){
    tempo_desligar = 0;
    flag_desligar = false;
  }
}

/*
 * Faz a ligação do led e informa, ao MQTT, seu novo estado além de capturar
 * o seu horário de inicio.
 */
void ligarLed(){
  digitalWrite(LED_BUILTIN, LOW);
  hora_inicio = capturarData();
  
  //Publica o novo estado da lampada (ligada):
  enviarEstado("LIGADO");
}

/*
 * Altera o fuso horário para o horário de brasília.
 * 
 * Parâmetros:
 *    int data[4] -> a data passada no fuso horário -3 hrs em relação ao fuso horário de brasília
 * 
 * return:
 *    retorna o fuso horário correto.
 */
int * fusoHorario(int * data){
  if(data[1] >= 3)
    data[1] = data[1] - 3;
  else{
    data[1] = data[1] + 21;
    if(data[0] == 1)
      data[0] = 7;
    else
      data[0] = data[0] - 1;
  }
  return data;
}

/*
 * Procedimento que faz a sincronização com o provedor.
 * 
 * return:
 *    Retorna os segundos, desde 1970 até o instante foi chamada.
 *    Retorna 0 se não conseguir sincronizar.
 */
time_t getNtpTime(){
  IPAddress ntpServerIP; //enderço IP do servidor NTP

  //descartar todos os pacotes recebidos anteriormente
  while (Udp.parsePacket() > 0);
  // obtém um servidor aleatório do pool
  WiFi.hostByName(ntpServerName, ntpServerIP);

  //Envia uma solicitação NTP para o servidor obtido aleatóriamente:
  sendNTPpacket(ntpServerIP);

  //Permanece no loop por 1500 ms:
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      //ler o pacote no buffer Udp:
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long secsSince1900;
      // Converte quatro bytes começando na localização 40 em um long int:
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  
  return 0; // Retorna 0 se não for possível obter a hora
}

/*
 * Envia uma solicitação NTP para o servidor de horário no endereço fornecido
 * 
 * Parâmetros:
 *    endereço IP do servidor.
 */
void sendNTPpacket(IPAddress &address){
  // Setta todos os bytes no buffer para 0:
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  
  // Inicializa os valores necessários para formar a solicitação NTP:
  packetBuffer[0] = 0b11100011;   // LI, versão, modo
  packetBuffer[1] = 0;     // estrado, ou tipo de clock
  packetBuffer[2] = 6;     // Intervalo de votação
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes de zero para Root Delay e Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  //Agora, envia um pacote solicitando um carimbo de data/hora:
  Udp.beginPacket(address, 123); // Os pedidos NTP são para a porta 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

/*
 * Envia os dados de Log para a tabela do banco de dados MySQL Logs
 * 
 * Parâmetros:
 *    char * data -> data do instante que está enviando (timestemp);
 *    double energia -> energia gasta pela Led;
 *    double custo -> preço da energia gasta pelo Led;
 */
void enviarLogs(char * data, double energia, double custo) {
  //transforma os dados em um comando do MySQL:
  sprintf(query, INSERT_SQL_LOGS, data, energia, custo);
  // Inicia a instancia da classe de consulta:
  MySQL_Cursor * cur_mem = new MySQL_Cursor(&conn);
  //Executa a consulta (query):
  cur_mem->execute(query);

  // deleta o cursor para liberar memória:
  delete cur_mem;
}

/*
 * Envia o Estado da lampada para a tabela do banco de dados MySQL estado
 * 
 * Parâmetros:
 *    char * estado -> Estado atual da lampada.
 *    
 */
void enviarEstado(char * estado){
  //transforma o estado em um comando do MySQL:
  sprintf(query, INSERT_SQL_STATUS, estado);
  // Inicia a instancia da classe de consulta:
  MySQL_Cursor * cur_mem = new MySQL_Cursor(&conn);
  //Executa a consulta (query):
  cur_mem->execute(query);
  
  // deleta o cursor para liberar memória:
  delete cur_mem;
}
