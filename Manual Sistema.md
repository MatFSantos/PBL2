# Sistema de lâmpada Smart  - Manual do Sistema

Nesse manual é explicado como o usuário faz a integração de *softwares* e outras ferramentas para consegui utilizar o ***sistema de lâmpada smart*** de forma simples e concisa. No final desse manual o usuário será capaz de usar o sistema e todas as suas funcionalidades. ***Baixe todos os arquivos referentes ao sistema e mãos à obra!***

## Amazon Web Service (AWS)
O ***sistema de lâmpada Smart*** utiliza o sistema de serviços Web da Amazon (AWS). Serviço esse que disponibiliza um servidor para a utilização do protocolo de comunicação MQTT, através do serviço *IoT Core*, que é utilizado pela placa para comunicação direta com a Web.
Assim, para a utilização do sistema é necessário fazer alguns procedimentos referentes à esse serviço que vão ser explicados à seguir.

###  Criação de uma Thing e uma Police
O primeiro procedimento é a criação de uma *Thing* e sua *Police*. Para isso, entre no site da *AWS educate*  e navegue até o serviço *IoT Core*. No menu lateral, navegue para *Manage/Thing* e crie uma *Thing* única, dê um nome e clique em *next*. Após a *Thing* criada, Navegue no menu lateral para *Secure/Polices* e crie uma *Police*  adicionando um nome, e ao campo ***Action***  coloque ``iot:*`` e no campo ***Resource ARN*** altere  ``topic/replaceWithATopic`` para ``*``, marque a box ``Alow`` e clique em *Create*. Com isso, sua *Thing* e *Police* foram criadas.

### Criação de um certificado
Em seguida, será necessário um certificado para a associação da *Thing* com a *Police*. Para isso, navegue no menu lateral para *Secure/Certificates* e crie um certificado. Após a criação você será direcionado para uma página onde existem alguns arquivos que serão necessários. Faça o download deles: ``A certificate for this thing``; ``A public key``; e ``A private key``. Após isso, abra o link para download de uma *Root CA*. Procure por ``RSA 2048 bit key`` clique no link e o salve. Após guardar esses arquivos, volte para *Secure/Certificates*, vá ao certificado que você criou, clique em ``...`` e vincule a *Thing* e a *Police* criadas anteriormente com a opção *Attach Thing/Pocile*. Pronto, seu certificado foi criado.

### Conversão dos arquivos de certificado
Três arquivos serão utilizados pelo sistema, são esses: ``A certificate for this thing``; ``Root CA``;  e ``A private key``. Os arquivos baixados estão em um formato que não é possível utilizar. Para isso é necessário fazer a conversão destes para um arquivo *.der*. Para fazer isso você pode instalar o software *OpenSSL* e utilizar os comandos que vão ser deixados abaixo, ou pode usar outros meios para essa conversão, contanto que o arquivo ``-certificate.pem.crt`` tenha o nome de ``cert.der``, o arquivo ``-private.pem.key`` tenha o nome de ``private.der`` e o arquivo ``AmazonRootCA1.pem`` tenha o nome de ``ca.der``. Guarde-os, tanto os convertidos, quanto os que foram baixados primeiro.

Comandos para o *OpenSSL*:
- ``openssl x509 -in "código do seu arquivo"-certificate.pem.crt -out cert.der -outform DER``
- ``openssl rsa -in "código do seu arquivo"-private.pem.key -out private.der -outform DER``
- ``openssl x509 -in AmazonRootCA1.pem -out ca.der -outform DER``
Execute-os no terminal do SO.

## A placa NodeMCU
A placa *NodeMCU* é basicamente o circuito que faz todo o controle da sua lâmpada, recebendo requisições e comandos, tanto manuais quanto por meio da rede WiFi usando o Web Site. Para fazer o *setup* correto de configurações da placa, siga esse tópico até o fim.

### Conectar NodeMCU ao Computador
Antes de qualquer coisa, a placa precisa está devidamente conectada ao seu computador para carregá-la com as instruções de execução. Para isso siga os passos a seguir para conectar com sucesso sua placa ao computador:

- Instale a IDE do arduino: *Arduino IDE*;
> Você pode encontrá-la no site oficial da *Arduino*: https://www.arduino.cc/en/software ;

- Conecte sua placa ao computador com um cabo USB e verifique se o seu drive de *USB-Serial CH340* está instalado. Se não, instale-o, será necessário para reconhecer a placa;

 - Abra sua *Arduino IDE*  e navegue na barra de menu até *"Ferramentas->Placa->Gerenciar Placas"* e procure por *esp8266* e instale a versão mais recente;

- Agora navegue no menu novamente até *"Ferramentas->Placa->ESP8266 boards*" e escolha a placa *NodeMCU 1.0(ESP-12E module)*;

- Selecione a porta COM a qual sua placa está conectada em *"Ferramentas->Porta"*. Feche a *Arduino IDE*. Pronto, sua placa está conectada.

###  Integração dos arquivos com a NodeMCU
Após a conversão da extensão e alteração dos nomes dos três arquivos é necessário carregá-los para dentro da placa. Para isso, siga a sequência de passos a seguir, após a conexão da *NodeMCU* e instalação da IDE em seu computador, para carregar os arquivos para a memória da placa:

- Transfira os arquivos convertidos, ``private.der``, ``cert.der`` e ``ca.der``, para a pasta *data* que se encontra no diretório onde está o arquivo ``PBL2_V1.3.ino``  ( normalmente na pasta *PBL2\PBL2_V1.3\\*). Deixe-os lá;
>**OBS:** Caso não tenha pasta *data* nesse diretório, crie-a.

- Vá até a pasta onde se encontram todos os arquivos ( normalmente a pasta *\PBL2*) e procure por uma pasta *zipada* com nome de ``ESP8266FS-0.5.0.zip`` e extraia o conteúdo. Deixe-o lá;

- Abra o *Arduino IDE* e navegue na barra de menu até *Arquivo->Preferências* e veja qual o diretório da sua pasta de *SketchBook*. Navegue até essa pasta (normalmente fica em documentos);

- Chegando na pasta de *SketchBook* crie uma pasta com nome de ``tools`` e coloque a pasta que foi extraída no segundo passo;

- Feche e abra sua *Arduino IDE*;

- Navegue na barra de menu até *Ferramentas* e selecione ``ESP8266 Sketch data upload``;

- Espere o Upload. Pronto, os arquivos já foram carregados na placa.

###  Adicionando credenciais de acesso e dados às instruções da placa
Na placa estão definidas coisas como: nome e senha de WiFi, nome e senha do banco de dados e também dados referente à lâmpada. São algumas credenciais e dados pessoais que são necessárias para o funcionamento do sistema. Então, é necessário que você altere esses dados de acordo com a sua realidade.

Para realizar essa alteração navegue à pasta onde está contida o arquivo ``PBL2_V1.3.ino`` ( normalmente na pasta *PBL2\PBL2_V1.3\\*) e abra-o com a *Arduino IDE*. Já na *Arduino IDE* navegue até o arquivo ``credenciais.h`` e modifique os dados de WiFi, o valor da sua taxa local de energia, a potência da lâmpada e o *EndPoint* da *Thing* que foi criada no AWS.
> **OBS:** O valor da taxa de energia deve ser informado em Wh, ou seja, caso o seu valor local seja em kWh faça a conversão dividindo por 10³.

> **OBS:** Para localizar o *EndPoint* da sua *Thing*, vá ao AWS e entre no IoT Core. No menu lateral navegue para *"Manage->Thing"* e selecione sua *Thing*. Após isso nas informações da *Thing* procure pelo menu *Interact* e copie o link *EndPoint* fornecido. Esse é o seu *EndPoint*.

Após realizar essas alterações, navegue ao outro arquivo ``.h``, ``arduino_secrets.h``, e coloque o *id* e *senha* do banco de dados utilizado. No seu caso, utilize: *lonelynioro* como usuário e *99550011$f* como senha.

### Adicionando as bibliotecas necessárias à Arduino IDE

Para que seja possível carregar as instruções (código) na placa, primeiramente é necessário integrar algumas bibliotecas ao seu arduino. As bibliotecas necessárias são as quatro pastas presentes na pasta ``Libraries`` no diretório fonte do sistema (*\PBL2\Libaries*). Após a localização dessa pasta, copie as pastas dentro e cole no diretório do *Sketchbook* do *Arduino IDE* na pasta com nome de ``libaries`` (geralmente em *Documentos\Arduino\libraries*).

### Carregar código à placa NodeMCU
Por fim, após realizar todos esses passos vocês está pronto para carregar o código em sua placa *ESP8266*. Com sua placa conectada ao computador, abra na *Arduino IDE* o arquivo ``PBL2_V1.3.ino`` ( normalmente na pasta *PBL2\PBL2_V1.3\\*)  e clique no ícone de seta no canto superior direto para carregar o código em sua placa. Espere todo o processo, e pronto, ***sua placa está configurada para uso!***

## Website
O site presente no sistema foi criado a partir do *framework Laravel* e estará presente no diretório do sistema na pasta com nome de ``PBL2-website`` e para fazer uso dessa ferramenta é necessário configurá-la. Para a configuração será necessário a presença do Xampp, do composer e do Laravel em seu computador (Vide como fazer instalação desses recursos).
>Para auxiliar esse processo o software ***Visual Studio Code*** da *Microsoft* é recomendado, porém não é essencial.

### Configurar Website
Abra o terminal do seu SO e navegue à pasta ``PBL2-website``. Execute os comandos a seguir nessa mesma ordem: ``composer install``; ``composer update``; ``php artisan key:generate``. Espere os processos entre esses comandos.

Após executar esses comandos, vá ao diretório do website, ``PBL2\PBL2-website``, crie um arquivo com nome vazio e extensão ``.env``, abra o arquivo ``.env.example`` e copie todo o seu conteúdo para o arquivo ``.env``.

### Credenciais MQTT no Website
Abra o arquivo ``.env`` e vá ao final do mesmo e você verá  vários comandos MQTT.  Serão necessários alterar alguns desses:
- Altere o ``MQTT_HOST`` colocando o *EndPoint* da Thing do AWS;
- Altere, em seguida, o ``MQTT_TLS_CA_FILE``,  o ``MQTT_TLS_CLIENT_CERT_FILE`` e o ``MQTT_TLS_CLIENT_CERT_KEY_FILE``, colocando os diretórios da ``Root CA``, do ``A certificate for this thing`` e da ``A private key``. Salve o arquivo.
> **OBS:** Os arquivos devem ser colocados em seus formatos originais, sem conversão;

Após todo esse processo seu site está pronto para uso! Basta apenas você executar o comando ``php artisan serve`` no terminal na pasta ``PBL2-website`` e abrir no navegador com o endereço passado pelo *Laravel*.
