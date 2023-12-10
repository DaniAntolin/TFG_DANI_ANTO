#include "ENVIARV1.h" 
ENVIARV1 ENVIARV1;
#define POWER_SAVING
#difine NUM_ESTACIONES 3
int estaciones[NUM_ESTACIONES]={0,1,2}; // Tres estaciones, dentro el stationID entre 0 y 7 (-1, no usar esa estación)

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(100);
  GUARDAR.INICIALIZAR_GUARDAR ();
  if (1){   // JALO: AHORA SERVIDOR SIEMPRE INICIADO (para ver su funcionamiento)
  
  ENVIARV1.wifi_ON(); // Modo arranque wifi con WiFi.h
  ENVIARV1.conectarServidor();
 
  } //Servidor
  

} // Fin Setup()

int cont=0;
int T=10000; //milisegundos
uint8_t N_clientes;

//===========================================================================
void loop() {
  process_serial_commands();
  // Imprimir Info 
  if (1) print_variables();  
  Serial.println(F("--------------------------------"));
  cont=cont+(T/1000);
  Serial.print(F("Segundos: "));   Serial.print(cont);
  delay(T); 
}
