#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>

//Pino do botao da placa
#define BUTTON D3

//Nome e senha da rede WiFi:
const char * ssid = "Fazendinha";
const char * password = "15253545";

//O end point da thing criada no AWS:
const char * awsEndPoint = "a2jm8vp5br1x7j-ats.iot.us-east-1.amazonaws.com";

//instancia um objeto do tipo WiFiUDP:
WiFiUDP ntpUDP;
//instancia um objeto do tipo NTPClient:
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Declaração de procedimentos:
void callback(char* topic, byte *payload, unsigned int length); // Procedimento chamado quando algum tópico inscrito é atualizado.
void setupWifi(); //Procedimento para conectar a placa ao WiFi.
void reconnect(); //Procedimento para conectar a placa ao MQTT.
void carregarArquivos(); // Procedimento que carrega os arquivos de certificados do AWS.
int * capturarData(); // Procedimento que captura o dia, hora, minuto e segundo do instante em que foi chamado.
int calcularTempo(int fim [4], int inicio [4]); //Calcula qual o período de tempo, em segundos, que se passou do inicio ao fim.
int calcularSegundos(int instante [4]); // Calcula qual o período de tempo, em segundos, que se passou do inicio do dia ao instante.
int * fusoHorario(int data [4]); // Modifica o fuso horário da data para o Br.
void ligarLed(); //Faz a ligação do led.
void desligarLed(); // Desliga o led.

//Variaveis globais:
int * hora_inicio = 0; // instante do dia onde a led ligou.
int * hora_fim = 0; // instante do dia onde a led desligou.
int tempo_desligar = 0; // tempo programado para a led desligar.
int tempo_ligar = 0; // tempo programado para a led ligar.
boolean flag_ligar = false; // flag para o temporizador de ligar.
boolean flag_desligar = false; // flag para o temporizador de desligar.
int contador = 0;

//instancia um objeto do tipo WiFiClientSecure:
WiFiClientSecure espClient;
//Configuração padrão do MQTT:
PubSubClient client(awsEndPoint, 8883, callback, espClient);
char msg[50];

void setup() {
  Serial.begin(115200); //inicia o display serial, para depuração.
  pinMode(LED_BUILTIN, OUTPUT); // inicializa o pino do led como saída.
  pinMode(BUTTON, INPUT_PULLUP); //inicializa o pino do botao como entrada.

  //conexão com o WiFi:
  setupWifi();
  delay(1000);

  //faz o carregamentos dos certificados no espClient:
  carregarArquivos();

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
    if(calcularSegundos(aux_data) == tempo_ligar){
      if(!digitalRead(LED_BUILTIN))
        desligarLed();

        //Reinicia variaveis
        tempo_desligar = 0;
        flag_desligar = false;
    }
    //libera o ponteiro:
    free(aux_data);
  }

  if(contador == 20){
    //Avisa, periodicamente, que a placa esta ativa:
    client.publish("VERIFICAR","{\"status\": \"PLACA ATIVA\"}");
    contador = 0;
  }

  delay(500);
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
  if(strcmp(topic, "ON_OFF")){
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
      tempo_desligar = 0;
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
      }
    }
    //Se time1 não for 0, a led terá um tempo para ligar
    else{
      flag_ligar = true;
      int aux = atoi(time1);
      tempo_ligar = (aux*60) + calcularSegundos(data);
      
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
int *capturarData(){
  int vetor_data[4];

  //capturo a data:
  vetor_data[0] = timeClient.getDay();
  vetor_data[1] = timeClient.getHours();
  vetor_data[2] = timeClient.getMinutes();
  vetor_data[3] = timeClient.getSeconds();

  free(vetor_data);
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
int calcularTempo(int fim [4], int inicio [4]){
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
int calcularSegundos(int instante [4]){
  return (instante[1] * 3600) + (instante[2] * 60) + instante[3]; 
}


/*
 * Faz o desligamento do led, captura a hora e verifica o tempo que a led ficou ligada.
 * Além disso, faz a postagem no MQTT do tempo que a led ficou ligada e do novo estado
 * da mesma.
 * 
 */
void desligarLed(){
  //Desliga o led e captura o tempo total que ficou ligado:
  digitalWrite(LED_BUILTIN, HIGH);
  hora_fim = capturarData();
  int tempo_ativo = calcularTempo(hora_fim, hora_inicio);
  
  //Publica o tempo total que ficou ligada:
  char * string1;
  sprintf(string1, "%d", tempo_ativo);
  char * string2 = "{\"status\": \"";
  char * string3 = "\"}";
  strcat(string2, string1);
  strcat(string2, string3);
  Serial.println(string2);
  client.publish("TEMPO_ATIVO", string2);

  //Publica o novo estado da lampada (desligado):
  client.publish("ESTADO", "{\"status\": \"DESLIGADA\"}");

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
  client.publish("ESTADO","{\"status\": \"LIGADA\"}");
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
int * fusoHorario(int data [4]){
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
