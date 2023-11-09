#include <Arduino.h>
#include <RTClib.h>

#include <SD.h>
RTC_PCF8523 rtc;
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
void setup() {
  Serial.begin(115200); // definido en la libreria datalogger 115200 (Establece la velocidad de datos en bits por segundo (baudios) para la transmisión de datos en serie)
  delay(10);
  //Inizializo SD
  if (!SD.begin(33)) {    // define el pin de selección (33)
    Serial.println("Card Mount Failed 1");
  }else{
    Serial.println("bien configurado tarjeta sd");
  }
  delay(10);
  //Inizializo RTC
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
  //cambiar_variables(); // almacena los datos almacenados en un archivo .txt en sus respectivas variables, util??
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
getTimeStamp(); ////mira el tiempo actual
  canal++;
  velocidad++;
  direccion++;
  temperatura++;
  humedad++;
  if (canal > 5){
    canal=1;
    velocidad=1;
    direccion=1;
    temperatura=1;
    humedad=1;
    }
    //
  Nombre_Fichero = "/Estacion" + String(canal) + "_" + String(dia) + "_" + String(mes) + "_" + String(anho) + ".csv"; // crea un fichero de cada estacion por dia
  dataMessage = String(dia) + "-" + String(mes) + "-" + String(anho) + ";" + String(hora) + ":" + String(minutos) + ":" + String(segundos) + ";" + String(temperatura) + ";" + String(humedad) + ";" + String(velocidad) + ";" + String(direccion) + "\n"; // los datos
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  File file = SD.open(Nombre_Fichero.c_str(), FILE_APPEND); //abre el fichero, c_str() devuelve un puntero constante (const char*) que apunta a la memoria interna del objeto, FILE_APPENED no entiendo archivo adjunto whatt??
  if (!file) { //error si no se abre el fichero
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(dataMessage.c_str())) { // escribe el mensaje en el archivo pero no entiendo como funciona el condicional
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close(); // se cierra el archivo
  //

  String fileName = "/datos.txt"; //archivo datos.txt
  File dataFile = SD.open(fileName.c_str(), FILE_APPEND); //abre el archivo
  String dataString = String(rtc.now().unixtime())+";"+ "Estacion_"+String(canal) + ";" + String(temperatura) + ";" + String(humedad) + ";" + String(velocidad) + ";" + String(direccion); //guarda los datos en una variable
  if (dataFile) { // ve si se ha abierto bien el archivo
    dataFile.println(dataString); // escrive los datos de la variable en el archivo 
    dataFile.close(); //cierra el archivo
    Serial.printf("Datos de la estación %d guardados correctamente\n", canal);
  } else {
    Serial.println("Error al abrir el archivo de datos");
  }
  String fileName2 = "/backup.txt"; // lo mismo que el archivo anterior, clonado puesto que utiliza la misma variable para escribir
  File dataFile2 = SD.open(fileName2.c_str(), FILE_APPEND);
  if (dataFile2) {
    dataFile2.println(dataString);
    dataFile2.close();
    Serial.printf("Datos de la estación %d clonado correctamente\n", canal);
  } else {
    Serial.println("Error al abrir el archivo de datos");
  }
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

