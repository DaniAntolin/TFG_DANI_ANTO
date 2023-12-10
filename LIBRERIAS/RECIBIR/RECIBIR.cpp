#include <Arduino.h>
#include "RECIBIR.h"

void RECIBIR:: INICIALIZAR_RADIO(){
radio.initialize();
  radio.setChannel(0);  // Frequency / Channel is *not* set in the initialization. Do it right after.
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  Serial.println(F("Waiting for signal in region defined in DavisRFM69.h"));
}
void RECIBIR:: escuchar(int stationID){
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


void RECIBIR:: processPacket() {
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
  
  #ifdef DEBUG
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
  
  #ifdef DEBUG
    Serial.print(F("Wind Direction: "));
    Serial.print(loopData.windDirection);
    Serial.print(F("  Rx Byte 2: "));
    Serial.println(radio.DATA[2]);
  #endif

  loopData.transmitterBatteryStatus = (radio.DATA[0] & 0x8) >> 3;
  #ifdef DEBUG
    Serial.print("Battery status: ");
    Serial.println(loopData.transmitterBatteryStatus);
  #endif
  canal = (radio.DATA[0] & 0x7);
  #ifdef DEBUG
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
    
  #ifdef DEBUG
      Serial.print(F("Outside Temp: "));
      Serial.print(loopData.outsideTemperature);
      Serial.print(F("  Rx Byte 3: "));
      Serial.print(radio.DATA[3]);
      Serial.print(F("  Rx Byte 4: "));
      Serial.println(radio.DATA[4]);

      Serial.print(F(  He recibido la temperatura cada ));
      Serial.println((millis()-tiempo_temp));
      tiempo_temp=millis();
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

  #ifdef DEBUG
      Serial.print("Outside Humdity: ");
      Serial.print(loopData.outsideHumidity);
      Serial.print("  Rx Byte 3: ");
      Serial.print(radio.DATA[3]);
      Serial.print("  Rx Byte 4: ");
      Serial.println(radio.DATA[4]);

      Serial.print(F(  He recibido la humedad cada ));
      Serial.println((millis()-tiempo_hum));
      tiempo_hum=millis();
  #endif

      break;
    }

  #ifdef DEBUG
    printFreeRam();
  #endif

  // Ver si se han leido todos los sensores:
  DataAll = (N_temperatura>0)&&(N_direccion>0)&&(N_velocidad)&&(N_humedad>0);

}

void RECIBIR:: print_debug_packet_info () {
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
void RECIBIR:: printStrm() {
  for (byte i = 0; i < DAVIS_PACKET_LEN; i++) {
    Serial.print(i);
    Serial.print(F(" = "));
    Serial.print(radio.DATA[i], HEX);
    Serial.print(F("\n\r"));
  }
  Serial.print(F("\n\r"));
}