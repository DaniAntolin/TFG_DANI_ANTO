// Sample usage of the DavisRFM69 library to sniff the packets from a Davis Instruments
// wireless Integrated Sensor Suite (ISS).
//
// This is part of the DavisRFM69 library from https://github.com/dekay/DavisRFM69
// (C) DeKay 2014-2016 dekaymail@gmail.com
// Example released under the MIT License (http://opensource.org/licenses/mit-license.php)
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/
//
// This program has been adapted by Luis Piñuel to work on an Adafruit Feather ESP32 V2 board 
// with an attached RFM69W Feather Wing connected (see DavisRFM69.cpp for connection details).
//


#include "RECIBIR.h" 
RECIBIR RECIBIR;
#define POWER_SAVING
int stationID = 0;
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(100);
  RECIBIR.INICIALIZAR_RADIO();
}



void loop() {
  //process any serial input
  process_serial_commands();
  RECIBIR.escuchar(stationID);
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
      Serial.println(F("- i: set the current station ID"));
      Serial.println(F("- I: show the current station ID"));
    }
    if (input == 'r') //r=dump all register values
    {
      RECIBIR.radio.readAllRegs();
      Serial.println();
    }
    if (input == 't') // read radio temp
    {
      byte temperature =  RECIBIR.radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
      byte fTemp = 9 * temperature / 5 + 32; // 9/5=1.8
      Serial.print(F("Radio Temp is "));
      Serial.print(temperature);
      Serial.print(F("C, "));
      Serial.print(fTemp); //converting to F loses some resolution, obvious when C is on edge between 2 values (ie 26C=78F, 27C=80F)
      Serial.println(F("F"));
    }
    if (input == 's') // show packet stats
    {
      Serial.print(F(" packetsReceived: "));
      Serial.print(packetStats.packetsReceived);
      /*Serial.print(F(" goodPacket is: "));
      Serial.print(RECIBIR.goodpacket);*/
      /*Serial.print(F(" Paquetes de humedad recividos: "));
      Serial.print(RECIBIR.HumRecived);
      Serial.print(F(" % humedad recivida: "));
      Serial.print(RECIBIR.HumRecivedToT);
      Serial.print(F(" Paquetes de temperatura recividos: "));
      Serial.print(RECIBIR.TemRecived);
      Serial.print(F(" % temperatura recivida: "));
      Serial.print(RECIBIR.TemRecivedToT);*/
      Serial.print(F(" packetsMissed: "));
      Serial.print(packetStats.packetsMissed);
      Serial.print(F(" numResyncs: "));
      Serial.print(packetStats.numResyncs);
      Serial.print(F(" receivedStreak: "));
      Serial.print(packetStats.receivedStreak);
      Serial.print(F(" crcErrors: "));
      Serial.print(packetStats.crcErrors); 
      Serial.print(F(" idErrors: "));
      Serial.print(RECIBIR.idErrors);
      Serial.print(F(" packets/min: "));
      unsigned long min = millis()/60000;
      unsigned long correct_packets = packetStats.packetsReceived - packetStats.crcErrors - RECIBIR.idErrors;
      float rate = 0.0;
      if (min>0) rate = correct_packets/min;
      Serial.println(rate);
    }
    if (input == 'h') // manually hop radio channel
    {
      RECIBIR.radio.hop();
    }
    if (input == 'R') // reset radio
    {
        RECIBIR.radio.reset();
    }
    if (input == 'm') // show time in milliseconds
    {
        Serial.println(millis());
    }
    if (input == 'c') // show current radio channel
    {
        Serial.println(RECIBIR.radio.CHANNEL);
    }
    if (input == 'i') // set the current station ID, ej: i 1
    {
        stationID = Serial.parseInt();
    }
    if (input == 'I') // show the current station ID
    {
        Serial.println(stationID);
    }
  }
}
