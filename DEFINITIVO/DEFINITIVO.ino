#include "ENVIARV1.h" 
ENVIARV1 ENVIARV1;
#include "RECIBIR.h" 
RECIBIR RECIBIR;
#include "GUARDAR.h" 
GUARDAR GUARDAR;
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(100);
  GUARDAR.INICIALIZAR_GUARDAR ();
  ENVIARV1.Init_BOTON ();
  ENVIARV1.Init_Servidor();
  RECIBIR.INICIALIZAR_RADIO();
} // Fin Setup()

//===========================================================================
void loop() {
  if (0) print_variables();

  ENVIAR.comprobarEstadoBoton_WiFi();
 
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
      RECIBIR.escuchar(stationID);  
      tiempo1=millis();

      process_serial_commands();

      ENVIAR.comprobarEstadoBoton_WiFi();
    } // While

    Serial.print(F("tiempo de escucha: ")); Serial.println(tiempo1-tiempo2);

    print_information();
    
    GUARDAR_CSV (stationID,N_velocidad,N_velocidad, N_temperatura, N_humedad);   //Guardar en el archivo Datos
    } // stationID

  }
}
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