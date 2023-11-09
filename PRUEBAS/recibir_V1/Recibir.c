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


// NOTE: *** One of DAVIS_FREQS_US, DAVIS_FREQS_EU, DAVIS_FREQS_AU, or
// DAVIS_FREQS_NZ MUST be defined at the top of DavisRFM69.h ***

#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define SERIAL_BAUD   115200
#define PACKET_INTERVAL 2555
#define DEBUG true
boolean strmon = true;       // Print the packet when received?


DavisRFM69 radio;
LoopPacket loopData;

void setup() {
  pinMode(14,OUTPUT);
  digitalWrite(14,HIGH);
  Serial.begin(SERIAL_BAUD);
  delay(10);
  radio.initialize();
  radio.setChannel(0);              // Frequency / Channel is *not* set in the initialization. Do it right after.
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  Serial.println(F("Waiting for signal in region defined in DavisRFM69.h"));
  delay(1000);
  digitalWrite(14,LOW);
}

unsigned long lastRxTime = 0;
byte hopCount = 0;

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
    packetStats.packetsReceived++;
    unsigned int crc = radio.crc16_ccitt(radio.DATA, 6);
    if ((crc == (word(radio.DATA[6], radio.DATA[7]))) && (crc != 0)) {
      processPacket();
      // This is a good packet
      goodCrc = true;
      goodRssi = radio.RSSI;
      packetStats.receivedStreak++;
      hopCount = 1;
      digitalWrite(14,HIGH);
      delay(200);
      digitalWrite(14,LOW);
    } else {
      Serial.println(F("No hay paquetes"));
      goodCrc = false;
      packetStats.crcErrors++;
      packetStats.receivedStreak = 0;
    }

    if (strmon) printStrm();
#if DEBUG
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
      radio.hop();
#if DEBUG
      Serial.print(millis());
      Serial.println(F(":  Hopped channel and ready to receive."));
#endif
    } else {
      radio.waitHere();
#if DEBUG
      Serial.print(millis());
      Serial.println(F(":  Waiting here"));
#endif
    }
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
#if DEBUG
    Serial.print(millis());
    Serial.println(F(":  Hopped channel and ready to receive."));
#endif
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
#if 1
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

#if 1
  Serial.print(F("Wind Direction: "));
  Serial.print(loopData.windDirection);
  Serial.print(F("  Rx Byte 2: "));
  Serial.println(radio.DATA[2]);
#endif

  loopData.transmitterBatteryStatus = (radio.DATA[0] & 0x8) >> 3;
#if 1
  Serial.print("Battery status: ");
  Serial.println(loopData.transmitterBatteryStatus);
#endif

  // Now look at each individual packet. The high order nibble is the packet type.
  // The highest order bit of the low nibble is set high when the ISS battery is low.
  // The low order three bits of the low nibble are the station ID.

  switch (radio.DATA[0] >> 4) {
  case VP2P_TEMP:
    loopData.outsideTemperature = (int16_t)(word(radio.DATA[3], radio.DATA[4])) >> 4;
    loopData.outsideTemperature = ((loopData.outsideTemperature-320)*5)/9;
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
#if 1
    Serial.print("Outside Humdity: ");
    Serial.print(loopData.outsideHumidity);
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
