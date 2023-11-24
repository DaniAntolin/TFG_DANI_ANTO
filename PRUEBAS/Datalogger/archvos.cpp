#include <Arduino.h>
#include "datalogger.h"


//datalogger::datalogger() : server(80) {} // implementación de la función constructora por defecto
datalogger::datalogger(uint16_t port) : server(port) {}
datalogger::datalogger() : client("http://192.168.4.1:8086", "Datalogger"), server(80) {}

void datalogger::getTimeStamp() { //mira el tiempo actual
  DateTime now = rtc.now();
  hora = now.hour();
  minutos = now.minute();
  segundos = now.second();
  anho = now.year();
  mes = now.month();
  dia = now.day();
}

void datalogger::cambiar_variables() {
  int i = 1; 
  File file = SD.open("/variables.txt", FILE_WRITE); // abre el archivo y lo carga en variable file
  if (!file) {  // si no hay nada cargado ERROR
    Serial.println("Failed to open file for reading variables.txt");
    return;
  }
  while (file. available()) { // mira si hay archivos para leer
    String linea = file.readStringUntil('\n'); // guarda una linea del archivo, para posteriormente ir guardando cada linea en su correspondiente variable
    if (i == 1) tiempo_escucha = linea.toInt(); // puede que sea mas intuitivo un switch
    else if (i == 2) tiempo_dormido = linea.toInt();
    else if (i == 3) estaciones[0] = linea.toInt();
    else if (i == 4) estaciones[1] = linea.toInt();
    else if (i == 5) estaciones[2] = linea.toInt();
    else if (i == 6) tiempo_despertarse = linea.toInt();
    else if (i == 7) zona_horaria = linea.toInt();
    Serial.println(linea);//
    i++;
  }
  file.close(); // se cierra el archivo
}

void datalogger::printvariables(){
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
  Serial.print("Tiempo despertarse: ");
  Serial.println(tiempo_despertarse);
  Serial.print("Zona horaria: ");
  Serial.println(zona_horaria);
}

void datalogger::escuchar(int stationID) {
  // The check for a zero CRC value indicates a bigger problem that will need
  // fixing, but it needs to stay until the fix is in.
  if (radio.receiveDone()) {
    packetStats.packetsReceived++;
    unsigned int crc = radio.crc16_ccitt(radio.DATA, 6);
    if ((crc == (word(radio.DATA[6], radio.DATA[7]))) && (crc != 0)) {
      goodCrc = true;
      packetStats.receivedStreak++;
      processPacket(); // This is a good packet
      correctID = (stationID == int (radio.DATA[0] & 0x7));
      if (correctID) 
        hopCount = 1; // From correct station
      else
        idErrors++; // From wrong station
    } else {
      goodCrc = false; // Incorrect packet
      packetStats.crcErrors++;
      packetStats.receivedStreak = 0;
    }
    // Radio hop only for correct packects (i.e. good CRC and correct station)
    if (goodCrc && correctID) {
      // Hop radio & update channel and lastRxTime
      lastRxTime = millis();  
      radio.hop();
#ifdef POWER_SAVING
      esp_sleep_enable_timer_wakeup(2 * 1000000); // light sleep (2s) to save energy
      esp_light_sleep_start();
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
    esp_sleep_enable_timer_wakeup(2 * 1000000); // light sleep (2s) to save energy
    esp_light_sleep_start();
#endif
  }
}

void datalogger::print_information(int estacion) {
  estacion++;
  getTimeStamp(); //mira el tiempo actual
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

void datalogger::reset_datos(){
  humedad = 999;
  direccion = 999;
  velocidad = 999;
  temp_float = 999;
}

void datalogger::logSDCard(int i) {
  getTimeStamp(); ////mira el tiempo actual
  i++;
  Nombre_Fichero = "/Estacion" + String(i) + "_" + String(dia) + "_" + String(mes) + "_" + String(anho) + ".csv"; // crea un fichero de cada estacion por dia
  dataMessage = String(dia) + "-" + String(mes) + "-" + String(anho) + ";" + String(hora) + ":" + String(minutos) + ":" + String(segundos) + ";" + String(temp_float) + ";" + String(humedad) + ";" + String(velocidad) + ";" + String(direccion) + "\n"; // los datos
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
}

void datalogger::processPacket() { // procesa el paquete
  // Every packet has wind speed, direction, and battery status in it
  loopData.windSpeed = radio.DATA[1]; // ,, byte 2
  velocidad = loopData.windSpeed;
  #if 0
      Serial.print("Velocidad de viento: ");
      Serial.print(loopData.windSpeed);
      Serial.print("  Rx Byte 1: ");
      Serial.println(radio.DATA[1]);
  #endif

  // There is a dead zone on the wind vane. No values are reported between 8
  // and 352 degrees inclusive. These values correspond to received byte
  // values of 1 and 255 respectively
  // See http://www.wxforum.net/index.php?topic=21967.50
  loopData.windDirection = radio.DATA[2] * 360.0f / 255.0f; // 9 + radio.DATA[2] * 342.0f / 255.0f
  direccion = loopData.windDirection;
  #if 0
      Serial.print(F("Wind Direction: "));
      Serial.print(loopData.windDirection);
      Serial.print(F("  Rx Byte 2: "));
      Serial.println(radio.DATA[2]);
  #endif

  loopData.transmitterBatteryStatus = (radio.DATA[0] & 0x8) >> 3; // estado de la bateria ,, byte 1
  #if 0
      Serial.print("Battery status: ");
      Serial.println(loopData.transmitterBatteryStatus);
  #endif
  canal = (radio.DATA[0] & 0x7); // ,, byte 1
  #if 0
    Serial.print("Actual channel: ");
    Serial.println(loopData.channel);
  #endif

  // Now look at each individual packet. The high order nibble is the packet type.
  // The highest order bit of the low nibble is set high when the ISS battery is low.
  // The low order three bits of the low nibble are the station ID.

  switch (radio.DATA[0] >> 4) {
    case VP2P_TEMP:
      loopData.outsideTemperature = (int16_t)(word(radio.DATA[3], radio.DATA[4]));
    temp_float = (((loopData.outsideTemperature/160)-32)*5)/9;
  #if 1
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
      humedad = loopData.outsideHumidity; // esto es la variable humedad
  #if 1
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

void datalogger::printStrm() {
  for (byte i = 0; i < DAVIS_PACKET_LEN; i++) {
    Serial.print(i);
    Serial.print(F(" = "));
    Serial.print(radio.DATA[i], HEX);
    Serial.print(F("\n\r"));
  }
  Serial.print(F("\n\r"));
}

void datalogger::inicializar(){
  //Inizializo SD
  if (!SD.begin(33)) {    // define el pin de selección (33)
    Serial.println("Card Mount Failed 1");
    return;
  }
  delay(10);
  //Inizializo RTC
  if (!rtc.begin()) { // intenta inicializar el módulo RTC
    Serial.println("No se pudo encontrar el módulo RTC. Verifica la conexión.");
    Serial.flush(); // Espera a que todos los datos pendientes se envíen antes de continuar
    while (1) delay(10);
  }
  if (!rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    rtc.adjust("2020:01:01:01:01:01");
  }
  rtc.start(); //empieza el contador
  delay(10);
  getTimeStamp(); // carga el reloj actual
  cambiar_variables(); // almacena los datos almacenados en un archivo .txt en sus respectivas variables
  //
  
  //
  radio.initialize();   //se inicializa el modulo de radio pin 32 es el de cs, el 15 el de reset y el 27 el de interrupciones
  radio.setChannel(0);  // Frequency / Channel is *not* set in the initialization. Do it right after.
#ifdef IS_RFM69HW
  radio.setHighPower();  //uncomment only for RFM69HW!
#endif
  Serial.println(F("Waiting for signal in region defined in DavisRFM69.h"));
}

String datalogger::hora_actual() {
  getTimeStamp();
  String hora_actual = String(dia) + "/" + String(mes) + "/" + String(anho) + "   " + String(hora) + ":" + String(minutos) + ":" + String(segundos);
  return String(hora_actual);
}

String datalogger::processor(const String &var) {
  if (var == "HORA") {
    return hora_actual();
  } else if (var == "ESCUCHA") {
    return String(tiempo_escucha);
  } else if (var == "DORMIDO") {
    return String(tiempo_dormido); //JALO
  } else if (var == "ESTACION1") {
    return String(estaciones[0]);
  } else if (var == "ESTACION2") {
    return String(estaciones[1]);
  } else if (var == "ESTACION3") {
    return String(estaciones[2]);
  } else if (var == "DESPIERTO") {
    return String(tiempo_despertarse);
  }
  return String();
}

String datalogger::escucha() {
  cambiar_variables();
  return String(tiempo_escucha);
}

String datalogger::dormido() {
  cambiar_variables();
  return String(tiempo_dormido);
}

String datalogger::despertarse() {
  cambiar_variables();
  return String(tiempo_despertarse);
}

String datalogger::estacion1() {
  cambiar_variables();
  return String(estaciones[0]);
}

String datalogger::estacion2() {
  cambiar_variables();
  return String(estaciones[1]);
}

String datalogger::estacion3() {
  cambiar_variables();
  return String(estaciones[2]);
}

void datalogger::modificar_fichero(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println(message);///
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void datalogger::SD_dir(AsyncResponseStream *response, AsyncWebServerRequest *request) {

  File root = SD.open("/");
  if (root) {
    root.rewindDirectory();
    response->println("<html> <head>");
    response->println("<script language='JavaScript'>");
    response->println("function pregunta(){");
    response->println("if (confirm('Seguro que quiere borrar este archivo?')){");
    response->println("return true;}");
    response->println("else{");
    response->println("return false;} };");
    response->println("function pregunta_desconectar(){");
    response->println("if (confirm('Seguro que quiere Desconectar?')){");
    response->println("return true;}");
    response->println("else{");
    response->println("return false;}");
    response->println("}</script> </head>");
    response->println("<body>");

    response->print("<table align='center'>");
    response->print("<tr><th>Nombre</th><th style='width:20%'>&nbsp;&nbsp;</th><th>Tamano</th></tr>");
    printDirectory("/", 0, response);
    response->print("</table>");
    response->print("<a href='/'><button>Variables</button></a>");
    response->print("<br>");
    response->print("<br>");
    response->print("<a onclick='return pregunta_desconectar()' href='/desconectar'><button>Desconectar</button></a>");

    response->println("</body> </html>");

    root.close();
  } else {
    response->print("<h3>No Files Found</h3>");
    root.close();
  }
}

void datalogger::printDirectory(const char *dirname, uint8_t levels, AsyncResponseStream *response) {
  File root = SD.open(dirname);
  if (!root) {
    return;
  }
  if (!root.isDirectory()) {
    return;
  }
  //Leer nombre de los archivos:
  File file = root.openNextFile();
  int cont = 0;
  boolean noFin = true;

  String arch;
  while ((file) && (noFin)) {
    if (file.isDirectory()) {
      // Nolo guardo
    } else {
      arch = String(file.name());
      if (arch.startsWith("Estacion")) {  //Solo los archivos de las estaciones
        listaFiles[cont] = file.name();
        sizeFiles[cont] = file.size();
        cont++;
      }
    }
    file = root.openNextFile();
    if (cont >= MAX_FILES) {
      Serial.printf("Error, numero de archivos mayor de %d\n", MAX_FILES);
      noFin = false;  //Me salgo del while
    }
  };
  file.close();

  QuickSortDesc(listaFiles, sizeFiles, 0, cont - 1);

  String ant = "  ";
  for (int f = 0; f < cont; f++) {
    arch = listaFiles[f];
    int bytes = sizeFiles[f];
    if (!arch.startsWith(ant)) {
      response->print("<tr><td><hr></td><td><hr></td><td><hr></td><td><hr></td><td><hr></td></tr>");
      ant = arch.substring(0, 10);
      Serial.printf("%s \n", ant.c_str());
    }
    response->print("<tr><td>" + arch + "</td>");
    response->print("<td> &nbsp; </td>");
    String fsize = "";
    if (bytes < 1024) fsize = String(bytes) + " B";
    else if (bytes < (1024 * 1024)) fsize = String(bytes / 1024.0, 3) + " KB";
    else if (bytes < (1024 * 1024 * 1024)) fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
    else fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
    response->print("<td>" + fsize + "</td>");
    response->print("<td>");
    response->print("<FORM action='/download' method='get'>");
    response->print("<button type='submit' name='download'");
    response->print("' value='");
    response->print(arch);
    response->print("'>Descargar</button>");
    response->print("</FORM> </td>");
    response->print("<td>");
    response->print("<FORM name='form_borrar' action='/delete' method='get'>");
    response->print("<button type='submit' onclick='return pregunta()' name='delete'");
    response->print("' value='");
    response->print(arch);
    response->print("'>Eliminar</button>");
    response->print("</FORM> </td>");
    response->print("</tr>");
  }
}

String datalogger::file_size(int bytes) {
  String fsize = "";
  if (bytes < 1024) fsize = String(bytes) + " B";
  else if (bytes < (1024 * 1024)) fsize = String(bytes / 1024.0, 3) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
  else fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
  return fsize;
}

void datalogger::SD_file_delete(String filename, AsyncResponseStream *response) {  // Delete the file
  File dataFile = SD.open("/" + filename, FILE_READ);                  // Now read data from SD Card
  if (dataFile) {
    if (SD.remove("/" + filename)) {
      response->print("alert('Archivo eliminado')");
    } else {
      response->print("alert('Archivo NO eliminado')");
    }
  }
  dataFile.close();
}

void datalogger::QuickSortDesc(String *lista, int *tam, const int left, const int right) {
int i = left, j = right;
  String tmp;
  int tmp2;

  String pivot = lista[(left + right) / 2];

  while (i <= j) {
    while (lista[i] > pivot) i++;
    while (lista[j] < pivot) j--;
    if (i <= j) {
      tmp = lista[i];
      tmp2 = tam[i];
      lista[i] = lista[j];
      tam[i] = tam[j];
      lista[j] = tmp;
      tam[j] = tmp2;

      i++;
      j--;
    }
  };

  if (left < j)
    QuickSortDesc(lista, tam, left, j);
  if (i < right)
    QuickSortDesc(lista, tam, i, right);
}

bool datalogger::compruebo_dormir() { // ver si se ha dormido
  if ((WiFi.softAPgetStationNum() > 0) && (forzar_desconectar == 0)) { //WiFi.softAPgetStationNum() para conocer cuántos dispositivos se han conectado al punto de acceso que has creado
    int tiempo_pasado = millis(); // si no se fuerza el punto de acceso wifi a desconectarse y hay alguien conectado miras el tiempo que va pasando
    if (tiempo_pasado - tiempo_desconecto >= 15 * 60 * 1000) { // entiendo que miras el whatchdog si ha pasado mas tiempo eso significa que se ha dormido y sino esta despierto, DUDOSO A MIRAR
      return true;
    }
    return false;
  } else
    return true;
}

void datalogger::initialize_server(){
  //----------------- JALO
  int auxT=tiempo_despertarse+(tiempo_dormido/60)+2; 
  int wifiSI=0;
  if (auxT>=59) {
    auxT=(59-tiempo_despertarse)+(auxT-59);
    if((minutos.toInt()>=(tiempo_despertarse-3))||(minutos.toInt()<=auxT))  {wifiSI=1;}
    }
  else {
    if(((minutos.toInt()>=(tiempo_despertarse-3))&&(minutos.toInt()<=auxT))) {wifiSI=1;}  
  } 
  
  //if((wifiSI)||(anho.toInt()<2022)){ 
  //------------------ JALO
  if((minutos.toInt()>0)&&(minutos.toInt()<60)){
  //if (((minutos.toInt() >= (tiempo_despertarse - 2)) && (minutos.toInt() <= (tiempo_despertarse + (tiempo_dormido / 60) + 1))) || (anho.toInt() < 2022)) {
  	
    WiFi.softAP(ssid);
    tiempo_desconecto = millis();
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

    server.on("/variables", HTTP_GET, [this](AsyncWebServerRequest *request) {
    String hora = this->processor("HORA");
    String tiempoescucha = this->processor("ESCUCHA");
    String tiempodormido = this->processor("DORMIDO");
    String estacion1 = this->processor("ESTACION1");
    String estacion2 = this->processor("ESTACION2");
    String estacion3 = this->processor("ESTACION3");
    String tiempodespertarse = this->processor("DESPIERTO");

    String response = "{\"hora\": \"" + hora + "\", \"estacion3\": \"" + estacion3 + "\", \"estacion2\": \"" + estacion2 + "\", \"estacion1\": \"" + estacion1 + "\", \"tiempodespertarse\": \"" + tiempodespertarse + "\", \"tiempodormido\": \"" + tiempodormido + "\", \"tiempoescucha\": \"" + tiempoescucha + "\"}";
    request->send(200, "application/json", response);
    });

    server.on("/actualizar", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if ((request->hasParam("hour")) && (request->hasParam("minutes")) && (request->hasParam("seconds")) && (request->hasParam("year")) && (request->hasParam("mounth")) && (request->hasParam("day"))) {
        String hora_actualizar = request->getParam("hour")->value();
        String minutos_actualizar = request->getParam("minutes")->value();
        String segundos_actualizar = request->getParam("seconds")->value();
        String anho_actualizar = request->getParam("year")->value();
        String mes_actualizar = request->getParam("mounth")->value();
        String dia_actualizar = request->getParam("day")->value();
        fecha = String(anho_actualizar) + ":" + String(mes_actualizar) + ":" + String(dia_actualizar) + ":" + String(hora_actualizar) + ":" + String(minutos_actualizar) + ":" + String(segundos_actualizar);
        rtc.adjust(fecha.c_str());
    }
    request->send(200, "text/html", index_html);
});
    server.on("/tiempo_escucha", HTTP_GET, [this](AsyncWebServerRequest *request) {
      if ((request->hasParam("tiempo_escucha"))) {
        String t_escucha_actualizar = request->getParam("tiempo_escucha")->value();
        String mensaje = String(t_escucha_actualizar) + "\n" + String(tiempo_dormido) + "\n" + String(estaciones[0]) + "\n" + String(estaciones[1]) + "\n" + String(estaciones[2]) + "\n" + String(tiempo_despertarse);
        modificar_fichero(SD, "/variables.txt", mensaje.c_str());
        cambiar_variables();
      }
      request->send(200, "text/html", index_html);
    });
    server.on("/tiempo_dormido", HTTP_GET, [this](AsyncWebServerRequest *request) {
      if ((request->hasParam("tiempo_dormido"))) {
        String t_dormido_actualizar = request->getParam("tiempo_dormido")->value();
        //String mensaje = String(tiempo_escucha) + "\n" + String(t_dormido_actualizar.toInt() * 60) + "\n" + String(estaciones[0]) + "\n" + String(estaciones[1]) + "\n" + String(estaciones[2]) + "\n" + String(tiempo_despertarse);
        // JALO
        String mensaje = String(tiempo_escucha) + "\n" + String(t_dormido_actualizar.toInt()) + "\n" + String(estaciones[0]) + "\n" + String(estaciones[1]) + "\n" + String(estaciones[2]) + "\n" + String(tiempo_despertarse);
        modificar_fichero(SD, "/variables.txt", mensaje.c_str());
        cambiar_variables();
      }
      request->send(200, "text/html", index_html);
    });
    server.on("/tiempo_despierto", HTTP_GET, [this](AsyncWebServerRequest *request) {
      if ((request->hasParam("tiempo_despierto"))) {
        String t_despertarse_actualizar = request->getParam("tiempo_despierto")->value();
        String mensaje = String(tiempo_escucha) + "\n" + String(tiempo_dormido) + "\n" + String(estaciones[0]) + "\n" + String(estaciones[1]) + "\n" + String(estaciones[2]) + "\n" + String(t_despertarse_actualizar);
        modificar_fichero(SD, "/variables.txt", mensaje.c_str());
        cambiar_variables();
      }
      request->send(200, "text/html", index_html);
    });
    server.on("/canal1", HTTP_GET, [this](AsyncWebServerRequest *request) {
      if ((request->hasParam("canal1"))) {
        String canal1_actualizar = request->getParam("canal1")->value();
        String mensaje = String(tiempo_escucha) + "\n" + String(tiempo_dormido) + "\n" + String(canal1_actualizar) + "\n" + String(estaciones[1]) + "\n" + String(estaciones[2]) + "\n" + String(tiempo_despertarse);
        modificar_fichero(SD, "/variables.txt", mensaje.c_str());
        cambiar_variables();
      }
      request->send(200, "text/html", index_html);
    });
    server.on("/canal2", HTTP_GET, [this](AsyncWebServerRequest *request) {
      if ((request->hasParam("canal2"))) {
        String canal2_actualizar = request->getParam("canal2")->value();
        String mensaje = String(tiempo_escucha) + "\n" + String(tiempo_dormido) + "\n" + String(estaciones[0]) + "\n" + String(canal2_actualizar) + "\n" + String(estaciones[2]) + "\n" + String(tiempo_despertarse);
        modificar_fichero(SD, "/variables.txt", mensaje.c_str());
        cambiar_variables();
      }
      request->send(200, "text/html", index_html);
    });
    server.on("/canal3", HTTP_GET, [this](AsyncWebServerRequest *request) {
      if ((request->hasParam("canal3"))) {
        String canal3_actualizar = request->getParam("canal3")->value();
        String mensaje = String(tiempo_escucha) + "\n" + String(tiempo_dormido) + "\n" + String(estaciones[0]) + "\n" + String(estaciones[1]) + "\n" + String(canal3_actualizar) + "\n" + String(tiempo_despertarse);
        modificar_fichero(SD, "/variables.txt", mensaje.c_str());
        cambiar_variables();
      }
      request->send(200, "text/html", index_html);
    });
    server.on("/download", HTTP_GET, [this](AsyncWebServerRequest *request) {
      File descarga = SD.open("/" + request->arg("download"));
      Serial.print(request->arg("download"));
      request->send(descarga, request->arg("download"), "text/html", true);
    });
    server.on("/delete", HTTP_GET, [this](AsyncWebServerRequest *request) {
      File eliminar = SD.open("/" + request->arg("download"));
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      SD_file_delete(request->arg("delete"), response);
      request->redirect("/files");
    });
    server.on("/desconectar", HTTP_GET, [this](AsyncWebServerRequest *request) {
      forzar_desconectar = 1;
      Serial.println("Fuerzo desconexión");
      request->redirect("/");
    });
    server.on("/files", HTTP_GET, [this](AsyncWebServerRequest *request) {
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      SD_dir(response, request);
      request->send(response);
    });
    
    
    // Start server
    server.begin();
    Serial.println(F("Servidor iniciado"));
    delay(10);
  }
  }
//}

void datalogger::save_data(int i){ //guardar datos
  i++;
  String fileName = "/datos.txt"; //archivo datos.txt
  File dataFile = SD.open(fileName.c_str(), FILE_APPEND); //abre el archivo
  String dataString = String(rtc.now().unixtime())+";"+ "Estacion_"+String(i) + ";" + String(temp_float) + ";" + String(humedad) + ";" + String(velocidad) + ";" + String(direccion); //guarda los datos en una variable
  if (dataFile) { // ve si se ha abierto bien el archivo
    dataFile.println(dataString); // escrive los datos de la variable en el archivo 
    dataFile.close(); //cierra el archivo
    Serial.printf("Datos de la estación %d guardados correctamente\n", i);
  } else {
    Serial.println("Error al abrir el archivo de datos");
  }
  String fileName2 = "/backup.txt"; // lo mismo que el archivo anterior, clonado puesto que utiliza la misma variable para escribir
  File dataFile2 = SD.open(fileName2.c_str(), FILE_APPEND);
  if (dataFile2) {
    dataFile2.println(dataString);
    dataFile2.close();
    Serial.printf("Datos de la estación %d guardados correctamente\n", i);
  } else {
    Serial.println("Error al abrir el archivo de datos");
  }
}
void datalogger::send_data(){ // envio de datos
  WiFi.begin("datalogger", "datalogger"); //Conectarse a la red WiFi ssid: datalogger, contraseña:datalogger
  unsigned long startAttemptTime = millis(); //ver cuando comienza el envio de datos por wifi
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 5000) { // espera hasta que se conecta o pasa el tiempo establecido de 5 segunods
    delay(500); // Esperar medio segundo antes de intentar nuevamente
  }
  if(WiFi.status() == WL_CONNECTED) { //si se cumple es que se ha conectado y no es por que ha transcurrido el tiempo
    Serial.println("Conectado");
    changeInfluxdb(); //protocolo http para enviar a grafana puede ser
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
      point.addField("velocidad", values[4].toInt());
      Serial.println(values[4]);
      point.addField("direccion", values[5].toInt());
      Serial.println(values[5]);

      Serial.print("Writing point: ");
      Serial.println(client.pointToLineProtocol(point)); // para pintar en grafana con influxdb
      if (!client.writePoint(point)) {
        Serial.println("Failed to write point to InfluxDB: " + client.getLastErrorMessage());    
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

void datalogger::changeInfluxdb() { //protocolo http
  HTTPClient http;
 String query = "SELECT LAST(canal1) AS canal1, LAST(canal2) AS canal2, LAST(canal3) AS canal3, LAST(tiempo_escucha) AS tiempo_escucha, LAST(tiempo_dormido) AS tiempo_dormido, LAST(zona) AS zona FROM VARIABLES.autogen.variables";
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

void datalogger::setRTC(){ // actualizar hora
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
