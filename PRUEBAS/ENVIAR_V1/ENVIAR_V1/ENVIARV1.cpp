void ENVIARV1:: SD_dir(AsyncResponseStream *response,AsyncWebServerRequest *request){

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

void ENVIARV1:: printDirectory(const char * dirname, uint8_t levels,AsyncResponseStream *response){
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
      file_size(bytes);
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

String ENVIARV1:: file_size(int bytes){ // CUANTO OCUPA EL FICHERO
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes)+" B";
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,3)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
  else                              fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
  return fsize;
}

void ENVIARV1:: SD_file_delete(String filename, AsyncResponseStream *response) { // Delete the file 
  File dataFile = SD.open("/"+filename, FILE_READ); // Now read data from SD Card 
  if (dataFile){
    if (SD.remove("/"+filename)) {
      response->print("alert('Archivo eliminado')");
    }else{ 
      response->print("alert('Archivo NO eliminado')");
    }
  } 
}

String ENVIARV1:: hora_actual() {
  getTimeStamp();
  String hora_actual = String(dia) + "/" +String(mes) + "/" + String(anho)+"   "+String(hora) + ":" +String(minutos) + ":" + String(segundos);
  return String(hora_actual);
}

String ENVIARV1:: processor(const String& var){
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
String ENVIARV1:: escucha(){
   return String(tiempo_escucha);
}
String ENVIARV1:: encwifi(){
   return String(tiempo_wifi);
}
String ENVIARV1:: estacion1(){
   return String(estaciones[0]);
}
String ENVIARV1:: estacion2(){
   return String(estaciones[1]);
}
String ENVIARV1:: estacion3(){
   return String(estaciones[2]);
}

void ENVIARV1:: WiFiEvent(WiFiEvent_t event){
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
void ENVIARV1:: conectarServidor(){
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
      GUARDAR.modificar_fichero(SD, "/variables.txt",mensaje.c_str(),1,t_escucha_actualizar);
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/tiempo_wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("tiempo_wifi"))){      
      String t_wifi_actualizar=request->getParam("tiempo_wifi")-> value();
      String mensaje = String(tiempo_escucha)+"\n"+String(t_wifi_actualizar)+"\n"+String(estaciones[0])+"\n"+String(estaciones[1])+"\n"+String(estaciones[2]);
      GUARDAR.modificar_fichero(SD, "/variables.txt",mensaje.c_str(),2,t_wifi_actualizar);
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/canal1", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("canal1"))){
      String canal1_actualizar=request->getParam("canal1")-> value();
      String mensaje = String(tiempo_escucha)+"\n"+String(tiempo_wifi)+"\n"+String(canal1_actualizar)+"\n"+String(estaciones[1])+"\n"+String(estaciones[2]);
      GUARDAR.modificar_fichero(SD, "/variables.txt",mensaje.c_str(),3,canal1_actualizar);
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/canal2", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("canal2"))){
      String canal2_actualizar=request->getParam("canal2")-> value();
      String mensaje = String(tiempo_escucha)+"\n"+String(tiempo_wifi)+"\n"+String(estaciones[0])+"\n"+String(canal2_actualizar)+"\n"+String(estaciones[2]);
      GUARDAR.modificar_fichero(SD, "/variables.txt",mensaje.c_str(),4,canal2_actualizar);
      }
    request->send(200, "text/html", index_html); 
  });
  server.on("/canal3", HTTP_GET, [](AsyncWebServerRequest *request){
    if ((request->hasParam("canal3"))){
      String canal3_actualizar=request->getParam("canal3")-> value();
      String mensaje = String(tiempo_escucha)+"\n"+String(tiempo_wifi)+"\n"+String(estaciones[0])+"\n"+String(estaciones[1])+"\n"+String(canal3_actualizar);
      GUARDAR.modificar_fichero(SD, "/variables.txt",mensaje.c_str(),5,canal3_actualizar);
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
void ENVIARV1:: wifi_ON(){
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  WiFi.onEvent(WiFiEvent);

}
// Interruppcion & WiFi

void ENVIARV1:: comprobarEstadoBoton_WiFi(){
  tiempoACT=millis();  //tiempo actual

  if (encenderWifi){
    RECIBIR.radio.sleep(); //Evitar interrupcion de radio durante la escritura de SD
    wifi_ON(); 
    conectarServidor();
    ServidorON=true;
    encenderWifi=false;
    apagarWifi=false; //Está ya a false pero por si acaso
   }; 

  if ((ServidorON)&&((tiempoACT-tiempoINT)>(60*tiempo_wifi*1000))) apagarWifi = true;

  if (apagarWifi){
    WiFi.mode(WIFI_OFF);
    Serial.println(F("Wifi Apagada "));
    delay(1000);  
    ServidorON=false;
    tiempoINT=0;  //Se apaga el servidor (si es interrupcion o si es timeout hay que ponerlo)
    apagarWifi=false;
    encenderWifi=false; //Está ya a false pero por si acaso
    RECIBIR.radio.setChannel(radio.CHANNEL); //Reactivar la radio (se invoca ReceiveBegin())
   }
}
void ENVIARV1:: Init_Servidor (){
 // Inicializar Servidor
  if((anho.toInt()<2022)||(encenderWifi)){
  wifi_ON(); // Modo arranque wifi con WiFi.h
  conectarServidor();
  ServidorON=true;
  encenderWifi=false;
  apagarWifi=false;
  } //Servidor
}
void  ENVIARV1:: Init_BOTON (){
//Interrupciones (botón):
  pinMode(PINTIERRA, OUTPUT);
  digitalWrite(PINTIERRA,LOW);
  pinMode(PINBOTON, INPUT_PULLUP); // Configurar el pin como entrada con resistencia pull-up interna
  attachInterrupt(digitalPinToInterrupt(PINBOTON), miFuncionInterrupcion, RISING); // Configurar la interrupción
}

  