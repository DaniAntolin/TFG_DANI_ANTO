#ifndef GUARDAR_h
#define GUARDAR_h

#include <Arduino.h>
#include <RTClib.h>
#include <SD.h>
#include "RECIBIR.h" 
class RECIBIR{
  private:
    RECIBIR RECIBIR;
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
    int tiempo_escucha=90;  //(seg) Tiempo máximo de escucha de cada canal para recibir todos los datos
    int tiempo_wifi=5;  //(min) Tiempo máximo de encendido de la wifi si no se apaga con el botón
    int estaciones[NUM_ESTACIONES]={0,1,2}; // Tres estaciones, dentro el stationID entre 0 y 7 (-1, no usar esa estación)
    getTimeStamp()
  public:
    GUARDAR();
    void inicializar_SD (); 
    void inicializar_RTC ();
    void INICIALIZAR_GUARDAR ();
    void GUARDAR_CSV (int canal, int velocidad, int direccion, int temperatura, int humedad);
    void GUARDAR_TXT_BACKUP(int canal, int velocidad, int direccion, int temperatura, int humedad);
    void GUARDAR_TODO (int canal, int velocidad, int direccion, int temperatura, int humedad);
    void appendFile(fs::FS &fs, const char * path,const char * message);
    void modificar_fichero(fs::FS &fs, const char * path,const char * message, int var, String valor);
    leer_variables();
    
};
#endif