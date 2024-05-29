# NodoIoT_RFM69
# TFG: Implementación de nodo IoT para la captura y monitorización de datos de una estación meteorológica Davis
## Equipo:
### Hardware usado
**Marca Davis:**  
[Sensor Transmitter Davis 6331/2](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/Informaci%C3%B3n/documentos/07395_359_sensor_manual_6331_6332.pdf)  
[Sensor de temperatura y humedad Davis](https://www.meteo-shopping.com/es/sensores/112-sensor-de-temperatura-y-humedad-con-proteccion-contra-la-radiacion.html)  
[Sensor de viento](https://www.meteo-shopping.com/es/sensores/109-anemometro-de-paletas-vantage-pro.html)  
**Nodo IOT:**  
[Placa Adafruit Adalogger FeatherWing](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/Informaci%C3%B3n/documentos/adafruit-adalogger-featherwing.pdf)  
Tarjeta microSD de 2Gbytes   
[PLaca Adafruit Radio FeatherWing](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/Informaci%C3%B3n/documentos/RFM69HCW-V1.1.pdf) 
[Placa Adafruit Feather ESP32 V2](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/Informaci%C3%B3n/documentos/MCU)  
[Antena](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/Informaci%C3%B3n/documentos/1052620001.pdf)  
[Botón interruptor](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/RECIBIR) 
[Batería y placa solar](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/Informaci%C3%B3n/documentos/6612-6614_Kit%20alimentation_FICHE%20PRODUIT_FR_DAVIS.pdf)
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
CS SD -> Pin 33  
IRQ -> Pin 27  
PinBotón -> Pin 12  
PinBotónTierra -> Pin 13  
### Adafruit Radio FeatherWing  
## Desarrollo. 
Para el desarrollo de este proyecto se divirá en tres partes que de forma independiente se podrá testear las diferentes fases del proyecto: [Recibir](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/RECIBIR), [Guardar](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/GUARDAR) y [Enviar](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/ENVIAR).  
Estas tres fases serán librerías en un archivo .h/cpp que posteriormente se harán [pruebas](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/PRUEBAS) para testear de forma independiente el [boton interruptor]().
Finalmente, serán junatadas en un archivo   *.ino* llamado [DEFINITIVO.ino](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/DEFINITIVO) .  
En el transcurso del TFG se recopilarán datos en [ESTADISTICAS](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/ESTADISTICAS), donde:  
*- se medirá la potencia para saber el consumo en función del tiempo con y sin Placa Adafruit Adalogger FeatherWing, tambien radio + SD, radio + SD + WIFI.* [Aquí](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/ESTADISTICAS/Consumo)  
*- se medirá la recepción para los diferentes canales que tiene el transmisor de radio Davis.* [Aquí](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/ESTADISTICAS/estadisticas_radio)   
*- se harán pruebas previas con todo el sistema montado correctamente para ver que no hay problemas ,obtendiendo datos de los mismos en formato xlsx, donde se verificará con la información recopilada de Putty .* [Aquí](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/ESTADISTICAS/Datos%20almacenados)  
### Código final.
[**CÓDIGO](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/DEFINITIVO/Datalogger.ino) 
[**DIAGRAMA DE FLUJO SISTEMA](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/DEFINITIVO/diagramas_flujo/diagramtot.png) 
[**DIAGRAMA DE RECEPCIÓN RADIO](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/DEFINITIVO/diagramas_flujo/diagramradio.png) 
[**DIAGRAMA DE BOTÓN INTERRUPTOR](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/DEFINITIVO/diagramas_flujo/boton_diagram.drawio.png) 
### Recibir.
[**CÓDIGO y DIAGRAMA DE FLUJO**](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/RECIBIR)  
Esta fase tiene como objetivo recibir paquetes de las distintas estaciones con los datos obtenidos en sus sensores. En nuestro caso tenemos sensores de temperatura, humedad, dirección del viento e intensidad.  

El Sensor Transmitter Davis tiene en su interior 4 switches que dependiendo si estan en **ON** u **OFF** representan un canal u otro. Vease la siguiente tabla con información mas detallada  
![Image text](https://github.com/DaniAntolin/TFG_DANI_ANTO/blob/main/FOTOS/tablaIDsRadio.JPG)  
Dependiendo del canal escogido transmitirá mas o menos lento los paquetes segun la siguiente equación:  
**(40 + ID)/16 [segundos]**  

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
Esto funciona en nuestro caso de tal forma que una vez te sincronzas con el emisor (Estacion davis A), vas saltando de frecuencia al mismo tiempo recibiendo todos los paquetes. Pero en el momento que a una frecuencia llega un paquete indeseado, supongamos de nuestra estacion B, en el mismo instante de tiempo y a la misma frecuencia se produce una colisión. Para analizar estas colisiones se ha utilizado un hack_rf y con el software SDR_SHARP se ha podido analizar estos datos. [Detalles de algunas capturas aquí](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/FOTOS/captura_sdr_sharp)<br>   
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

Se utilizará el modo de ahorro de batería light_sleep durante los 2segundos posteriores a una recepción de un paquete de radio, haciendo que sea posible obtener el siguiente paquete, ya que entre un paquete y el siguiente hay una duración minima de 2.5segundos.    
### Guardar.
[**CÓDIGO**](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/GUARDAR)  
En esta fase, en la tarjeta microSD se creará un archivo con la fecha actual si no está creado. Y dentro los datos obtenidos en la escucha del canal se almacenaran en formato .csv  
Se va a crear un archivo .txt con variables que puedan ser modiifcadas por el usuario, como puede ser la fecha y hora, el numero de estaciones, el tiempo que está el wifi activo y el tiempo escucha de cada canal.  
En esta fase, al igual que la fase anterior, se va a utilizar un protocolo de [COMUNICACIÓN SPI](#COMUNICACIONSPI)  

<details>  
<summary><strong>Reloj o RTC externo</strong></summary>   
Aunque la placa cuenta con un reloj interno, es necesario utilizar un reloj externo o RTC alimentado por una pila para mantener la hora en caso de por ejemplo un recambio de la batería. 
</details>  

### Enviar.
[**CÓDIGO**](https://github.com/DaniAntolin/TFG_DANI_ANTO/tree/main/LIBRERIAS/ENVIAR)    
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

