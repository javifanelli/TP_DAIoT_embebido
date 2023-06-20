# Software embebido para ESP para el trabajo práctico de la materia DAIoT (sensores dummies)

El siguiente proyecto es el software embebido de los ESP32 para la materia Desarrollo de aplicaciones para IoT. El proyecto consta de 2 componentes de software, el presente para los nodos y el ubicado [aquí](https://github.com/javifanelli/web-proyecto) para los componentes web (frontend y backend).

Este componente fue realizado en el framework ESP-IDF para Visual Studio Code. Esta rama tiene el código de los sensores que reportan temperatura hardcodeada que aumenta y disminuye con cada ciclo de 30 segundos. Para ver el códiog del sensor que reporta temperatura real dirigirse a la rama *main* de este proyecto.

## Creacion de certificados

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

### Siguientes pasos

Es importante aclarar que los certificados deben crearse en el servidor y guardarlos de forma segura, ya que sin estos no será posible conectarse al broker MQTT. En este caso el servidor está implementado en una Raspberry Pi400, como se aclara en el readme del componente web.

Una vez creados los certificados, se deben copiar en la carpeta *main* de este proyecto y adecuar las siguientes líneas de la librería *mqtt_funcs.c*:
extern const uint8_t client_cert_pem_start[] asm("_binary_client_pem_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_pem_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_pem_end");

Además se debe modificar el archivo *CMakeLists.txt* ubicado en la carpeta raíz de este componente, agregando los archivos de las claves al final del mismo. Aquí se ve el ejemplo:
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "main/client.pem" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "main/client.key" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "main/ca.pem" TEXT)

- Enlaces útiles: https://www.emqx.com/en/blog/enable-two-way-ssl-for-emqx

## Configurar el proyecto

