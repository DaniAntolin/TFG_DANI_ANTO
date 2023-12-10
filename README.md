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
<summary><strong>TDMA</strong></summary>  
El objetivo final es escuchar cada estación, se utilizará un protocolo de comunicacion vía radio llamado <em>TDMA (Acceso Multiple por División de Tiempo)</em>, que consiste en escuchar durante un periodo de tiempo cada canal como se muestra en la siguiente imagen, por lo que no divides el ancho de banda de la señal y puedes escuchar todos los canales en un tiempo determinado.<br>   
<img src="https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/FOTOS/TDMA.jpg" width="200" /><br>   
Ventaja:        No pierdes ancho de banda vease <em>FDMA (Acceso Múltiple de División de Frecuencia)</em>.<br>    
Desventaja:     En cada periodo de tiempo que escuchas una estación, no puedes escuchar las otras, perdiendo información de los otros canales a los que no escuchas.<br>  
<ul>  
<li><em>¿Por qué el uso de TDMA frente a FDMA?</em></li><br>   
</ul>  
- Es simple, el protocolo de comunicación que utiliza la estación Davis no permite dividir el ancho de banda de la señal, haciendo imposible el uso de FDMA. Ademas que el ancho de banda que se utiliza en UE es limitado.<br>   
</details>  
<details>  
<summary><strong>RECEPCION CON FHSS</strong></summary>  
FHSS (Espectro Ensanchado por Salto de Frecuencia), técnica de transmisión de datos inalámbrica que utiliza un ancho de banda mucho mayor que el necesario para transmitir la información. Lo hace mediante el uso de una banda de frecuencia determinada, la cual es dividida en múltiples subfrecuencias. Estas subfrecuencias son saltadas en un orden preestablecido y sincronizado entre el emisor y el receptor.<br>
<ul>
<li><em>¿Cuáles son las ventajas?</em><br></li>  
</ul>
- Se basa en la idea de que un salto rápido y constante entre frecuencias dificulta la interceptación de la señal por parte de terceros. Además, permite una mayor resistencia a interferencias y una transmisión más eficiente de los datos.<br>   
Esto funciona en nuestro caso de tal forma que una vez te sincronzas con el emisor (Estacion davis A) tu vas saltando de frecuencia al mismo tiempo recibiendo todos los paquetes. Pero en el momento que a una frecuencia llega un paquete indeseado, supongamos de nuestra estacion B, ....... a explicar<br>   
</details>
<a name="COMUNICACIONSPI"></a>
<details>  
<summary><strong>COMUNICACIÓN SPI</strong></summary>   
<ol>
<li><p>Selección del esclavo: El maestro configura la línea SS/CS en estado bajo para seleccionar un esclavo específico.</p></li>
<li><p>Generación de la señal de reloj: El maestro genera una señal de reloj en la línea SCLK.</p></li>
<li><p>Transmisión de datos: Durante cada ciclo de reloj, el maestro envía un bit de datos por la línea MOSI y lee un bit de datos de la línea MISO. Esto permite una comunicación Full Duplex, es decir, el maestro puede enviar y recibir datos simultáneamente.</p></li>
<li><p>Registros de desplazamiento: Para que este proceso se haga realidad es necesario la existencia de dos registros de desplazamiento, uno para el maestro y uno para el esclavo respectivamente. Los registros de desplazamiento se encargan de almacenar los bits de manera paralela para realizar una conversión paralela a serial para la transmisión de información.</p></li>
</ol>  
</details>  

<details>  
<summary><strong>Metodos de ahorro de energía Sleep</strong></summary>   
Se usaran metodos de ahorro de energía deep_sleep y light_sleep:<br>  
Los modos que podemos usar son:<br>  
<ol>   
<li>modem-sleep: este modo de ahorro permite desactivar la conexión WiFi de tipo Station , establecida con un punto de acceso (router), cuando no sea necesario su uso y volver a activarla cuando se necesite.. El consumo típico en este modo es de 15mA.</li>  
<li>light-sleep: este modo de ahorro permite mantener la conexión WiFi de tipo Station, pero reduce el consumo de energía en los momentos en los que no hay envío de información.. El consumo típico pasa a ser de unos 0,5 mA.</li>  
<li>deep-sleep: es el modo que genera mayor ahorro, pero a costa de dejar la placa en suspenso. La única parte de la placa que funciona durante este modo es reloj en tiempo real (Real Time Clock o RTC) para poder reiniciarla cuando haya finalizado el tiempo de reposo. El consumo típico pasa a ser de unos 10 uA.</li>  
</ol>  
Además de dispones de los siguientes modos de reinicio.<br>  
<ol>  
<li>WAKE_RF_DEFAULT: Calibración de señal de radio si es necesario</li>  
<li>WAKE_RFCAL: Calibración de señal de radio siempre</li>  
<li>WAKE_NO_RFCAL: Sin calibración de la señal de radio</li>  
<li>WAKE_RF_DISABLED: Deshabilita la señal de radio después del reencendido</li>  
</ol>  
</details>  
<details>  
<summary><strong>Reloj o RTC externo</strong></summary>   
Aunque la placa cuenta con un reloj interno, es necesario utilizar un reloj externo o RTC alimentado por una pila para mantener la hora en caso de por ejemplo un recambio de la batería. 
</details>  

Se hará una escucha de cada canal de Xminutos.  
Una vez escuchado un canal se duerme para que no haya problemas con la comunicacion i2c de la tarjeta SD, ya que comparten ...  
### Guardar.
[**CÓDIGO y DIAGRAMA DE FLUJO**](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/GUARDAR)  
En esta fase, en la tarjeta microSD se creará un archivo con la fecha actual si no está creado. Y dentro los datos obtenidos en la escucha del canal se almacenaran en formato .csv  
Se va a crear un archivo .txt con los datos obtenidos de la radio y luego hacer una copia de ese archivo almacenando los datos para que una vez enviado los datos nuevos obtenidos se eliminen los antiguos, lo que se conoce como *buckup*.  
En esta fase, al igual que la fase anterior, se va a utilizar un protocolo de [COMUNICACIÓN SPI](#COMUNICACIONSPI)  

### Enviar.
[**CÓDIGO y DIAGRAMA DE FLUJO**](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/ENVIAR)  
A la hora de enviar, se va a diseñar dos versiones:  
<details>  
<summary><strong>V1 - CONEXIÓN A PAGINA WEB</strong></summary>      
Una vez obtenido los datos se crea una red wifi donde se crea una direccion ip donde el usuario podrá:<br>  
<img src="https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/FOTOS/webV1.png" align="left" width="200" />   
<em>-cambiar la hora manualmente si se quiere y poder verla</em><br>  
<em>-modificar los tres podibles canales el ID que tiene, introducir -1 en caso de que algún canal no tengas.</em><br>  
<em>-ver los datos almacenados de cada día pulsando en "archivos" y poder descargarselos o eliminarlos</em><br>  
<em>-Cambiar el tiempo de escucha de cada canal sin que este sea inferior a 90segundos, ya que superado este limite habrá una perdida significativa de los datos</em><br>  
<em>-Cambiar el tiempo wifi, que es el tiempo que puede estar el usuario usando el wifi</em><br>  
<br clear="left"/>  
Para acceder a los datos de la pagina que el usuario a podido modificar se hara una solicitud HTTP GET para obtener esos datos.<br>   
<ul>  
<li><strong>BOTON INTERRUPTOR</strong></li><br>  
</ul>  
<em>En esta versión se va a implementar un botón interruptor:</em><br>
<img src="https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/FOTOS/NodoIoT_BOTON_2.jpg" width="200" /><br>   
El uso del botón es para optimizar el uso de la bateria ya que una conexión prolongada del wifi hará que se gaste la batería.<br>   
Este botón se usará para que el usuario que quiera obtener los datos o configurar el datalogger a traves de la página web, conecte el wifi del ESP32 pulsando el botón integrado.<br>  
Una vez haya terminado de manipular la pagina web creada procederá a pulsar otra vez el boton para que se apage el wifi.<br>  
Para solventar un posible problema de que el usuario se le olvide volver a pulsar el boton par desconectar el wifi habra integrado un timer que cuando pase se desconectará automaticamente haciendo que no se pierda bateria inecesaria.<br>  
</details>  
<details>  
<summary><strong>V2 - INFLUXDB Y GRAFANA + RASPBERRY PI</strong></summary>   
Mediante una Raspberry pi se creará un punto de acceso. Este punto de acceso servirá para comectarse desde el datalogger.<br>   
Cada vez que termine de guardar, se intentará conectar al punto de acceso.<br>  
Una vez conectada se actualizará la hora, se hará una conexión http con solicitud get para obtener la información de la pagina influxdb en el puerto 5000 y poder actualizar los datos, que han podido ser modificados por el usuario.<br> 
Los datos obtenidos, que son:<br>   
<em>-modificar los tres podibles canales el ID que tiene.</em><br>  
<em>-Cambiar la zona horaria</em><br>  
<em>-Cambiar el tiempo de escucha de cada canal sin que este sea inferior a 90segundos, ya que superado este limite habrá una perdida significativa de los datos</em><br>  
<em>-Cambiar el tiempo dormido, que será el tiempo que se quede en ahorro de energía</em><br>  
Estas variables se guardarán en el archivo variables.txt<br>  
En el puerto 3000 se graficaran con grafana los datos obtenidos de las distintas estaciones.<br>  
</details>  

