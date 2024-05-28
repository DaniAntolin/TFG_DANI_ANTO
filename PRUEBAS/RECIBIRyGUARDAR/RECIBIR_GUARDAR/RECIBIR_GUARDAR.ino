#include "GUARDAR.h" 
GUARDAR GUARDAR;
#define POWER_SAVING
#define NUM_ESTACIONES 3
int stationID = 0;
int estaciones[NUM_ESTACIONES]={0,1,2}; // Tres estaciones, dentro el stationID entre 0 y 7 (-1, no usar esa estación)
void setup() {
 Serial.begin(SERIAL_BAUD);
  delay(10);
  GUARDAR.recibir.INICIALIZAR_RADIO();
  GUARDAR.INICIALIZAR_GUARDAR ();
}

void loop() {
  if (0) print_variables();
 
  // Para cada ESTACION:
  for(int i=0;i<NUM_ESTACIONES;i++){
    stationID=estaciones[i];

    if ((stationID>=0)){
    int tiempo2 = millis();
    int tiempo1 = millis();
    GUARDAR.recibir.humedad=0; GUARDAR.recibir.N_humedad=0;
    GUARDAR.recibir.temperatura=0; GUARDAR.recibir.N_temperatura=0;
    GUARDAR.recibir.direccion=0; GUARDAR.recibir.N_direccion=0;
    GUARDAR.recibir.velocidad=0; GUARDAR.recibir.N_velocidad=0;
    GUARDAR.recibir.temp_float=0; 
    GUARDAR.recibir.DataAll=false;

    Serial.print(F("---------------------- "));
    Serial.print(F("Estacion: "));
    Serial.print(i+1);
    Serial.print(F(" -- ID: "));
    Serial.println(stationID); 
    
    while(((tiempo1-tiempo2)<(GUARDAR.tiempo_escucha*1000))&&(!GUARDAR.recibir.DataAll)){
      GUARDAR.recibir.escuchar(stationID); 
      tiempo1=millis();
      process_serial_commands();
    } // While

    Serial.print(F("tiempo de escucha: ")); Serial.println(tiempo1-tiempo2);
    GUARDAR.GUARDAR_CSV ();
    } 

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
      GUARDAR.recibir.radio.readAllRegs();
      Serial.println();
    }
    if (input == 't') // read radio temp
    {
      byte temperature =  GUARDAR.recibir.radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
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
      Serial.print(GUARDAR.recibir.idErrors);
      Serial.print(F(" packets/min: "));
      unsigned long min = millis()/60000;
      unsigned long correct_packets = packetStats.packetsReceived - packetStats.crcErrors - GUARDAR.recibir.idErrors;
      float rate = 0.0;
      if (min>0) rate = correct_packets/min;
      Serial.println(rate);
    }
    if (input == 'h') // manually hop radio channel
    {
      GUARDAR.recibir.radio.hop();
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
        Serial.println(GUARDAR.recibir.radio.CHANNEL);
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
        GUARDAR.print_information(stationID);
    }
  }
}
void print_variables(){
  Serial.println("");
  Serial.print("Tiempo escucha: ");
  Serial.println(GUARDAR.tiempo_escucha);
  Serial.print("Tiempo wifi: ");
  Serial.println(GUARDAR.tiempo_wifi);
  Serial.print("Estacion1 (ID): ");
  Serial.print(GUARDAR.estaciones[0]);
  Serial.print(" -- Estacion2 (ID): ");
  Serial.print(GUARDAR.estaciones[1]);
  Serial.print(" -- Estacion3 (ID): ");
  Serial.println(GUARDAR.estaciones[2]);
}