Na pasta data coloque os certificados transformados de .pem para .der:
 openssl x509 -in 2b495edf21-certificate.pem.crt -out cert.der -outform DER 
 openssl rsa -in 2b495edf21-private.pem.key -out private.der -outform DER
 openssl x509 -in AmazonRootCA1.pem -out ca.der -outform DER

Em seguida faça as modificações de WiFi e o seu endpoint (disponível em configurações no ambiente do IoT Core) no .ino.

 OBS.: Para fazer a conversão, é necessário instalar o openssl na máquina https://slproweb.com/products/Win32OpenSSL.html