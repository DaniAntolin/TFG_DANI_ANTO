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

RTC_PCF8523 rtc;


// NOTE: *** One of DAVIS_FREQS_US, DAVIS_FREQS_EU, DAVIS_FREQS_AU, or
// DAVIS_FREQS_NZ MUST be defined at the top of DavisRFM69.h ***

#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define SERIAL_BAUD   115200
#define PACKET_INTERVAL 2555
#define DEBUG false
boolean strmon = true;      // Print the packet when received?


int humedad;
int temperatura;
float temp_float;
int direccion;
int velocidad;
int canal;
int tiempo_escucha=90;
int tiempo_dormido=60;
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

bool opened = false;

DavisRFM69 radio;
LoopPacket loopData;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>

<body>
  <h2>Servidor WEB</h2>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Fecha:</span>
    <span id="hora">HORA</span>
  </p>
  <form action="/actualizar" method="get" id="formReloj">
    <input type="text" name="year" placeholder="Ano (AAAA)" id="ano"><br>
    <input type="text" name="mounth" placeholder="Mes (MM)" id="mes"><br>
    <input type="text" name="day" placeholder="Dia (DD)" id="dia"><br>
    <input type="text" name="hour" placeholder="Hora (HH)" id="hora"><br>
    <input type="text" name="minutes" placeholder="Minutos (MM)" id="min"><br>
    <input type="text" name="seconds" placeholder="Segundos (SS)" id="seg"><br>
    <a href=\"/on\"><button type='submit'>Actualizar</button></a>
  </form>
      </p>
      <br>
  <span class="dht-labels">Tiempo escucha:</span>
  <span id="tiempoescucha">ESCUCHA</span>
  <form action="/tiempo_escucha" id="t_escucha">
    <input type="text" name="tiempo_escucha" placeholder="Tiempo escucha (segundos)" id="escucha">
    <a href=\"/on\"><button type='submit'>Guardar</button></a>
  </form>

  <span class="dht-labels">Tiempo dormido:</span>
  <span id="tiempodormido">DORMIDO</span>
  <form action="/tiempo_dormido" id="t_dormido">
    <input type="text" name="tiempo_dormido" placeholder="Tiempo dormido (segundos)" id="dormido">
    <a href=\"/on\"><button type='submit'>Guardar</button></a>
  </form>

  <span class="dht-labels">Canal 1:</span>
  <span id="estacion1">ESTACION1</span>
  <form action="/canal1" id="estacion_1">
    <input type="text" name="canal1" placeholder="Canal 1" id="canal_1">
    <a href=\"/on\"><button type='submit'>Guardar</button></a>
  </form>

  <span class="dht-labels">Canal 2:</span>
  <span id="estacion2">ESTACION2</span>
  <form action="/canal2" id="estacion_2">
    <input type="text" name="canal2" placeholder="Canal 2" id="canal_2">
    <a href=\"/on\"><button type='submit'>Guardar</button></a>
  </form>

  <span class="dht-labels">Canal 3:</span>
  <span id="estacion3">ESTACION3</span>
  <form action="/canal3" id="estacion_3">
    <input type="text" name="canal3" placeholder="Canal 3" id="canal_3">
    <a href=\"/on\"><button type='submit'>Guardar</button></a>
  </form>
  <br>
  <br>
  <a href='/files'><button>Archivos</button></a>
</body>

<script type="text/javascript">
   document.addEventListener("DOMContentLoaded", function() {
  document.getElementById("formReloj").addEventListener('submit', validarFormReloj); 
});
   document.addEventListener("DOMContentLoaded", function() {
  document.getElementById("t_dormido").addEventListener('submit', validarDormido); 
});
   document.addEventListener("DOMContentLoaded", function() {
  document.getElementById("t_escucha").addEventListener('submit', validarEscucha); 
});
   document.addEventListener("DOMContentLoaded", function() {
  document.getElementById("estacion_1").addEventListener('submit', validarCanalA); 
});
   document.addEventListener("DOMContentLoaded", function() {
  document.getElementById("estacion_2").addEventListener('submit', validarCanalB); 
});
   document.addEventListener("DOMContentLoaded", function() {
  document.getElementById("estacion_3").addEventListener('submit', validarCanalC); 
});

function validarFormReloj(evento) {
  evento.preventDefault();
  var ano = document.getElementById('ano').value;
  var mes = document.getElementById('mes').value;
  var dia = document.getElementById('dia').value;
  var hora = document.getElementById('hora').value;
  var min = document.getElementById('min').value;
  var seg = document.getElementById('seg').value;
  if ((ano=="")||(mes=="")||(dia=="")||(hora=="")||(min=="")||(seg=="")) {
    alert('Hay un campo vacio');
    return false;
  };
  this.submit();
};
function validarDormido(evento) {
  evento.preventDefault();
  var tdormido = document.getElementById('dormido').value;
  if (tdormido==""){
    alert('El campo esta vacio');
    return false;
  };
  if ((tdormido<5)||(tdormido>15)) {
    alert('El tiempo dormido debe estar entre 5 min y 15 min');
    return false;
  };
  this.submit();
};
function validarEscucha(evento) {
  evento.preventDefault();
  var tescucha = document.getElementById('escucha').value;
  if (tescucha==""){
    alert('El campo esta vacio');
    return false;
  };
  if ((tescucha<60)) {
    alert('El tiempo de escucha debe de ser mayor a 60 segundos');
    return false;
  };
  this.submit();
};
function validarCanalA(evento) {
  evento.preventDefault();
  var can1 = document.getElementById('canal_1').value;
  var can2 = document.getElementById("estacion2").innerHTML;
  var can3 = document.getElementById("estacion3").innerHTML;
  if (can1==""){
    alert('El campo esta vacio');
    return false;
  };
  if ((can1<0)||(can1>7)){
    alert('El canal debe estar entre 0 y 7');
    return false;
  };
  if ((can1==can2)){
    alert('Este canal es igual al canal 2');
    return false;
  };
  if ((can1==can3)){
    alert('Este canal es igual al canal 3');
    return false;
  };
  this.submit();
};
function validarCanalB(evento) {
  evento.preventDefault();
  var can2 = document.getElementById('canal_2').value;
  var can1 = document.getElementById("estacion1").innerHTML;
  var can3 = document.getElementById("estacion3").innerHTML;
  if (can2==""){
    alert('El campo esta vacio');
    return false;
  };
  if ((can2<0)||(canal2>7)) {
    alert('El canal debe estar entre 0 y 7');
    return false;
  };
  if ((can1==can2)){
    alert('Este canal es igual al canal 1');
    return false;
  };
  if ((can2==can3)){
    alert('Este canal es igual al canal 3');
    return false;
  };
  this.submit();
};
function validarCanalC(evento) {
  evento.preventDefault();
  var can3 = document.getElementById('canal_3').value;
  var can2 = document.getElementById("estacion2").innerHTML;
  var can1 = document.getElementById("estacion1").innerHTML;
  if (can3==""){
    alert('El campo esta vacio');
    return false;
  };
  if ((can3<0)||(can3>7)) {
    alert('El canal debe estar entre 0 y 7');
    return false;
  };
  if ((can3==can2)){
    alert('Este canal es igual al canal 2');
    return false;
  };
  if ((can1==can3)){
    alert('Este canal es igual al canal 1');
    return false;
  };
  this.submit();
};
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tiempoescucha").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/escucha", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tiempodormido").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/dormido", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("estacion1").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/estacion1", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("estacion2").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/estacion2", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("estacion3").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/estacion3", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("hora").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/hora", true);
  xhttp.send();
}, 1000 ) ;

</script>
</html>)rawliteral";

const char* ssid = "ESP32Wifi";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


int estaciones[3]={0,1,2};

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);

  //Inizializo SD
  if(!SD.begin(33)){
      Serial.println("Card Mount Failed");
      return;
  }
  delay(10);
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
  
  getTimeStamp();
  //if((minutos.toInt()>0)&&(minutos.toInt()<60)){
  if (1){   // JALO: AHORA SERVIDOR SIEMPRE INICIADO (para ver su funcionamiento)
    WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

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
  server.on("/dormido", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", dormido().c_str());
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
      cambiar_variables();
      String t_escucha_actualizar=request->getParam("tiempo_escucha")-> value();
      String mensaje = String(t_escucha_actualizar)+"\n"+String(tiempo_dormido)+"\n"+String(estaciones[0])+"\n"+String(estaciones[1])+"\n"+String(estaciones[2]);
      modificar_fichero(SD, "/variables.txt",mensaje.c_str());
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/tiempo_dormido", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("tiempo_dormido"))){
      cambiar_variables();
      String t_dormido_actualizar=request->getParam("tiempo_dormido")-> value();
      String mensaje = String(tiempo_escucha)+"\n"+String(t_dormido_actualizar)+"\n"+String(estaciones[0])+"\n"+String(estaciones[1])+"\n"+String(estaciones[2]);
      modificar_fichero(SD, "/variables.txt",mensaje.c_str());
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/canal1", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("canal1"))){
      cambiar_variables();
      String canal1_actualizar=request->getParam("canal1")-> value();
      String mensaje = String(tiempo_escucha)+"\n"+String(tiempo_dormido)+"\n"+String(canal1_actualizar)+"\n"+String(estaciones[1])+"\n"+String(estaciones[2]);
      modificar_fichero(SD, "/variables.txt",mensaje.c_str());
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/canal2", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("canal2"))){
      cambiar_variables();
      String canal2_actualizar=request->getParam("canal2")-> value();
      String mensaje = String(tiempo_escucha)+"\n"+String(tiempo_dormido)+"\n"+String(estaciones[0])+"\n"+String(canal2_actualizar)+"\n"+String(estaciones[2]);
      modificar_fichero(SD, "/variables.txt",mensaje.c_str());
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/canal3", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("canal3"))){
      cambiar_variables();
      String canal3_actualizar=request->getParam("canal3")-> value();
      String mensaje = String(tiempo_escucha)+"\n"+String(tiempo_dormido)+"\n"+String(estaciones[0])+"\n"+String(estaciones[1])+"\n"+String(canal3_actualizar);
      modificar_fichero(SD, "/variables.txt",mensaje.c_str());
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
  delay(10);
  }
  
  // JALO
  /// --------------- NO ACTIVO LA RADIO - PONER LIBRERIA DE LUIS y adaptar su codigo aquí y donde haga falta
  radio.initialize();
  radio.setChannel(0);              // Frequency / Channel is *not* set in the initialization. Do it right after.
  #ifdef IS_RFM69HW
    radio.setHighPower(); //uncomment only for RFM69HW!
  #endif
  Serial.println(F("Waiting for signal in region defined in DavisRFM69.h"));

} // Fin Setup()

unsigned long lastRxTime = 0;
byte hopCount = 0;
int actual_channel = 0;
boolean goodCrc = false;
int16_t goodRssi = -999;
unsigned long tiempo1 = 0;
unsigned long tiempo2 = 0;

void loop() {
  cambiar_variables();
  esp_sleep_enable_timer_wakeup(tiempo_dormido*1000000);
  Serial.println("Me despierto y escribo"); 
  //process any serial input
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == 'r') //r=dump all register values
    {
      radio.readAllRegs();
      Serial.println();
    }
    if (input == 't')
    {
      byte temperature =  radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
      byte fTemp = 1.8 * temperature + 32; // 9/5=1.8
      Serial.print(F("Radio Temp is "));
      Serial.print(temperature);
      Serial.print(F("C, "));
      Serial.print(fTemp); //converting to F loses some resolution, obvious when C is on edge between 2 values (ie 26C=78F, 27C=80F)
      Serial.println(F("F"));
    }
  }
  Serial.println("");
  Serial.print("Tiempo escucha: ");
  Serial.println(tiempo_escucha);
  Serial.print("Tiempo dormido: ");
  Serial.println(tiempo_dormido);
  Serial.print("Estacion1: ");
  Serial.println(estaciones[0]);
  Serial.print("Estacion2: ");
  Serial.println(estaciones[1]);
  Serial.print("Estacion3: ");
  Serial.println(estaciones[2]);

  for(int i=0;i<3;i++){
    tiempo2 = millis();
    tiempo1 = millis();
    humedad=0;
    temperatura=0;
    direccion=0;
    velocidad=0;
    temp_float=0;

    //JALO:
    Serial.println(F("Radio: "));
    Serial.println(estaciones[i]); 
    while((tiempo1-tiempo2)<(tiempo_escucha*1000)){
      escuchar(estaciones[i]);
      tiempo1=millis();
    }
    print_information(i);
    logSDCard(i);
  }
  Serial.println("Me duermo"); 
  esp_deep_sleep_start(); 
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
void processPacket() {
  // Every packet has wind speed, direction, and battery status in it
  loopData.windSpeed = radio.DATA[1];
  velocidad = loopData.windSpeed;
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
  direccion = loopData.windDirection;
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
  loopData.channel = (radio.DATA[0] & 0x7);
  canal = loopData.channel;
  #if 0
  Serial.print("Actual channel: ");
  Serial.println(loopData.channel);
  #endif

  // Now look at each individual packet. The high order nibble is the packet type.
  // The highest order bit of the low nibble is set high when the ISS battery is low.
  // The low order three bits of the low nibble are the station ID.

  switch (radio.DATA[0] >> 4) {
  case VP2P_TEMP:
    loopData.outsideTemperature = (int16_t)(word(radio.DATA[3], radio.DATA[4])) >> 4;
    //loopData.outsideTemperature = ((loopData.outsideTemperature-320)*5)/9;
    temperatura = loopData.outsideTemperature;
    temp_float = float(((temperatura-320)*5)/90);
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
    humedad = loopData.outsideHumidity;
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

}
void print_information(int estacion){
  estacion++;
  getTimeStamp();
  Serial.print(dia);
  Serial.print("/");  
  Serial.print(mes);
  Serial.print("/"); 
  Serial.println(anho);
  Serial.print("Hora: ");
  Serial.print(hora);
  Serial.print(":");
  Serial.print(minutos);
  Serial.print(":");
  Serial.println(segundos);
  Serial.print(F("Estación: "));
  Serial.println(estacion);
  Serial.print(F("Temperatura: "));
  Serial.println(temp_float);
  Serial.print(F("Humedad: "));
  Serial.println(humedad);
  Serial.print(F("Dirección del viento: "));
  Serial.println(direccion);
  Serial.print(F("Velocidad del viento: "));
  Serial.println(velocidad);
}
void escuchar(int estacion){
  if (radio.receiveDone()) {
    //if (strmon) printStrm();
    packetStats.packetsReceived++;
    unsigned int crc = radio.crc16_ccitt(radio.DATA, 6);
    if ((crc == (word(radio.DATA[6], radio.DATA[7]))) && (crc != 0)) {
        //if (strmon) printStrm();
        if((radio.DATA[0] & 0x7)==estacion){
          //if (strmon) printStrm();
          processPacket();
          goodCrc = true;
          goodRssi = radio.RSSI;
          packetStats.receivedStreak++;
          hopCount = 1;
        }else{
          goodCrc = false;
          packetStats.crcErrors++;
          packetStats.receivedStreak = 0;
          radio.hop();
        }
      } else {
        goodCrc = false;
        packetStats.crcErrors++;
        packetStats.receivedStreak = 0;
        radio.hop(); 
      }
    if (goodCrc && (radio.RSSI < (goodRssi + 15))) {
      lastRxTime = millis();
      
      radio.hop();
    } else {
      radio.waitHere();
    }
  }
  // If a packet was not received at the expected time, hop the radio anyway
  // in an attempt to keep up.  Give up after 25 failed attempts.  Keep track
  // of packet stats as we go.  I consider a consecutive string of missed
  // packets to be a single resync.  Thx to Kobuki for this algorithm.
  
}
void logSDCard(int i) {
  getTimeStamp();
  i++;
  Nombre_Fichero = "/Estacion"+String(i)+"_"+String(dia)+"_"+String(mes)+"_"+String(anho)+".csv";
  dataMessage = String(dia) + "-" +String(mes) + "-" + String(anho) + ";" + String(hora) + ":" +String(minutos) + ":" + String(segundos) + ";" + String(temp_float) + ";" + String(humedad) + ";" + String(velocidad)+ ";"+ String(direccion) +"\n";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  Serial.println(nombreFichero);
  appendFile(SD, Nombre_Fichero.c_str(), dataMessage.c_str());
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
  }else if(var == "DORMIDO"){
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
  cambiar_variables();
  return String(tiempo_escucha);
}
String dormido(){
  cambiar_variables();
  return String(tiempo_dormido);
}
String estacion1(){
  cambiar_variables();
  return String(estaciones[0]);
}
String estacion2(){
  cambiar_variables();
  return String(estaciones[1]);
}
String estacion3(){
  cambiar_variables();
  return String(estaciones[2]);
}
void cambiar_variables(){
  int error=0;
  error=readFile(SD, "/variables.txt");

  if (error){
    Serial.println("Failed to open file for reading /variables.txt");
    Serial.println("Creating file /variables.txt with actual variables");
    String mensaje = String(tiempo_escucha)+"\n"+String(tiempo_dormido)+"\n"+String(estaciones[0])+"\n"+String(estaciones[1])+"\n"+String(estaciones[2]);
    modificar_fichero(SD, "/variables.txt",mensaje.c_str());
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
      else if(i==2) tiempo_dormido=linea.toInt();
      else if(i==3) estaciones[0]=linea.toInt();
      else if(i==4) estaciones[1]=linea.toInt();
      else if(i==5) estaciones[2]=linea.toInt();
      i++;
    }
    file.close();
    return 0;
}
void modificar_fichero(fs::FS &fs, const char * path,const char * message) {
  Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
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

void SD_dir(AsyncResponseStream *response,AsyncWebServerRequest *request){

  File root = SD.open("/");
  if (root) {
    root.rewindDirectory();
    response->print("<table align='center'>");
    response->print("<tr><th>Nombre</th><th style='width:20%'>Tipo File/Dir</th><th>Tamano</th></tr>");
    printDirectory("/",0,response);
    response->print("</table>");
    response->print("<a href='/'><button>Variables</button></a>");
    root.close();
  }else{
    response->print("<h3>No Files Found</h3>"); 
  }
}
void printDirectory(const char * dirname, uint8_t levels,AsyncResponseStream *response){
  File root = SD.open(dirname);
  if(!root){
    return;
  }
  if(!root.isDirectory()){
    return;
  }
  File file = root.openNextFile();
  int i = 0;
  while(file){
    if(file.isDirectory()){
      response->print("<tr><td>"+String(file.isDirectory()?"Dir":"File")+"</td><td>"+String(file.name())+"</td><td></td></tr>");
      printDirectory(file.name(), levels-1,response);
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