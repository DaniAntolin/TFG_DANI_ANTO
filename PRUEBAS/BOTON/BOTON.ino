#include "ENVIAR.h" 
ENVIAR ENVIAR;

int pinBoton = 12; // Pin GPIO donde está conectado el botón
int PINTIERRA= 13;
bool has_interrupted = false;
bool encendido= false;
unsigned long tiempo_push = 0;
void setup() 
{
  Serial.begin(115200);
  ENVIAR.Init_BOTON ();
  tiempo_push=millis();
  Serial.println("setup");
}

void loop() 
{
  comprobarEstadoBoton_WiFi();
  if(ENVIAR.encenderWifi){
      Serial.println("¡wifi encendido!");
    }
  if(ENVIAR.apagarWifi){
      Serial.println("¡wifi apagado!");
    }
}
