#include "ENVIARV1.h" 
ENVIARV1V1 ENVIAR;
void setup() 
{
  Serial.begin(115200);
  ENVIARV1.Init_BOTON ();
  tiempo_push=millis();
  Serial.println("setup");
}

void loop() 
{
  ENVIARV1.comprobarEstadoBoton_WiFi();
  if(ENVIARV1.encenderWifi){
      Serial.println("¡wifi encendido!");
    }
  if(ENVIARV1.apagarWifi){
      Serial.println("¡wifi apagado!");
    }
}
