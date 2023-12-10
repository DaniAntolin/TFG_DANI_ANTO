# NodoIoT_RFM69
# TFG: Implementación de nodo IoT para la captura y monitorización de datos de una estación meteorológica Davis
## Equipo:
### Hardware
Sensor Transmitter Davis 6331/2  
Sensor de temperatura y humedad Davis  
Sensor de viento...
Placa Adafruit Adalogger FeatherWing 
Tarjeta microSD de 2Gbytes
PLaca Adafruit Radio FeatherWing  
Placa Adafruit Feather ESP32 V2  
Antena ...  
Botón interruptor  
### Software  
Arduino IDE versión 2.2.1  
Placa Esp32 de espressif version 2.0.4  
## Conexiones pines:

### Placa Adafruit Feather ESP32 V2
![Image text](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/FOTOS/Adafruit_ESP32_Feather_V2_Pinout.png)
SCK -> Pin 5  
MISO -> Pin 21  
MOSI -> Pin 19   
CS RADIO -> Pin 32  
CS CD -> Pin 33  
IRQ -> Pin 27  
PinBotón -> Pin 12  
PinBotónTierra -> Pin 13  
### Adafruit Radio FeatherWing  
## Desarrollo.  
Para el desarrollo de este proyecto se divirá en tres partes que de forma independiente se podrá testear las diferentes fases del proyecto: [Recibir](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/RECIBIR), [Guardar](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/GUARDAR) y [Enviar](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/ENVIAR).  
Estas tres fases serán librerías en un archivo .h/cpp que posteriormente se harán [pruebas](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/PRUEBAS).
Finalmente, serán junatadas en un archivo   *.ino* llamado [DEFINITIVO.ino](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/DEFINITIVO) .  
Tanto las pruebas realizadas como las conclusiones seran archivados en el archivo llamado [ESTADISTICAS](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/ESTADISTICAS), donde:  
*- se medirá la potencia para saber el consumo en función del tiempo con y sin Placa Adafruit Adalogger FeatherWing, tambien radio + SD, radio + SD + WIFI.*  
*- se medirá ... para los diferentes canales que tiene el transmisor de radio Davis.*  
*- se hará testeos del wifi para identificar posibles errores...*  
*- se hara un testeo de 3 dias para ver que no haya problemas con la transmision i2c que comparte.*  

### Recibir.
[**CÓDIGO y DIAGRAMA DE FLUJO**](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/RECIBIR)  
Esta fase tiene como objetivo recibir paquetes de las distintas estaciones con los datos obtenidos en sus sensores. En nuestro caso tenemos sensores de temperatura, humedad, dirección del viento e intensidad.  

El Sensor Transmitter Davis tiene en su interior 4 switches que dependiendo si estan en **ON** u **OFF** representan un canal u otro. Vease la siguiente tabla con información mas detallada  
![Image text](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/FOTOS/tablaIDsRadio.JPG)  
Dependiendo del canal escogido transmitirá mas o menos lento los paquetes segun la siguiente equación:  
**(41 + ID)/16 [segundos]**  

**- PROTOCOLO DE COMUNICACION RADIO**  
<details>  
<summary>TDMA</summary>  
El objetivo final es escuchar cada estación, se utilizará un protocolo de comunicacion vía radio llamado *TDMA* (Acceso Multiple por División de Tiempo), que consiste en escuchar durante un periodo de tiempo cada canal como se muestra en la siguiente imagen, por lo que no divides el ancho de banda de la señal y puedes escuchar todos los canales en un tiempo determinado.  
![Image text](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/FOTOS/TDMA.jpg)  
Ventaja:        No pierdes ancho de banda vease FDMA (Acceso Múltiple de División de Frecuencia).   
Desventaja:     En cada periodo de tiempo que escuchas una estación, no puedes escuchar las otras, perdiendo información de los otros canales a los que no escuchas.  
*¿Por qué el uso de TDMA frente a FDMA?*  
- Es simple, el protocolo de comunicación que utiliza la estación Davis no permite dividir el ancho de banda de la señal, haciendo imposible el uso de FDMA. Ademas que el ancho de banda que se utiliza en UE es limitado.  
</details>  
<details>  
<summary>RECEPCION CON FHSS</summary>  
FHSS (Espectro Ensanchado por Salto de Frecuencia), técnica de transmisión de datos inalámbrica que utiliza un ancho de banda mucho mayor que el necesario para transmitir la información. Lo hace mediante el uso de una banda de frecuencia determinada, la cual es dividida en múltiples subfrecuencias. Estas subfrecuencias son saltadas en un orden preestablecido y sincronizado entre el emisor y el receptor.  
*Cuáles son las ventajas?*  
se basa en la idea de que un salto rápido y constante entre frecuencias dificulta la interceptación de la señal por parte de terceros. Además, permite una mayor resistencia a interferencias y una transmisión más eficiente de los datos.  
Esto funciona en nuestro caso de tal forma que una vez te sincronzas con el emisor (Estacion davis A) tu vas saltando de frecuencia al mismo tiempo recibiendo todos los paquetes. Pero en el momento que a una frecuencia llega un paquete indeseado, supongamos de nuestra estacion B, ....... a explicar  
</details>
<a name="COMUNICACIONI2C"></a>
<details>  
<summary>COMUNICACIÓN I2C</summary>   
</details>  

Se hará una escucha de cada canal de Xminutos.  
Una vez escuchado un canal se duerme para que no haya problemas con la comunicacion i2c de la tarjeta SD, ya que comparten ...  
### Guardar.
[**CÓDIGO y DIAGRAMA DE FLUJO**](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/GUARDAR)  
En esta fase, en la tarjeta microSD se creará un archivo con la fecha actual si no está creado. Y dentro los datos obtenidos en la escucha del canal se almacenaran en formato .csv  
En esta fase, al igual que la fase anterior, se va a utilizar un protocolo de [COMUNICACIÓN I2C](#COMUNICACIONI2C)  

### Enviar.
[**CÓDIGO y DIAGRAMA DE FLUJO**](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/ENVIAR)  
Se va a crear un archivo .txt con los datos obtenidos de la radio y luego hacer una copia de ese archivo almacenando los datos para que una vez enviado los datos nuevos obtenidos se eliminen..... a redactar mejor   
a la hora de enviar, va a diseñar dos versiones:  
<details>  
<summary>V1 - CONEXIÓN A PAGINA WEB</summary>    
<pre>  
Una vez obtenido los datos se crea una red wifi donde se crea una direccion ip donde el usuario podrá:  
<img src="https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/FOTOS/webV1.png" align="left" width="200" />   
*-cambiar la hora manualmente si se quiere y poder verla*  
*-modificar los tres podibles canales el ID que tiene, introducir -1 en caso de que algún canal no tengas.*  
*-ver los datos almacenados de cada día pulsando en "archivos" y poder descargarselos o eliminarlos*  
*-Cambiar el tiempo de escucha de cada canal sin que este sea inferior a 90segundos, ya que superado este limite habrá una perdida significativa de los datos*  
*-Cambiar el tiempo wifi, que es el tiempo que puede estar el usuario usando el wifi*  
<br clear="left"/>  
Para acceder a los datos de la pagina que el usuario a podido modificar se hara una solicitud HTTP GET para obtener esos datos.   
**EN ESTA VERSION SE VA A IMPLEMENTAR UN BOTON:**  
<img src="https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/FOTOS/NodoIoT_BOTON_2.jpg" width="200" />   
El uso del botón es para optimizar el uso de la bateria ya que una conexión prolongada del wifi hará que se gaste la batería.   
Este botón se usará para que el usuario que quiera obtener los datos o configurar el datalogger a traves de la página web, conecte el wifi del ESP32 pulsando el botón integrado.  
Una vez haya terminado de manipular la pagina web creada procederá a pulsar otra vez el boton para que se apage el wifi.  
Para solventar un posible problema de que el usuario se le olvide volver a pulsar el boton par desconectar el wifi habra integrado un timer que cuando pase se desconectará automaticamente haciendo que no se pierda bateria inecesaria.  
</pre>  
</details>  
<details>  
<summary>V2 - INFLUXDB Y GRAFANA + RASPBERRY PI</summary>  
<pre>  
Mediante una Raspberry pi se creará un punto de acceso. Este punto de acceso servirá para comectarse desde el datalogger. Cada vez que termine de guardar, se intentará conectar al punto de acceso. Una vez conectada se actualizará la hora, se hará una conexión http con solicitud get para obtener la información de la pagina influxdb en el puerto 5000. los datos obtenidos, que son: ..... se guardaran en el archivo variables.txt  
En el puerto 3000 se graficaran con grafana los datos obtenidos de las distintas estaciones.  
</pre>  
</details>  

