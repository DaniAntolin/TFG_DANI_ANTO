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

#include <esp_task_wdt.h>

RTC_PCF8523 rtc;


// NOTE: *** One of DAVIS_FREQS_US, DAVIS_FREQS_EU, DAVIS_FREQS_AU, or
// DAVIS_FREQS_NZ MUST be defined at the top of DavisRFM69.h ***

#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define SERIAL_BAUD   115200
#define PACKET_INTERVAL 2563  // Antes estaba a 2555
#define POWER_SAVING
#define DEBUG false
#define NUM_ESTACIONES 3
#define WDT_TIMEOUT 5 // tiempo de Watdog (min). Será WDT_TIMEOUT + Tiempo_escucha de todas estaciones activas

boolean strmon = false;      // Print the packet when received?
boolean strpktinfo = true;   // Print packet info

DavisRFM69 radio;

// Para almacenar los datos leidos
float humedad;  // %
int temperatura;  
float temp_float; // ºC
float temp_float_aux;
int direccion;  // Grados (1..360)
int velocidad;   // millas/h
// Para promediar las lecturas
int N_humedad=0;  
int N_temperatura=0;
int N_direccion=0;
int N_velocidad=0;
boolean DataAll=false; // Para saber si se han leido todos los sensores

int canal;
int tiempo_escucha=90;  //(seg) Tiempo máximo de escucha de cada canal para recibir todos los datos
int tiempo_wifi=5;  //(min) Tiempo máximo de encendido de la wifi si no se apaga con el botón
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
String Nombre_Fichero_Log;

bool opened = false;

int estaciones[NUM_ESTACIONES]={0,1,2}; // Tres estaciones, dentro el stationID entre 0 y 7 (-1, no usar esa estación)


// --- Radio
LoopPacket loopData;

unsigned long lastRxTime = 0;
byte hopCount = 0;
boolean goodCrc = false;

// Added support to filter packets from a specific station and add specific stats
int stationID = 0;
bool correctID;
long idErrors = 0;

int actual_channel = 0;
int16_t goodRssi = -999;
unsigned long tiempo1 = 0;
unsigned long tiempo2 = 0;

// ---- Servidor
boolean ServidorON = false;
const char* ssid = "ESP32Wifi";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

#include "paginaIndexHTML.h"

// CADENAS TEXTO:
#define MAXIMA_LONGITUD_CADENA 30
#define MAX_ARCHIVOS 200

char archivos[MAX_ARCHIVOS][MAXIMA_LONGITUD_CADENA];   // Cadena para ordenar los archivos
char tamanos[MAX_ARCHIVOS][MAXIMA_LONGITUD_CADENA];   // Cadena para guardar los tamaños de los archivos
int longitudARCH;

// --- Interrupciones:
// Pin GPIO donde está conectado el botón
#define PINBOTON 12 
#define PINTIERRA 13

volatile bool encenderWifi = false;
volatile bool apagarWifi = false;
unsigned long tiempoINT = 0;
unsigned long tiempoAUX = 0;
unsigned long tiempoACT = 0;

void IRAM_ATTR miFuncionInterrupcion() {
  tiempoAUX= millis();
  if ((tiempoAUX-tiempoINT)>5000){
   if (ServidorON){
    encenderWifi = false;  // Apagar
    apagarWifi = true;
    tiempoINT=tiempoAUX;
   } 
   else{ 
    encenderWifi = true;  // Encender
    apagarWifi= false;
    tiempoINT=tiempoAUX;
   }  
  }
  
};

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(100);

  //Inizializo SD
  int ok_mount=0;
  int cont_mount=1;
  while ((!ok_mount)&&(cont_mount<5)){ 
    ok_mount=SD.begin(33);
    if(!ok_mount){
      Serial.println("Card Mount Failed");
      delay(2000);
      //return;
    }
    cont_mount++;
  }  
  if (cont_mount>=5){
    Serial.println("Card Mount Failed 5 tried => REBOOT");
    ESP.restart();
  }
  
  delay(100);
  //Inizializo RTC
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!"); 
    rtc.adjust("2020:01:01:01:01:01");
  }
  rtc.start();
  delay(10);
  
  leer_variables();

  getTimeStamp();

  //Interrupciones (botón):
  pinMode(PINTIERRA, OUTPUT);
  digitalWrite(PINTIERRA,LOW);
  pinMode(PINBOTON, INPUT_PULLUP); // Configurar el pin como entrada con resistencia pull-up interna
  attachInterrupt(digitalPinToInterrupt(PINBOTON), miFuncionInterrupcion, RISING); // Configurar la interrupción

  // Inicializar Servidor
  if((anho.toInt()<2022)||(encenderWifi)){
  //if(false){
  wifi_ON(); // Modo arranque wifi con WiFi.h
  conectarServidor();
  ServidorON=true;
  encenderWifi=false;
  apagarWifi=false;
  } //Servidor
  
  
  /// --------------- INICIALIZACIÓN DE LA RADIO
  radio.initialize();
  radio.setChannel(0);              // Frequency / Channel is *not* set in the initialization. Do it right after.
  #ifdef IS_RFM69HW
    radio.setHighPower(); //uncomment only for RFM69HW!
  #endif
  Serial.println(F("Waiting for signal in region defined in DavisRFM69.h"));

  //Inicializo el WDT
  int cont=0;
  for (int i=1; i<NUM_ESTACIONES;i++){ //Ver cuantas estaciones están activas (-1 inactiva)
    if (estaciones[i]>=0){cont++;} 
  }
  esp_task_wdt_init((cont*tiempo_escucha + WDT_TIMEOUT*60), true);
  esp_task_wdt_add(NULL);

} // Fin Setup()

//===========================================================================
void loop() {
      
  if (ServidorON) delay(200); 

  // Imprimir info Tiempo Wifi
  if (ServidorON){
  Serial.print(F("Tiempo wifi (min): "));   Serial.print(tiempo_wifi); 
  Serial.print(F("--  tiempoINT: ")); Serial.print(tiempoINT);
  Serial.print(F("   tiempoACT: ")); Serial.println(tiempoACT);
  }

  comprobarEstadoBoton_WiFi();
 
  // Para cada ESTACION:
  for(int i=0;i<NUM_ESTACIONES;i++){
    stationID=estaciones[i];

    if ((stationID>=0)&&(!ServidorON)){
    tiempo2 = millis();
    tiempo1 = millis();
    humedad=0; N_humedad=0;
    temperatura=0; N_temperatura=0;
    direccion=0; N_direccion=0;
    velocidad=0; N_velocidad=0;
    temp_float=0; 
    DataAll=false;

    Serial.print(F("---------------------- "));
    Serial.print(F("Estacion: "));
    Serial.print(i+1);
    Serial.print(F(" -- ID: "));
    Serial.println(stationID); 
    
    while(((tiempo1-tiempo2)<(tiempo_escucha*1000))&&(!DataAll)&&(!ServidorON)){
      escuchar(); 
      tiempo1=millis();

      process_serial_commands();

      comprobarEstadoBoton_WiFi();
    } // While

    //Serial.print(F("tiempo de escucha: ")); Serial.println(tiempo1-tiempo2);

    print_information();
    
    logSDCard(i);   //Guardar en el archivo Datos
    //logSD_Radio(stationID,(tiempo1-tiempo2)/1000.0);  //Guardar Log de radio
    
    esp_task_wdt_reset(); //Reseteo el WDT
    } // stationID

  } //for

  //Serial.println("Me duermo"); 
  //esp_deep_sleep_start(); 
}
//===========================================================================

// Interruppcion & WiFi

void comprobarEstadoBoton_WiFi(){
  tiempoACT=millis();  //tiempo actual

  if (encenderWifi){
    radio.sleep(); //Evitar interrupcion de radio durante la escritura de SD
    // Imprimir Info 
    if (1) print_variables();
    
    wifi_ON(); 
    conectarServidor();
    ServidorON=true;
    encenderWifi=false;
    apagarWifi=false; //Está ya a false pero por si acaso
   }; 

  if ((ServidorON)&&((tiempoACT-tiempoINT)>(tiempo_wifi*60*1000))) apagarWifi = true;

  if (apagarWifi){
    WiFi.mode(WIFI_OFF);
    Serial.println(F("Wifi Apagada "));
    delay(1000);  
    radio.setChannel(radio.CHANNEL); //Reactivar la radio (se invoca ReceiveBegin())
    ServidorON=false;
    tiempoINT=0;  //Se apaga el servidor (si es interrupcion o si es timeout hay que ponerlo)
    apagarWifi=false;
    encenderWifi=false; //Está ya a false pero por si acaso
   };
}

//--------------------- Radio ------------------
void processPacket() {
  // Every packet has wind speed, direction, and battery status in it
  loopData.windSpeed = radio.DATA[1];
  if (N_velocidad){
    N_velocidad=N_velocidad+1;
    velocidad =(loopData.windSpeed + (N_velocidad-1)*velocidad)/N_velocidad;    
  }
  else
  {
    N_velocidad=N_velocidad+1;
    velocidad = loopData.windSpeed;
  }
  
  #if 0
    Serial.print("Wind Speed: ");
    Serial.print(loopData.windSpeed);
    Serial.print("  Rx Byte 1: ");
    Serial.println(radio.DATA[1]);
  #endif

  // There is a dead zone on the wind vane. No values are reported between 8
  // and 352 degrees inclusive. These values correspond to received byte
  // values of 1 and 255 respectively
  // See http://www.wxforum.net/index.php?topic=21967.50
  loopData.windDirection = 9 + radio.DATA[2] * 342.0f / 255.0f;

  if (N_direccion){
    N_direccion=N_direccion+1;
    direccion =(loopData.windDirection + (N_direccion-1)*direccion)/N_direccion; 
  }
  else
  {
    N_direccion=N_direccion+1;
    direccion = loopData.windDirection;
  }
  
  #if 0
    Serial.print(F("Wind Direction: "));
    Serial.print(loopData.windDirection);
    Serial.print(F("  Rx Byte 2: "));
    Serial.println(radio.DATA[2]);
  #endif

  loopData.transmitterBatteryStatus = (radio.DATA[0] & 0x8) >> 3;
  #if 0
    Serial.print("Battery status: ");
    Serial.println(loopData.transmitterBatteryStatus);
  #endif
  canal = (radio.DATA[0] & 0x7);
  #if 0
  Serial.print("Actual channel: ");
  Serial.println(canal);
  #endif

  // Now look at each individual packet. The high order nibble is the packet type.
  // The highest order bit of the low nibble is set high when the ISS battery is low.
  // The low order three bits of the low nibble are the station ID.

  switch (radio.DATA[0] >> 4) {
  case VP2P_TEMP:
    loopData.outsideTemperature = (int16_t)(word(radio.DATA[3], radio.DATA[4])) >> 4;
    //loopData.outsideTemperature = ((loopData.outsideTemperature-320)*5)/9;
    temperatura = loopData.outsideTemperature;
    temp_float_aux = ((float(temperatura)-320.0)*5.0)/90.0;

    if (N_temperatura){
      N_temperatura=N_temperatura+1;
      temp_float =(temp_float_aux + (N_temperatura-1)*temp_float)/N_temperatura; 
    }
    else
    {
      N_temperatura=N_temperatura+1;
      temp_float = temp_float_aux;
    }
    
  #if 0
      Serial.print(F("Outside Temp: "));
      Serial.print(loopData.outsideTemperature);
      Serial.print(F("  Rx Byte 3: "));
      Serial.print(radio.DATA[3]);
      Serial.print(F("  Rx Byte 4: "));
      Serial.println(radio.DATA[4]);
  #endif
    break;

  case VP2P_HUMIDITY:
    loopData.outsideHumidity = (float)(word((radio.DATA[4] >> 4), radio.DATA[3])) / 10.0;
    
    if (N_humedad){
      N_humedad=N_humedad+1;
      humedad =(loopData.outsideHumidity + (N_humedad-1)*humedad)/N_humedad; 
    }
    else
    {
      N_humedad=N_humedad+1;
      humedad = loopData.outsideHumidity;
    }

  #if 0
      Serial.print("Outside Humdity: ");
      Serial.print(loopData.outsideHumidity);
      Serial.print("  Rx Byte 3: ");
      Serial.print(radio.DATA[3]);
      Serial.print("  Rx Byte 4: ");
      Serial.println(radio.DATA[4]);
  #endif

      break;
    }

  #if 0
    printFreeRam();
  #endif

  // Ver si se han leido todos los sensores:
  DataAll = (N_temperatura>0)&&(N_direccion>0)&&(N_velocidad)&&(N_humedad>0);

}

void escuchar(){
// The check for a zero CRC value indicates a bigger problem that will need
  // fixing, but it needs to stay until the fix is in.
  if (radio.receiveDone()) {
    packetStats.packetsReceived++;
    unsigned int crc = radio.crc16_ccitt(radio.DATA, 6);
    if ((crc == (word(radio.DATA[6], radio.DATA[7]))) && (crc != 0)) {
      // This is a good packet
      goodCrc = true;
      packetStats.receivedStreak++;
      correctID = (stationID == int (radio.DATA[0] & 0x7));
      if (correctID){ 
        hopCount = 1; // From correct station
        processPacket();  //JALO: Proceso el paquete si es de la estación correcta y CRC correcto
      }  
      else{
        idErrors++; // From wrong station
      }
    } else {
      goodCrc = false; // Incorrect packet
      packetStats.crcErrors++;
      packetStats.receivedStreak = 0;
    }

    // STRMON debugging (RAW packet)
    if (strmon) printStrm();

    // Print packet info
    if (strpktinfo) print_debug_packet_info();

    // Radio hop only for correct packects (i.e. good CRC and correct station)
    if (goodCrc && correctID) {
      // Hop radio & update channel and lastRxTime
      lastRxTime = millis();  
      radio.hop();
#ifdef POWER_SAVING
// No haría falta mirar ServidorON porque sólo se activa al salir de aquí 
// y ya no se entra más si se activa, pero por si acaso.
    dormir(!ServidorON);
#endif
    } else {
      // Do not hop the radio for incorrect packets but activate reception. 
      // Problem: receiveBegin() is a private function of the radio.classs 
      // Workaround: use setChannel to indirectly invoke it (could be improved but it works)
      radio.setChannel(radio.CHANNEL); 
    }
  }

  // If a packet was not received at the expected time, hop the radio anyway
  // in an attempt to keep up.  Give up after 4 failed attempts.  Keep track
  // of packet stats as we go.  We consider a consecutive string of missed
  // packets to be a single resync. 
  if ((hopCount > 0) && ((millis() - lastRxTime) > (hopCount * (PACKET_INTERVAL + (1000*stationID/16)) + 50))) {
    packetStats.packetsMissed++;
    if (hopCount == 1) packetStats.numResyncs++;
    if (++hopCount > 4) hopCount = 0;
    radio.hop();
#ifdef POWER_SAVING
    dormir(!ServidorON);
#endif
  }
}

void dormir(boolean on){
Serial.print(F("Dormir: "));
Serial.println(on);
Serial.flush();
if (on){
  esp_sleep_enable_timer_wakeup(2 * 1000000); // light sleep (2s) to save energy
  esp_light_sleep_start();
}
}
// -----------------------------------------------------------------
void logSD_Radio(int id, float t) {
  getTimeStamp();
  Nombre_Fichero_Log = String("/LogRadio") + "_"+String(dia)+"_"+String(mes)+"_"+String(anho)+".csv";
  unsigned long min = millis()/60000;
  unsigned long correct_packets = packetStats.packetsReceived - packetStats.crcErrors - idErrors;
  float rate = 0.0;
  if (t>0) rate = correct_packets/(t*60);  // correct_packets/min;
  
  dataMessage = String(dia) + "-" +String(mes) + "-" + String(anho) + ";" + String(hora) + ":" +String(minutos) + ":" + String(segundos) + ";" + String(id) + ";" + String(packetStats.packetsReceived) + ";" + String(packetStats.packetsMissed) + ";" + String(packetStats.numResyncs)+ ";"+ String(packetStats.receivedStreak) + ";"+ String(packetStats.crcErrors) + ";"+ String(idErrors) + ";"+ String(rate) +"\n";
  Serial.print("Save log Radio: ");
  Serial.println(dataMessage);
  radio.sleep(); //Evitar interrupcion de radio durante la escritura de SD
  appendFile(SD, Nombre_Fichero.c_str(), dataMessage.c_str());
  radio.setChannel(radio.CHANNEL); //Reactivar la radio (se invoca ReceiveBegin())
}

void logSDCard(int i) {
  getTimeStamp();
  i++;
  String CD,CM;
  if (dia.toInt()<10){ CD="0"+String(dia);};
  if (mes.toInt()<10){ CM="0"+String(mes);};  
  Nombre_Fichero = "/Estacion"+String(i)+"_"+CD+"_"+CM+"_"+String(anho)+".csv";
  //Nombre_Fichero = "/Estacion"+String(i)+"_"+String(dia)+"_"+String(mes)+"_"+String(anho)+".csv";
  // Cambiar el orden para que se ordene mejor:
  //Nombre_Fichero = "/Estacion"+String(i)+"_"+String(anho)+"_"+String(mes)+"_"+String(dia)+".csv";
  //dataMessage = String(dia) + "-" +String(mes) + "-" + String(anho) + ";" + String(hora) + ":" +String(minutos) + ":" + String(segundos) + ";" + String(temp_float) + ";" + String(humedad) + ";" + String(velocidad)+ ";"+ String(direccion) +"\n";
  dataMessage = CD + "-" + CM + "-" + String(anho) + ";" + String(hora) + ":" +String(minutos) + ":" + String(segundos) + ";" + String(temp_float) + ";" + String(humedad) + ";" + String(velocidad)+ ";"+ String(direccion) +"\n";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  radio.sleep(); //Evitar interrupcion de radio durante la escritura de SD
  appendFile(SD, Nombre_Fichero.c_str(), dataMessage.c_str());
  radio.setChannel(radio.CHANNEL); //Reactivar la radio (se invoca ReceiveBegin())
}

void appendFile(fs::FS &fs, const char * path,const char * message) {
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

void getTimeStamp() {
  DateTime now = rtc.now();
  hora=now.hour();
  minutos= now.minute();
  segundos=now.second();
  anho=now.year();
  mes=now.month();
  dia=now.day();
}

void modificar_fichero(fs::FS &fs, const char * path,const char * message, int var, String valor) {
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

void leer_variables(){
  int error=0;
  error=readFile(SD, "/variables.txt");

  if (error){
    Serial.println("Failed to open file for reading /variables.txt");
    Serial.println("Creating file /variables.txt with actual variables");
    String mensaje = String(tiempo_escucha)+"\n"+String(tiempo_wifi)+"\n"+String(estaciones[0])+"\n"+String(estaciones[1])+"\n"+String(estaciones[2]);
    modificar_fichero(SD, "/variables.txt",mensaje.c_str(), 0, String(""));
  }

}

int readFile(fs::FS &fs, const char * path){
  int i=1;

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return 1;
    }
    while(file.available()){
      String linea = file.readStringUntil('\n');
      if(i==1)tiempo_escucha=linea.toInt();
      else if(i==2) tiempo_wifi=linea.toInt();
      else if(i==3) estaciones[0]=linea.toInt();
      else if(i==4) estaciones[1]=linea.toInt();
      else if(i==5) estaciones[2]=linea.toInt();
      i++;
    }
    file.close();
    return 0;
}

// ----------------------------------------------------------------

void process_serial_commands() {
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    if (input == '?') 
    {
      Serial.println(F("Help:"));
      Serial.println(F("- r: read all RFM69HW registers"));
      Serial.println(F("- t: show radio temp"));
      Serial.println(F("- s: show packet stats"));
      Serial.println(F("- h: radio channel hop"));
      Serial.println(F("- R: RFM69HW reset"));
      Serial.println(F("- m: show elapsed time in milliseconds"));
      Serial.println(F("- c: show current radio channel"));
      Serial.println(F("- i: set the current station ID; ej: i 1"));
      Serial.println(F("- I: show the current station ID"));
      Serial.println(F("- d: show actual data of the current station"));
    }
    if (input == 'r') //r=dump all register values
    {
      radio.readAllRegs();
      Serial.println();
    }
    if (input == 't') // read radio temp
    {
      byte temperature =  radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
      byte fTemp = 9 * temperature / 5 + 32; // 9/5=1.8
      Serial.print(F("Radio Temp is "));
      Serial.print(temperature);
      Serial.print(F("C, "));
      Serial.print(fTemp); //converting to F loses some resolution, obvious when C is on edge between 2 values (ie 26C=78F, 27C=80F)
      Serial.println(F("F"));
    }
    if (input == 's') // show packet stats
    {
      Serial.print(F("packetsReceived: "));
      Serial.print(packetStats.packetsReceived);
      Serial.print(F(" packetsMissed: "));
      Serial.print(packetStats.packetsMissed);
      Serial.print(F(" numResyncs: "));
      Serial.print(packetStats.numResyncs);
      Serial.print(F(" receivedStreak: "));
      Serial.print(packetStats.receivedStreak);
      Serial.print(F(" crcErrors: "));
      Serial.print(packetStats.crcErrors); 
      Serial.print(F(" idErrors: "));
      Serial.print(idErrors);
      Serial.print(F(" packets/min: "));
      unsigned long min = millis()/60000;
      unsigned long correct_packets = packetStats.packetsReceived - packetStats.crcErrors - idErrors;
      float rate = 0.0;
      if (min>0) rate = correct_packets/min;
      Serial.println(rate);
    }
    if (input == 'h') // manually hop radio channel
    {
      radio.hop();
    }
    if (input == 'R') // reset radio
    {
       //radio.reset();
    }
    if (input == 'm') // show time in milliseconds
    {
        Serial.println(millis());
    }
    if (input == 'c') // show current radio channel
    {
        Serial.println(radio.CHANNEL);
    }
    if (input == 'i') // set the current station ID, ej: i 1
    {
        stationID = Serial.parseInt();
    }
    if (input == 'I') // show the current station ID
    {
        Serial.println(stationID);
    }
    if (input == 'd') // show the current station ID
    {
        print_information();
    }
  }
} // process_serial_commands()

// --------------------- Funciones para imprimir

void print_debug_packet_info () {
    Serial.print(millis());
    Serial.print(F(":  "));
    Serial.print(radio.CHANNEL);
    Serial.print(F(" - Data: "));
    for (byte i = 0; i < DAVIS_PACKET_LEN; i++) {
      Serial.print(radio.DATA[i], HEX);
      Serial.print(F(" "));
    }
    Serial.print(F("  RSSI: "));
    Serial.print(radio.RSSI);
    //int freqError = radio.readReg(0x21) << 8 |radio.readReg(0x22);
    //Serial.print(F("      Freq error): "));
    //Serial.println(freqError);
    Serial.print(F(" CRC: "));
    if (goodCrc)
       Serial.println(F("OK"));
    else
       Serial.println(F("ERROR"));
}

void print_information(){
  getTimeStamp();
  Serial.print(dia);   Serial.print("/");  
  Serial.print(mes);   Serial.print("/"); 
  Serial.print(anho);   
  Serial.print("     Hora: ");  
  Serial.print(hora);      Serial.print(":");
  Serial.print(minutos);   Serial.print(":");
  Serial.println(segundos);
  Serial.print(F("Estacion ID: "));   Serial.println(stationID);
  Serial.print(F("Temperatura: "));   Serial.print(temp_float);
  Serial.print(F(" ºC"));
  Serial.print(F(" -- Humedad: "));   Serial.println(humedad);
  Serial.print(F("Dirección del viento: "));   Serial.print(direccion);
  Serial.print(F(" -- Velocidad: "));   Serial.println(velocidad);
  if (DataAll) Serial.println(F("Todos los datos leidos")); 
  else Serial.println(F("NO todos los datos leidos"));
}

void printStrm() {
  for (byte i = 0; i < DAVIS_PACKET_LEN; i++) {
    Serial.print(i);
    Serial.print(F(" = "));
    Serial.print(radio.DATA[i], HEX);
    Serial.print(F("\n\r"));
  }
  Serial.print(F("\n\r"));
}
void print_variables(){
  Serial.println("");
  Serial.print("Tiempo escucha: ");
  Serial.println(tiempo_escucha);
  Serial.print("Tiempo wifi: ");
  Serial.println(tiempo_wifi);
  Serial.print("Estacion1 (ID): ");
  Serial.print(estaciones[0]);
  Serial.print(" -- Estacion2 (ID): ");
  Serial.print(estaciones[1]);
  Serial.print(" -- Estacion3 (ID): ");
  Serial.println(estaciones[2]);
}


// ---------------- WIFI ----------------------
void SD_dir(AsyncResponseStream *response,AsyncWebServerRequest *request){

// Leer los archivos y ordenarlos:
  longitudARCH=leerDirectory("/",archivos,tamanos);
  burbuja2(archivos,tamanos,longitudARCH);
  if (longitudARCH>=0) {
    response->print("<table align='center'>");
    response->print("<tr><th>Nombre</th><th style='width:20%'>Tipo File/Dir</th><th>Tamano</th><th>Descargar</th><th>Borrar</th></tr>");    
    printDirectory(archivos,tamanos,longitudARCH,response);
    response->print("</table>");
    response->print("<a href='/'><button>Variables</button></a>");
  }else{
    response->print("<h3>Failed to open directory or not files</h3>"); 
  }
}

void printDirectory(char arregloA[][MAXIMA_LONGITUD_CADENA],char arregloT[][MAXIMA_LONGITUD_CADENA], int longitud, AsyncResponseStream *response){
  char arch[MAXIMA_LONGITUD_CADENA];
  char tam[MAXIMA_LONGITUD_CADENA];
  
  for (int i = 0; i < longitud; i++)
    {
      strcpy(arch,arregloA[i]);
      strcpy(tam,arregloT[i]);      
      response->print("<tr><td>"); response->print(arch); response->print("</td>");
      response->print("<td> File </td>");
      response->print("<td>"); response->print(tam); response->print("</td>");
      response->print("<td>");
      response->print("<FORM action='/download' method='get'>"); 
      response->print("<button type='submit' name='download'"); 
      response->print("' value='"); response->print(arch); 
      response->print("'>Descargar</button>");
      response->print("</FORM> </td>");
      response->print("<td>");
      response->print("<FORM action='/delete' method='get'>"); 
      response->print("<button type='submit' name='delete'"); 
      response->print("' value='"); response->print(arch); 
      response->print("'>Eliminar</button>");
      response->print("</FORM> </td>");
      response->print("</tr>");
    }
}

void burbuja2(char arreglo[][MAXIMA_LONGITUD_CADENA], char arregloaux[][MAXIMA_LONGITUD_CADENA],int longitud)
{
    // Útil para hacer intercambio de cadenas
    char temporal[MAXIMA_LONGITUD_CADENA];
    char temporalaux[MAXIMA_LONGITUD_CADENA];
    int x, indiceActual;
    for (x = 0; x < longitud; x++)
    {
        for (indiceActual = 0; indiceActual < longitud - 1;
             indiceActual++)
        {
            int indiceSiguienteElemento = indiceActual + 1;
            // Si la cadena es mayor que la siguiente (alfabeticamente) entonces intercambiamos
            if (strcmp(arreglo[indiceActual], arreglo[indiceSiguienteElemento]) > 0)
            {
                // Movemos la cadena actual a la temporal
                memcpy(temporal, arreglo[indiceActual], MAXIMA_LONGITUD_CADENA);
                memcpy(temporalaux, arregloaux[indiceActual], MAXIMA_LONGITUD_CADENA);
                // Movemos al actual el siguiente elemento
                memcpy(arreglo[indiceActual], arreglo[indiceSiguienteElemento], MAXIMA_LONGITUD_CADENA);
                memcpy(arregloaux[indiceActual], arregloaux[indiceSiguienteElemento], MAXIMA_LONGITUD_CADENA);
                // Y en el siguiente elemento, lo que había antes en el actual pero ahora está en temporal
                memcpy(arreglo[indiceSiguienteElemento], temporal, MAXIMA_LONGITUD_CADENA);
                memcpy(arregloaux[indiceSiguienteElemento], temporalaux, MAXIMA_LONGITUD_CADENA);
            }
        }
    }
    // No hay necesidad de devolver nada, pues modificamos al arreglo de manera interna
}

int leerDirectory(const char * dirname, char arregloA[][MAXIMA_LONGITUD_CADENA],char arregloT[][MAXIMA_LONGITUD_CADENA]){
  File root = SD.open(dirname);
  if(!root){       
    return -1;
  }
  if(!root.isDirectory()){
    return -1;
  }
  root.rewindDirectory();
  File file = root.openNextFile();
  int i = 0, longitud=0;
  char temporal[MAXIMA_LONGITUD_CADENA];
  while(file){  
    if(file.isDirectory()){
      // No lo copio en la lista ni incremento el contador del array
    }
    else
    {
      int bytes = file.size();
      String fsize = "";
      if (bytes < 1024)                     fsize = String(bytes)+" B";
      else if(bytes < (1024 * 1024))        fsize = String(bytes/1024.0,3)+" KB";
      else if(bytes < (1024 * 1024 * 1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
      else                                  fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
      
      // Separado en dos arrays:
      temporal[0]='\0';   // Borramos la cadena anterior
      strcat(temporal, file.name());
      memcpy(arregloA[i], temporal, MAXIMA_LONGITUD_CADENA);
      temporal[0]='\0';   // Borramos la cadena anterior
      strcat(temporal, fsize.c_str());
      memcpy(arregloT[i], temporal, MAXIMA_LONGITUD_CADENA);
                
      i++;
      longitud=i;

    }
    file = root.openNextFile();
  }
    file.close(); 
    root.close();
    return longitud;
}
/* ------------------------------------
// VERSION SIN ORDENAR LOS ARCHIVOS 
void SD_dir(AsyncResponseStream *response,AsyncWebServerRequest *request){

  File root = SD.open("/");
  if (root) {
    response->print("<table align='center'>");
    response->print("<tr><th>Nombre</th><th style='width:20%'>Tipo File/Dir</th><th>Tamano</th></tr>");
    printDirectory("/",0,response);
    //printDirectory(root,0,response);
    response->print("</table>");
    response->print("<a href='/'><button>Variables</button></a>");
    root.close();
  }else{
    response->print("<h3>Failed to open directory</h3>"); 
  }
}


void printDirectory(const char * dirname, uint8_t levels,AsyncResponseStream *response){
//void printDirectory(File root, uint8_t levels,AsyncResponseStream *response){
  File root = SD.open(dirname);
  if(!root){
    return;
  }
  if(!root.isDirectory()){
    return;
  }
  root.rewindDirectory();
  File file = root.openNextFile();
  int i = 0;
  while(file){  
    if(file.isDirectory()){
      response->print("<tr><td>"+String(file.isDirectory()?"Dir":"File")+"</td><td>"+String(file.name())+"</td><td></td></tr>");
      printDirectory(file.name(), levels-1,response);
      //printDirectory(file, levels-1,response);
    }
    else
    {
      response->print("<tr><td>"+String(file.name())+ "</td>");
      response->print("<td>"+String(file.isDirectory()?"Dir":"File")+"</td>");
      int bytes = file.size();
      String fsize = "";
      if (bytes < 1024)                     fsize = String(bytes)+" B";
      else if(bytes < (1024 * 1024))        fsize = String(bytes/1024.0,3)+" KB";
      else if(bytes < (1024 * 1024 * 1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
      else                                  fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
      response->print("<td>"+fsize+"</td>");
      response->print("<td>");
      response->print("<FORM action='/download' method='get'>"); 
      response->print("<button type='submit' name='download'"); 
      response->print("' value='"); 
      response->print(String(file.name())); 
      response->print("'>Descargar</button>");
      response->print("</FORM> </td>");
      response->print("<td>");
      response->print("<FORM action='/delete' method='get'>"); 
      response->print("<button type='submit' name='delete'"); 
      response->print("' value='"); 
      response->print(String(file.name())); 
      response->print("'>Eliminar</button>");
      response->print("</FORM> </td>");
      response->print("</tr>");

    }
    file = root.openNextFile();
    i++;
  }

  file.close(); 
}

String file_size(int bytes){
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes)+" B";
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,3)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
  else                              fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
  return fsize;
}

----------------------------------- */

void SD_file_delete(String filename, AsyncResponseStream *response) { // Delete the file 
  File dataFile = SD.open("/"+filename, FILE_READ); // Now read data from SD Card 
  if (dataFile){
    if (SD.remove("/"+filename)) {
      response->print("alert('Archivo eliminado')");
    }else{ 
      response->print("alert('Archivo NO eliminado')");
    }
  } 
}

String hora_actual() {
  getTimeStamp();
  String hora_actual = String(dia) + "/" +String(mes) + "/" + String(anho)+"   "+String(hora) + ":" +String(minutos) + ":" + String(segundos);
  return String(hora_actual);
}

String processor(const String& var){
  //Serial.println(var);
  if(var == "HORA"){
    return hora_actual();
  }else if(var == "ESCUCHA"){
    return String(tiempo_escucha);
  }else if(var == "WIFI"){
    return String(tiempo_escucha);
  }else if(var == "ESTACION1"){
    return String(estaciones[0]);
  }else if(var == "ESTACION2"){
    return String(estaciones[1]);
  }else if(var == "ESTACION3"){
    return String(estaciones[2]);
  }
  return String();
}
String escucha(){
   return String(tiempo_escucha);
}
String encwifi(){
   return String(tiempo_wifi);
}
String estacion1(){
   return String(estaciones[0]);
}
String estacion2(){
   return String(estaciones[1]);
}
String estacion3(){
   return String(estaciones[2]);
}

void WiFiEvent(WiFiEvent_t event){
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY: 
            Serial.println("WiFi interface ready");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            Serial.println("Completed scan for access points");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("WiFi client started");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            Serial.println("WiFi clients stopped");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("Connected to access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("Disconnected from WiFi access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            Serial.println("Authentication mode of access point has changed");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("Obtained IP address: ");
            Serial.println(WiFi.localIP());
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            Serial.println("Lost IP address and IP address is reset to 0");
            break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
            Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_PIN:
            Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            Serial.println("WiFi access point started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            Serial.println("WiFi access point  stopped");
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            Serial.println("Client connected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            Serial.println("Client disconnected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            Serial.println("Assigned IP address to client");
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            Serial.println("Received probe request");
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            Serial.println("AP IPv6 is preferred");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            Serial.println("STA IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP6:
            Serial.println("Ethernet IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_START:
            Serial.println("Ethernet started");
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("Ethernet stopped");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("Ethernet connected");
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("Ethernet disconnected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.println("Obtained IP address");
            break;
        default: break;
    }}
    void conectarServidor(){
   // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    SD_dir(response,request);
    request->send(response);
  });
  server.on("/hora", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", hora_actual().c_str());
  });
  server.on("/escucha", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", escucha().c_str());
  });
  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", encwifi().c_str());
  });
  server.on("/estacion1", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", estacion1().c_str());
  });
  server.on("/estacion2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", estacion2().c_str());
  });
  server.on("/estacion3", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", estacion3().c_str());
  });
  server.on("/actualizar", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("hour"))&&(request->hasParam("minutes"))&&(request->hasParam("seconds"))&&(request->hasParam("year"))&&(request->hasParam("mounth"))&&(request->hasParam("day"))){
      String hora_actualizar=request->getParam("hour")-> value();
      String minutos_actualizar=request->getParam("minutes")-> value();
      String segundos_actualizar=request->getParam("seconds")-> value();
      String anho_actualizar=request->getParam("year")-> value();
      String mes_actualizar=request->getParam("mounth")-> value();
      String dia_actualizar=request->getParam("day")-> value();
      fecha=String(anho_actualizar)+":"+String(mes_actualizar)+":"+String(dia_actualizar)+":"+String(hora_actualizar)+":"+String(minutos_actualizar)+":"+String(segundos_actualizar);
      rtc.adjust(fecha.c_str());
    }
    request->send(200, "text/html", index_html);
  }); 
  server.on("/tiempo_escucha", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("tiempo_escucha"))){
      String t_escucha_actualizar=request->getParam("tiempo_escucha")-> value();
      String mensaje = String(t_escucha_actualizar)+"\n"+String(tiempo_wifi)+"\n"+String(estaciones[0])+"\n"+String(estaciones[1])+"\n"+String(estaciones[2]);
      modificar_fichero(SD, "/variables.txt",mensaje.c_str(),1,t_escucha_actualizar);
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/tiempo_wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("tiempo_wifi"))){      
      String t_wifi_actualizar=request->getParam("tiempo_wifi")-> value();
      String mensaje = String(tiempo_escucha)+"\n"+String(t_wifi_actualizar)+"\n"+String(estaciones[0])+"\n"+String(estaciones[1])+"\n"+String(estaciones[2]);
      modificar_fichero(SD, "/variables.txt",mensaje.c_str(),2,t_wifi_actualizar);
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/canal1", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("canal1"))){
      String canal1_actualizar=request->getParam("canal1")-> value();
      String mensaje = String(tiempo_escucha)+"\n"+String(tiempo_wifi)+"\n"+String(canal1_actualizar)+"\n"+String(estaciones[1])+"\n"+String(estaciones[2]);
      modificar_fichero(SD, "/variables.txt",mensaje.c_str(),3,canal1_actualizar);
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/canal2", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("canal2"))){
      String canal2_actualizar=request->getParam("canal2")-> value();
      String mensaje = String(tiempo_escucha)+"\n"+String(tiempo_wifi)+"\n"+String(estaciones[0])+"\n"+String(canal2_actualizar)+"\n"+String(estaciones[2]);
      modificar_fichero(SD, "/variables.txt",mensaje.c_str(),4,canal2_actualizar);
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/canal3", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("canal3"))){
      String canal3_actualizar=request->getParam("canal3")-> value();
      String mensaje = String(tiempo_escucha)+"\n"+String(tiempo_wifi)+"\n"+String(estaciones[0])+"\n"+String(estaciones[1])+"\n"+String(canal3_actualizar);
      modificar_fichero(SD, "/variables.txt",mensaje.c_str(),5,canal3_actualizar);
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
    File descarga = SD.open("/"+request->arg("download"));
    Serial.print(request->arg("download"));
    request->send(descarga, request->arg("download"), "text/html", true);
  });
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request){
    File eliminar = SD.open("/"+request->arg("download"));
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    SD_file_delete(request->arg("delete"), response);
    request->redirect("/files");
  });
  
  // Start server
  server.begin();
  Serial.println(F("Servidor iniciado"));
  delay(100);
}
void wifi_ON(){
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  WiFi.onEvent(WiFiEvent);
  delay(100);

}