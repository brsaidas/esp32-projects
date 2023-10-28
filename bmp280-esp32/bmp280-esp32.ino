#include <Adafruit_BMP280.h>
#include <BluetoothSerial.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth nao esta habilitado. Por favor execute 'make menuconfig' para habilita-lo
#endif

#define LED_BUILTIN 2
#define BMP280_ENDERECO 0x76

Adafruit_BMP280 sensorBMP;
BluetoothSerial SerialBT;

void callback(esp_spp_cb_event_t evento, esp_spp_cb_param_t * param)
{
  switch (evento)
  {
    case ESP_SPP_SRV_OPEN_EVT:
      digitalWrite(LED_BUILTIN, HIGH); break;
    case ESP_SPP_CLOSE_EVT:
      digitalWrite(LED_BUILTIN, LOW); break;
    default: 
      break;
  }
}

void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(9600);
  while(!Serial) delay(100);

  Serial.println("\n\n1. Serial iniciada com sucesso.");

  bool status = sensorBMP.begin(BMP280_ENDERECO);
  if (status)
  {
    Serial.println("2. BMP280 iniciado com sucesso.");
  }
  else
  {
    Serial.println("ERROR: Nao consegui encontrar um sensor BMP280 valido,"
                  "cheque os fios ou tente um endereco diferente.");
    Serial.print("SerialID foi: 0x"); Serial.println(sensorBMP.sensorID(), HEX);
    Serial.println("  ID de 0xFF provavelmente significa um endereco ruim, um BMP 180 ou BMP085.");
    Serial.println("  ID de [0x56, 0x58] representa um BMP280.");
    Serial.println("  ID de 0x60 representa um BME 280.");
    Serial.println("  ID de 0x61 representa um BME 680.");
    while (1) delay(10);
  }

  sensorBMP.setSampling(Adafruit_BMP280::MODE_NORMAL,
                        Adafruit_BMP280::SAMPLING_X2,
                        Adafruit_BMP280::SAMPLING_X16,
                        Adafruit_BMP280::FILTER_X16,
                        Adafruit_BMP280::STANDBY_MS_500);

  bool statusBT = SerialBT.begin("ESP32-GDAe");
  if(statusBT) 
  {
    Serial.println("3. Bluetooth iniciado.");
  } 
  else
  {
    Serial.println("ERROR: Nao consegui inicializar o bluetooth 'ESP32-GDAe'.");
    while (1) delay(10);
  }

  SerialBT.register_callback(callback);
}

void enviarBT(String msg)
{
  SerialBT.write('{');
  for (int i = 0; i < msg.length(); i++)
  {
    SerialBT.write(msg[i]);
  }
  SerialBT.write('}');
}

void loop() 
{
  float altura = sensorBMP.readAltitude();         // Altura em metros
  float pressao = sensorBMP.readPressure();        // Pressao em Pascal
  float temperatura = sensorBMP.readTemperature(); // Temperatura em graus Celsius

  Serial.println(String(altura) + " " + String(pressao) + " " + String(temperatura));

  enviarBT("A" + String(altura));
  enviarBT("P" + String(pressao));
  enviarBT("T" + String(temperatura));

  delay(2000);
}
