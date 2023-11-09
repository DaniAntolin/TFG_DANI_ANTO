#include <InfluxDbClient.h>

#include <HTTPClient.h>
#include <Arduino.h>
#include <RTClib.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SD.h>
#include <NTPClient.h>
#include "servidor_web.h"

RTC_PCF8523 rtc;
InfluxDBClient clientVariables; 
InfluxDBClient client;
String hora;
String minutos;
String segundos;
String anho;
String mes;
String dia;
String fecha;
String dataMessage;
String Nombre_Fichero;

int humedad=0;
int temperatura=0;
int direccion=0;
int velocidad=0;
int canal =0;

int zona_horaria=2;
int tiempo_despertarse=30;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // definido en la libreria datalogger 115200 (Establece la velocidad de datos en bits por segundo (baudios) para la transmisión de datos en serie)
  delay(10);
  //Inizializo SD
  if (!SD.begin(33)) {    // define el pin de selección (33)
    Serial.println("Card Mount Failed 1");
  }else{
    Serial.println("bien configurado tarjeta sd");
  }
  delay(10);
if (!rtc.begin()) { // intenta inicializar el módulo RTC
    Serial.println("No se pudo encontrar el módulo RTC. Verifica la conexión.");
    Serial.flush(); // Espera a que todos los datos pendientes se envíen antes de continuar
    while (1) delay(10);
  }else{
    Serial.println("bien modulo rtc");
  }
  if (!rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    rtc.adjust("2020:01:01:01:01:01");
  }else{
    Serial.println("RTC bien inicializado");
  }
  rtc.start(); //empieza el contador
  Serial.println("empieza el contador");
  getTimeStamp(); // carga el reloj actual
  Serial.println("carga el reloj");

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
WiFi.begin("datalogger", "datalogger"); //Conectarse a la red WiFi ssid: datalogger, contraseña:datalogger
  unsigned long startAttemptTime = millis(); //ver cuando comienza el envio de datos por wifi
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) { // espera hasta que se conecta o pasa el tiempo establecido de 5 segunods
    delay(500); // Esperar medio segundo antes de intentar nuevamente
  }
  if(WiFi.status() == WL_CONNECTED) { //si se cumple es que se ha conectado y no es por que ha transcurrido el tiempo
    Serial.println("Conectado");
    changeInfluxdb(); //protocolo http para cambiar variables.txt con las cambiadas en la nube
    setRTC(); // actualizar hora de forma manual o NTP
    File file = SD.open("/datos.txt", "r"); // abrir el archivo 1 al que se le hizo la copia
    if (!file) { // comprobar que esta bien abierto
      Serial.println("Failed to open file for reading datos.txt");
      return;
    }
    // Read each line and send it to InfluxDB
    String line;
    while (file.available()) { // mira si hay archivos para leer
      line = file.readStringUntil('\n'); // lee una cadena de caracteres hasta que se encuentre un enter
      line.trim(); //  eliminar los espacios en blanco (espacios, tabuladores y retornos de carro) al principio y al final de una cadena de caracteres
      if (line.length() == 0) continue; // Saltar líneas vacías

      String values[6];
      int numValues = 0;
      int startPos = 0;
      int endPos = 0;
      while (endPos != -1) {
        endPos = line.indexOf(';', startPos); //Si la búsqueda es exitosa, devuelve el índice del primer carácter de la coincidencia, si no se encuentra ninguna coincidencia devuelve -1
        if (endPos == -1) {
          values[numValues++] = line.substring(startPos); //extraer una porción específica de un String y crear una nueva cadena que contiene esa porción desde el indice que deseas empezar
        } else {
          values[numValues++] = line.substring(startPos, endPos); // lo mismo pero hasta un indice final
        }
        startPos = endPos + 1;
      }

      Point point("weather"); // point para graficar en grafana
      point.setTime(values[0].toInt() * 1000000000LL - 2 * 3600000000000LL); // Se resta porque Grafana está en formato UTC
      point.addTag("estacion", values[1]);
      Serial.println(values[1]);
      point.addField("temperature", values[2].toFloat());
      Serial.println(values[2]);
      point.addField("humidity", values[3].toFloat());
      Serial.println(values[3]);
      point.addField("velocidad", values[4].toFloat());//toInt
      Serial.println(values[4]);
      point.addField("direccion", values[5].toFloat());//toInt
      Serial.println(values[5]);

      Serial.print("Writing point: ");
      Serial.println(client.pointToLineProtocol(point)); // para pintar en grafana con influxdb
      if (!client.writePoint(point)) {
        Serial.println("Failed to write point to InfluxDB: " + client.getLastErrorMessage());   // hay errores 
      }
    }
    file.close(); // cierra el archivo
    WiFi.mode(WIFI_OFF); // apaga el wifi
    if (SD.remove("/datos.txt")) { // elimina el archivo 1 del que se ha hecho la copia previamente y se ha utilizado para transmitr datos
        Serial.println("Archivo eliminado correctamente");
    } else {
      Serial.println("Error al eliminar el archivo");
    }
  }else{ // si no esta conectado pues se apaga el wifi ya que no se ha podido encontrar ek wifi
      WiFi.mode(WIFI_OFF);
      Serial.println("Wifi no encontrada");
  }
}

void changeInfluxdb() { //protocolo http
  HTTPClient http;
 String query = "SELECT LAST(canal1) AS canal1, LAST(canal2) AS canal2, LAST(canal3) AS canal3, LAST(tiempo_escucha) AS tiempo_escucha, LAST(tiempo_dormido) AS tiempo_dormido, LAST(zona) AS zona FROM VARIABLES.autogen.variables";
// VARIABLES.TXT
String encodedQuery = urlEncode(query.c_str()); // para codificar URL antes de enviar una solicitud HTTP
String url = "http://192.168.4.1:8086/query?db=VARIABLES&q=" + encodedQuery; //la url??



  Serial.println("Enviando solicitud HTTP..."); 
  http.begin(url);  // Especificar la URL de la consulta

  int httpResponseCode = http.GET();  // Enviar la solicitud HTTP GET

  if (httpResponseCode == 200) {
    String response = http.getString();  // Obtener la respuesta del servidor
    Serial.println("Respuesta del servidor:");
    Serial.println(response);

    DynamicJsonDocument jsonDoc(2048);  // Tamaño del documento JSON (ajustar según sea necesario)
    deserializeJson(jsonDoc, response); //para analizar la cadena de caracteres JSON y cargar los datos en el documento JSON.

    // Obtener los valores de las variables
    String canal1_actualizar = jsonDoc["results"][0]["series"][0]["values"][0][1].as<String>();
    String canal2_actualizar = jsonDoc["results"][0]["series"][0]["values"][0][2].as<String>();
    String canal3_actualizar = jsonDoc["results"][0]["series"][0]["values"][0][3].as<String>();
    String t_escucha_actualizar = jsonDoc["results"][0]["series"][0]["values"][0][4].as<String>();
    String t_dormido_actualizar = jsonDoc["results"][0]["series"][0]["values"][0][5].as<String>();
    String zona_actualizar = jsonDoc["results"][0]["series"][0]["values"][0][6].as<String>();
    String mensaje = t_escucha_actualizar + "\n" + t_dormido_actualizar + "\n" + canal1_actualizar + "\n" + canal2_actualizar + "\n" + canal3_actualizar + "\n" + String(tiempo_despertarse)+ "\n" + zona_actualizar;
    modificar_fichero(SD, "/variables.txt", mensaje.c_str());

  } else {
    Serial.print("Código de respuesta: ");
    Serial.println(httpResponseCode);
  }

  http.end(); //termina el protocolo http
}

void setRTC(){ // actualizar hora
  WiFiUDP ntpUDP; // crea un objeto UDP
  NTPClient timeClient(ntpUDP, "192.168.4.1", 0, 0); 
  //obtener la hora actual a través del protocolo NTP, servidor NTP "192.168.4.1", Ajuste de zona horaria en segundos (0 para UTC), Ajuste de horario de verano en segundos (0 si no se aplica)

  timeClient.begin(); //se utiliza para iniciar la funcionalidad de cliente NTP y preparar el objeto NTPClient para realizar solicitudes de tiempo a un servidor NTP en tu red o en Internet
  timeClient.setTimeOffset(0); // este ajuste te permite corregir la hora recibida del servidor NTP según tu zona horaria o cualquier otro ajuste necesario

  timeClient.update(); // obtener la hora actual del servidor NTP

  // Obtener la hora actual del servidor NTP
  unsigned long epochTime = timeClient.getEpochTime(); //obtener el tiempo en formato de marca de tiempo (timestamp) en segundos
  epochTime += 60*60*zona_horaria; // no entiendo
  // Actualizar el RTC externo con la hora obtenida
  DateTime currentTime = DateTime(epochTime); //representar y manipular fechas y horas
  rtc.adjust(currentTime); /* establecerá la fecha y hora en el RTC una vez y no se actualizará automáticamente desde una fuente externa como un servidor NTP
  , OJO no es lo mismo que timeClient.update() puesto que este actualiza la hora a traves de internet en un servidor NTP, 
  mientras que rtc.adjust es para hacerlo de forma manual de forma periodica para tener mas precision*/

  // Realizar otras tareas en tu programa

  // Ejemplo: Imprimir la hora actual del RTC externo
  DateTime now = rtc.now();
  Serial.print("Hora actual del RTC externo: ");
  Serial.println(now.timestamp());
}

void modificar_fichero(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
void getTimeStamp() { //mira el tiempo actual
  DateTime now = rtc.now();
  hora = now.hour();
  minutos = now.minute();
  segundos = now.second();
  anho = now.year();
  mes = now.month();
  dia = now.day();
}


