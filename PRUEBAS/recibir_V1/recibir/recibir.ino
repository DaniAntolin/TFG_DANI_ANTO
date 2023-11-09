#include <DavisRFM69.h>
#include <Davisdef.h>
#include <RFM69registers.h>

// Sample usage of the DavisRFM69 library to sniff the packets from a Davis Instruments
// wireless Integrated Sensor Suite (ISS), demostrating compatibility between the RFM69
// and the TI CC1020 transmitter used in that hardware.  Packets received from the ISS are
// passes through to the serial port.  This code retains some of the debugging
// functionality of the LowPowerLabs Gateway sketch, on which this code is based.
//
// This is part of the DavisRFM69 library from https://github.com/dekay/DavisRFM69
// (C) DeKay 2014-2016 dekaymail@gmail.com
// Example released under the MIT License (http://opensource.org/licenses/mit-license.php)
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/
//
// This program has been developed on an ESP-12E based NodeMCU module with an
// attached RFM69W transceiver module connected as follows:
//      RFM69W      ESP-12E     NodeMCU
//      MISO        GPIO12      D6
//      MOSI        GPIO13      D7
//      SCK         GPIO14      D5
//      CS/SS       GPIO15      D8
//      DIO0        GPIO5       D1  (Interrupt)
//
// Do NOT connect the Reset of the two together!!! Reset on the ESP8266 is active LOW and on
// the RFM69 it is active HIGH.
//
//  See also https://github.com/esp8266/Arduino/blob/master/doc/reference.md
//  and https://github.com/someburner/esp-rfm69 and
//  http://www.cnx-software.com/2015/04/18/nodemcu-is-both-a-breadboard-friendly-esp8266-wi-fi-board-and-a-lua-based-firmware/

#include <SPI.h>
#include <EEPROM.h>


// NOTE: *** One of DAVIS_FREQS_US, DAVIS_FREQS_EU, DAVIS_FREQS_AU, or
// DAVIS_FREQS_NZ MUST be defined at the top of DavisRFM69.h ***

#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define SERIAL_BAUD   115200
#define PACKET_INTERVAL 2555
#define DEBUG true
boolean strmon = false;       // Print the packet when received?

int tiempPacket;
int paketrecived;
bool pakettrue=false;
int estado; // Variable para el estado de tu programa
DavisRFM69 radio;
LoopPacket loopData;

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);
  radio.initialize();
  radio.setChannel(1);              // Frequency / Channel is *not* set in the initialization. Do it right after.
  tiempPacket=((41+(1+1))*1000)/16; // millis (canal +1)
  //esp_sleep_enable_ext0_wakeup((gpio_num_t)27, LOW); // LOW indica que la interrupción ocurre en flanco descendente
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  Serial.println(F("Waiting for signal in region defined in DavisRFM69.h"));
  delay(1000);
}

unsigned long lastRxTime = 0;
unsigned long tiempo_temp = 0;
unsigned long tiempo_hum = 0;
byte hopCount = 0;

int canal;
int salto_freq;

boolean goodCrc = false;
int16_t goodRssi = -999;

void loop() {
  //process any serial input
  if (Serial.available() > 0)
  {
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

  // The check for a zero CRC value indicates a bigger problem that will need
  // fixing, but it needs to stay until the fix is in.
  // TODO Reset the packet statistics at midnight once I get my clock module.
  
  if (radio.receiveDone()) {
Serial.println(millis()-paketrecived);
    packetStats.packetsReceived++;
    unsigned int crc = radio.crc16_ccitt(radio.DATA, 6);
    if ((crc == (word(radio.DATA[6], radio.DATA[7]))) && (crc != 0)) {
      Serial.println(millis()-paketrecived);
      paketrecived=millis();
      pakettrue=true;
      processPacket();
      // This is a good packet
      goodCrc = true;
      goodRssi = radio.RSSI;
      packetStats.receivedStreak++;
      hopCount = 1;
    } else {
      Serial.println(F("No hay paquetes"));
      goodCrc = false;
      packetStats.crcErrors++;
      packetStats.receivedStreak = 0;
    }

    if (strmon) printStrm();
    #if 0
    // Debugging stuff
    Serial.print(millis());
    Serial.print(F(":  "));
    Serial.print(radio.CHANNEL);
    Serial.print(F(" - Data: "));
    for (byte i = 0; i < DAVIS_PACKET_LEN; i++) {
      Serial.print(radio.DATA[i], HEX);
      Serial.print(F(" "));
    }
    Serial.print(F("  RSSI: "));
    Serial.println(radio.RSSI);
    int freqError = radio.readReg(0x21) << 8 | radio.readReg(0x22);
    Serial.print(F("      Freq error): "));
    Serial.println(freqError);
    #endif
    // If packet was received earlier than expected, that was probably junk. Don't hop.
    // I use a simple heuristic for this.  If the CRC is bad and the received RSSI is
    // a lot less than the last known good RSSI, then don't hop.
    if (goodCrc && (radio.RSSI < (goodRssi + 15))) {
      lastRxTime = millis();
      radio.hop(); // si recibe un mensaje correcto cambia de canal, porque?
      //radio.empezar(); // salto de freq en 0 mejor RSII
      #if DEBUG
      Serial.println(millis());
      Serial.println(F(":  Hopped channel and ready to receive 1."));
      #endif
    } else {
      radio.waitHere();
      radio.empezar(); // a veces se queda pillado al estar en standby
      #if DEBUG
      Serial.print(millis());
      Serial.println(F(":  Waiting here"));
      #endif
    }
    
  

  // If a packet was not received at the expected time, hop the radio anyway
  // in an attempt to keep up.  Give up after 4 failed attempts.  Keep track
  // of packet stats as we go.  I consider a consecutive string of missed
  // packets to be a single resync.  Thx to Kobuki for this algorithm.
  if ((hopCount > 0) && ((millis() - lastRxTime) > (hopCount * PACKET_INTERVAL + 200))) {
    packetStats.packetsMissed++;
    if (hopCount == 1) packetStats.numResyncs++;
    if (++hopCount > 4) hopCount = 0;
    radio.hop();
    //radio.empezar();
#if DEBUG
    Serial.print(millis());
    Serial.println(F(":  Hopped channel and ready to receive 2."));
#endif
  }
  
 if (pakettrue){
  Serial.println("Haciendo if.");
  pakettrue=false;
  radio.sleep();
  int paketrecived2=millis()-paketrecived;
  radio.empezar();
  delay(tiempPacket-paketrecived2-400); //millis dormir entre paquetes
  //esp_deep_sleep_start();
  Serial.println("recived a true.");
  //radio._packetReceived=true;
  }
}
  
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
  loopData.windDirection = radio.DATA[2] * 360.0f / 255.0f;

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

 canal = (radio.DATA[0] & 0x7); // ,, byte 1
 salto_freq = radio.CHANNEL;
  #if 1
    Serial.print("Actual channel: ");
    Serial.println(canal);
    Serial.print("Actual salto de freq: ");
    Serial.println(salto_freq);
  #endif

  // Now look at each individual packet. The high order nibble is the packet type.
  // The highest order bit of the low nibble is set high when the ISS battery is low.
  // The low order three bits of the low nibble are the station ID.

  switch (radio.DATA[0] >> 4) {
  case VP2P_TEMP:
    loopData.outsideTemperature = (int16_t)(word(radio.DATA[3], radio.DATA[4]));
    loopData.outsideTemperature = (((loopData.outsideTemperature/160)-32)*5)/9;
    #if 1
    Serial.print(F("Outside Temp: "));
    Serial.print(loopData.outsideTemperature);
    #if 1
    Serial.print(F("  He recibido la temperatura cada: "));
    Serial.print((millis()-tiempo_temp));
    tiempo_temp=millis();
    #endif
    Serial.print(F("  Rx Byte 3: "));
    Serial.print(radio.DATA[3]);
    Serial.print(F("  Rx Byte 4: "));
    Serial.println(radio.DATA[4]);
    #endif
    break;
  case VP2P_HUMIDITY:
    loopData.outsideHumidity = (float)(word((radio.DATA[4] >> 4), radio.DATA[3])) / 10.0;
    #if 1
    Serial.print("Outside Humdity: ");
    Serial.print(loopData.outsideHumidity);
    #if 1
    Serial.print(F("  He recibido la humedad cada: "));
    Serial.print((millis()-tiempo_hum));
    tiempo_hum=millis();
    #endif
    Serial.print("  Rx Byte 3: ");
    Serial.print(radio.DATA[3]);
    Serial.print("  Rx Byte 4: ");
    Serial.println(radio.DATA[4]);
    #endif
    break;
    // default:
  }
#if 0
  printFreeRam();
#endif
}
