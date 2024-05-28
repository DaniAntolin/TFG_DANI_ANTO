#include "ENVIARV1.h" 
ENVIARV1 ENVIARV1;
void setup() 
{
  Serial.begin(115200);
  ENVIARV1.Init_BOTON ();
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
