#ifndef ENVIARV1_h
#define ENVIARV1_h

#include <InfluxDbClient.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <NTPClient.h>
#include "ESPAsyncWebServer.h"
#include "paginaIndexHTML.h"
#include "GUARDAR.h" 


class RECIBIR{
  private:
  GUARDAR GUARDAR;
  // ---- Servidor
    bool ServidorON = false;
    const char* ssid = "ESP32Wifi";

    // Create AsyncWebServer object on port 80
    AsyncWebServer server(80);

    // --- Interrupciones:
    // Pin GPIO donde está conectado el botón
    #define PINBOTON 12 
    #define PINTIERRA 13

    volatile bool encenderWifi = false;
    unsigned long tiempoINT = 0;
    unsigned long tiempoAUX = 0;
    unsigned long tiempoACT = 0;
  public:
    ENVIARV1(); 
    void SD_dir(AsyncResponseStream *response,AsyncWebServerRequest *request);
    void printDirectory(const char * dirname, uint8_t levels,AsyncResponseStream *response);
    String file_size(int bytes);
    void SD_file_delete(String filename, AsyncResponseStream *response);
    String hora_actual();
    String processor(const String& var);
    String escucha();
    String encwifi();
    String estacion1();
    String estacion2();
    String estacion3();
    void WiFiEvent(WiFiEvent_t event);
    void conectarServidor();
    void wifi_ON();
    void comprobarEstadoBoton_WiFi();
    void Init_Servidor ();
    void Init_BOTON ();
};
#endif