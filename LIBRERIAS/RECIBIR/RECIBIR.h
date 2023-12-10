#ifndef RECIBIR_h
#define RECIBIR_h

#include <DavisRFM69.h>
#include <Davisdef.h>
#include <RFM69registers.h>

#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define SERIAL_BAUD   115200
#define PACKET_INTERVAL 2563
//#define DEBUG // debug para ver lo que recibe 
//#define POWER_SAVING //si quieres conectar al wifi

class RECIBIR{
  private:
    unsigned long lastRxTime = 0;
    unsigned long tiempo_temp = 0;
    unsigned long tiempo_hum = 0;
    
    byte hopCount = 0;
    bool goodCrc = false;
    bool pakettrue=false;
    bool strmon = false;       // Print the packet when received?
    bool DataAll=false; // Para saber si se han leido todos los sensores
    bool correctID;
    bool strpktinfo = true;   // Print packet info

    float humedad;  // %
    float temp_float; // ºC
    float temp_float_aux;
    int stationID = 0;
    int canal;
    int salto_freq;
    int tiempPacket;
    int paketrecived;
    int estado; // Variable para el estado de tu programa
    int temperatura;  
    int direccion;  // Grados (1..360)
    int velocidad;   // millas/h
    // Para promediar las lecturas
    int N_humedad=0;  
    int N_temperatura=0;
    int N_direccion=0;
    int N_velocidad=0;
    int actual_channel = 0;
    long idErrors = 0;
    int16_t goodRssi = -999;
    LoopPacket loopData;
    void processPacket();
    void print_debug_packet_info ();
    void printStrm();
  public:
    RECIBIR(); 
    DavisRFM69 radio;
    void escuchar(int stationID);
    void INICIALIZAR_RADIO();
    
};
#endif