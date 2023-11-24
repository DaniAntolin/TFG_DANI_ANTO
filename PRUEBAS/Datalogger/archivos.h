/*
  Morse.h - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/
#ifndef archivos_h
#define archivos_h


#include <DavisRFM69.h>
#include <Davisdef.h>
#include <RFM69registers.h>
#include <SPI.h>
#include <Time.h>
#include <TimeLib.h>

#include "WiFi.h"
#include "ESPAsyncWebServer.h"

#include "FS.h"
#include "SD.h"

#include "RTClib.h"


#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define SERIAL_BAUD   115200
#define PACKET_INTERVAL 2555
#define DEBUG false

class archivos{
  private:
    DavisRFM69 radio;
    LoopPacket loopData;
    boolean strmon = true;      // Print the packet when received?
    int humedad;
    int temperatura;
    float temp_float;
    int direccion;
    int velocidad;
    int canal;
    int tiempo_escucha=90;
    int tiempo_dormido=60;
    int actual_channel = 0;
    int estaciones[3]={0,1,2};
    int16_t goodRssi = -999;
    String dataMessage;
    String nombreFichero;
    String hora;
    String minutos;
    String segundos;
    String anho;
    String mes;
    String dia;
    String fecha;
    String Nombre_Fichero;
    bool correctID;
    bool opened = false;
    bool goodCrc = false;
    const char* ssid = "ESP32Wifi";
    // Create AsyncWebServer object on port 80
    AsyncWebServer server(80);
    byte hopCount = 0;
    long idErrors = 0;
    unsigned long lastRxTime = 0;
    unsigned long tiempo1 = 0;
    unsigned long tiempo2 = 0;

  
  public:
    datalogger(); //
    datalogger(uint16_t port);//
    void printStrm();
    void processPacket();
    void print_information(int estacion);
};
#endif
