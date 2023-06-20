# Software embebido para ESP para el trabajo práctico de la materia DAIoT

El siguiente proyecto es el software embebido de los ESP32 para la materia Desarrollo de aplicaciones para IoT. El proyecto consta de 2 componentes de software, el presente para los nodos y el ubicado [aquí](https://github.com/javifanelli/web-proyecto) para los componentes web (frontend y backend).

Este componente fue realizado en el framework ESP-IDF para Visual Studio Code.

## Creacion de certificados

Utilizar estos enlaces
https://www.emqx.com/en/blog/enable-two-way-ssl-for-emqx

### Servidor

* sudo openssl genrsa -out ca.key 2048
* sudo openssl req -x509 -new -nodes -key ca.key -sha256 -days 3650 -out ca.pem
* sudo openssl genrsa -out server.key 2048
* crear openssl.cnf
[req]
default_bits  = 2048
distinguished_name = req_distinguished_name
req_extensions = req_ext
x509_extensions = v3_req
prompt = no
[req_distinguished_name]
countryName = AR
stateOrProvinceName = bsas
localityName = berazategui
organizationName = ceiot
commonName = server
[req_ext]
subjectAltName = @alt_names
[v3_req]
subjectAltName = @alt_names
[alt_names]
IP.1 = 192.168.0.70
DNS.1 = 192.168.0.70

* sudo openssl req -new -key ./server.key -config openssl.cnf -out server.csr
* sudo openssl x509 -req -in ./server.csr -CA ca.pem -CAkey ca.key -CAcreateserial -out server.pem -days 3650 -sha256 -extensions v3_req -extfile openssl.cnf

### Cliente

* sudo openssl genrsa -out client.key 2048
* sudo openssl req -new -key client.key -out client.csr -subj "/C=AR/ST=bsas/L=berazategui/O=ceiot/CN=client"
* sudo openssl x509 -req -days 3650 -in client.csr -CA ca.pem -CAkey ca.key -CAcreateserial -out client.pem



## How to use example

### Hardware Required

This example can be executed on any ESP32 board, the only required interface is WiFi and connection to internet.

### Configure the project

* Open the project configuration menu (`idf.py menuconfig`)
* Configure Wi-Fi or Ethernet under "Example Connection Configuration" menu. See "Establishing Wi-Fi or Ethernet Connection" section in [examples/protocols/README.md](../../README.md) for more details.
* When using Make build system, set `Default serial port` under `Serial flasher config`.

* Generate your client keys and certificate

Navigate to the main directory

```
cd main
```

Generate a client key and a CSR. When you are generating the CSR, do not use the default values. At a minimum, the CSR must include the Country, Organisation and Common Name fields.

```
openssl genrsa -out client.key
openssl req -out client.csr -key client.key -new
```

Paste the generated CSR in the [Mosquitto test certificate signer](https://test.mosquitto.org/ssl/index.php), click Submit and copy the downloaded `client.crt` in the `main` directory.

Please note, that the supplied files `client.crt` and `client.key` in the `main` directory are only placeholders for your client certificate and key (i.e. the example "as is" would compile but would not connect to the broker)

The server certificate `mosquitto.org.crt` can be downloaded in pem format from [mosquitto.org.crt](https://test.mosquitto.org/ssl/mosquitto.org.crt).

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

