#include <Arduino.h>
#include "GUARDAR.h"

void GUARDAR:: inicializar_SD (){
    //Inizializo SD
  if (!SD.begin(33)) {    // define el pin de selección (33)
    Serial.println("Card Mount Failed 1");
  }else{
    Serial.println("bien configurado tarjeta sd");
  }
  leer_variables();
}
void GUARDAR:: inicializar_RTC (){
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
}
void GUARDAR:: INICIALIZAR_GUARDAR (){
    inicializar_RTC();
    inicializar_SD();
}
void GUARDAR:: appendFile (fs::FS &fs, const char * path,const char * message){
  Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}
void GUARDAR:: leer_variables(){
  int error=0;
  error=readFile(SD, "/variables.txt");

  if (error){
    Serial.println("Failed to open file for reading /variables.txt");
    Serial.println("Creating file /variables.txt with actual variables");
    String mensaje = String(tiempo_escucha)+"\n"+String(tiempo_wifi)+"\n"+String(estaciones[0])+"\n"+String(estaciones[1])+"\n"+String(estaciones[2]);
    modificar_fichero(SD, "/variables.txt",mensaje.c_str(), 0, String(""));
  }

}
void GUARDAR:: modificar_fichero(fs::FS &fs, const char * path,const char * message, int var, String valor) {
  Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
        switch (var) {
        case 0: // No hacer nada            
            break;
        case 1: // tiempo escucha
            tiempo_escucha=valor.toInt();
            break;
        case 2: // tiempo wifi
            tiempo_wifi=valor.toInt();
            break;
        case 3: // canal 1
            estaciones[0]=valor.toInt();
            break;
        case 4: // canal 2
            estaciones[1]=valor.toInt();
            break;
        case 5: // canal 3
            estaciones[2]=valor.toInt();
            break;
        default: break;
        }
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void GUARDAR:: GUARDAR_CSV (int canal, int velocidad, int direccion, int temperatura, int humedad) {
  String Nombre_Fichero = "/Estacion" + String(canal) + "_" + String(dia) + "_" + String(mes) + "_" + String(anho) + ".csv"; // crea un fichero de cada estacion por dia
  String dataMessage = String(dia) + "-" + String(mes) + "-" + String(anho) + ";" + String(hora) + ":" + String(minutos) + ":" + String(segundos) + ";" + String(temperatura) + ";" + String(humedad) + ";" + String(velocidad) + ";" + String(direccion) + "\n"; // los datos
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  RECIBIR.radio.sleep(); //Evitar interrupcion de radio durante la escritura de SD
  appendFile(SD, Nombre_Fichero.c_str(), dataMessage.c_str());
  RECIBIR.radio.setChannel(radio.CHANNEL); //Reactivar la radio (se invoca ReceiveBegin())
  //
}
void GUARDAR:: GUARDAR_TXT_BACKUP(int canal, int velocidad, int direccion, int temperatura, int humedad) {
  String fileName = "/datos.txt"; //archivo datos.txt
  String dataString = String(rtc.now().unixtime())+";"+ "Estacion_"+String(canal) + ";" + String(temperatura) + ";" + String(humedad) + ";" + String(velocidad) + ";" + String(direccion); //guarda los datos en una variable
  RECIBIR.radio.sleep(); //Evitar interrupcion de radio durante la escritura de SD
  appendFile(SD, fileName.c_str(), dataString.c_str());
  RECIBIR.radio.setChannel(radio.CHANNEL); //Reactivar la radio (se invoca ReceiveBegin())
  String fileName2 = "/backup.txt"; // lo mismo que el archivo anterior, clonado puesto que utiliza la misma variable para escribir
  RECIBIR.radio.sleep(); //Evitar interrupcion de radio durante la escritura de SD
  appendFile(SD, fileName2.c_str(), dataString.c_str());
  RECIBIR.radio.setChannel(radio.CHANNEL); //Reactivar la radio (se invoca ReceiveBegin())
}
void GUARDAR:: GUARDAR_TODO (int canal, int velocidad, int direccion, int temperatura, int humedad){
    GUARDAR_CSV(int canal, int velocidad, int direccion, int temperatura, int humedad);
    GUARDAR_TXT_BACKUP(int canal, int velocidad, int direccion, int temperatura, int humedad);
}