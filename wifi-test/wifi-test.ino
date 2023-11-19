#include "WiFi.h"

int8_t indiceRede;
int intensidadeSinal;

void setup() 
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
}

void loop() 
{
  int8_t n = WiFi.scanNetworks();
  Serial.println("Escaneamento realizado");

  if (n == 0)
    Serial.println("Nenhuma rede encontrada");
  else
  {
    intensidadeSinal = -9999;
    Serial.println(String(n) + " rede(s) encontrada(s)");

    int8_t i;
    for (i = 0; i < n; i++)
    {
      Serial.println("    " + String(i) + " -> " + WiFi.SSID(i) + ": " + String(WiFi.RSSI(i)));
    
      if (WiFi.RSSI(i) > intensidadeSinal)
      {
        indiceRede = i;
        intensidadeSinal = WiFi.RSSI(i);
      }
    }
    Serial.println("REDE COM MELHOR SINAL ENCONTRADA:");
    Serial.println("    " + String(indiceRede) + " -> " + WiFi.SSID(indiceRede) + ": " + String(intensidadeSinal));
  }

  Serial.println("==============================");

  delay(5000);
}
