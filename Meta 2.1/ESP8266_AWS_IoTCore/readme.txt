Na pasta data coloque os certificados transformados de .pem para .der:
 openssl x509 -in 2b495edf21-certificate.pem.crt -out cert.der -outform DER 
 openssl rsa -in 2b495edf21-private.pem.key -out private.der -outform DER
 openssl x509 -in AmazonRootCA1.pem -out ca.der -outform DER

Extraia o .zip e coloque a pasta tools (se não tiver, crie, ela vai ficar lado a lado com a pasta libraries) no endereço do sketchbook que está em preferências do arduino.

Em seguida faça as modificações de WiFi e o seu endpoint (disponível em configurações no ambiente do IoT Core) no .ino.

Clique em ferramentas e clique em ESP82666 Sketch Data Upload, espere terminar e depois carregue o código na placa.

 OBS.: Para fazer a conversão, é necessário instalar o openssl na máquina https://slproweb.com/products/Win32OpenSSL.html
