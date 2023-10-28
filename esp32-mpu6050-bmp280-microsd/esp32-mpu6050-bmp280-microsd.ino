#include "FS.h"
#include "SD.h"
#include "SPI.h"
// #include <Adafruit_BMP280.h> // Biblioteca BMP280
#include <Adafruit_MPU6050.h> // Biblioteca MPU6050
#include <BluetoothSerial.h> // Biblioteca bluetooth

// Verificando se o hardware da placa possui bluetooth
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth nao esta habilitado. Por favor execute 'make menuconfig' para habilita-lo
#endif

#define LED_BUILTIN 2 // LED embutido na placa
// #define BMP280_ENDERECO 0x76 // Endereco I2C do BMP280
#define MPU6050_ENDERECO 0x68 // Endereco I2C do MPU6050
#define TAMANHO_FILTRO_ACELERACAO 50

// Adafruit_BMP280 bmp; // Objeto para manipular BMP280
Adafruit_MPU6050 mpu; // Objeto para manipular BMP6050
BluetoothSerial SerialBT; // Objeto para manipular Serial

// ========== Variaveis Globais ==========
float mediaAceleracaoZ = 0;
float filtroAceleracaoZ[TAMANHO_FILTRO_ACELERACAO];

sensors_event_t mpuAceleracao, mpuGiro, mpuTemperatura;

// ========== Prototipos das funcoes ==========
void printBT(String msg);
void callback(esp_spp_cb_event_t evento, esp_spp_cb_param_t * param);
float aceleracaoZ();
void appendFile(fs::FS &fs, const char * path, const char * message);

void setup() 
{
  delay(10000);

  pinMode(LED_BUILTIN, OUTPUT); // Configurando pino do LED embutido
  digitalWrite(LED_BUILTIN, LOW); // Iniciando como desconectado (LOW)

  // ========== Iniciando a comunicacao serial ==========
  Serial.begin(115200); 
  while (!Serial) delay(100);
  Serial.println("Serial iniciada com sucesso.");
  // ========== Fim da configuracao da comunicacao serial ==========

  /*
  // Iniciando conexao ao BMP280
  if (bmp.begin(BMP280_ENDERECO)) Serial.println("BMP280 iniciado com sucesso.");
  else
  {
    Serial.println("ERROR: Nao consegui encontrar um sensor BMP280 valido,"
                  "cheque os fios ou tente um endereco diferente.");
    Serial.print("SerialID foi: 0x"); Serial.println(bmp.sensorID(), HEX);
    Serial.println("  ID de 0xFF provavelmente significa um endereco ruim, um BMP 180 ou BMP085.");
    Serial.println("  ID de [0x56, 0x58] representa um BMP280.");
    Serial.println("  ID de 0x60 representa um BME 280.");
    Serial.println("  ID de 0x61 representa um BME 680.");
    while (1) delay(10);
  }

  // Configuracoes do BMP
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
  */

  // ========== Iniciando o bluetooth interno da ESP32. ==========
  if (SerialBT.begin("ESP32GDAe")) Serial.println("Bluetooth iniciado.");
  else
  {
    Serial.println("ERROR: Nao consegui inicializar o bluetooth 'ESP32-GDAe'.");
    while (1) delay(10);
  }

  // Registrando funcao callback
  SerialBT.register_callback(callback);
  // ========== Fim da configuracao do bluetooth ==========

  // ========== Iniciando MPU6050 ==========
  if (!mpu.begin())
  {
    Serial.println("Falha ao encontrar o chip MPU6050");
    while(1) delay(10); 
  }
  else Serial.println("MPU6050 encontrado");

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  Serial.print(" > Faixa do acelerometro: ");
  switch (mpu.getAccelerometerRange())
  {
    case MPU6050_RANGE_2_G: Serial.println("+-2g"); break;
    case MPU6050_RANGE_4_G: Serial.println("+-4g"); break;
    case MPU6050_RANGE_8_G: Serial.println("+-8g"); break;
    case MPU6050_RANGE_16_G: Serial.println("+-16g"); break;
    default: Serial.println("nao encontrado"); break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
  Serial.print(" > Largura de banda do filtro: ");
  switch (mpu.getFilterBandwidth())
  {
    case MPU6050_BAND_260_HZ: Serial.println("260 Hz"); break;
    case MPU6050_BAND_184_HZ: Serial.println("184 Hz"); break;
    case MPU6050_BAND_94_HZ: Serial.println("94 Hz"); break;
    case MPU6050_BAND_44_HZ: Serial.println("44 Hz"); break;
    case MPU6050_BAND_21_HZ: Serial.println("21 Hz"); break;
    case MPU6050_BAND_10_HZ: Serial.println("10 Hz"); break;
    case MPU6050_BAND_5_HZ: Serial.println("5 Hz"); break;
    default: Serial.println("nao encontrado"); break;
  }

  // Coletanto primeiros dados para o filtro
  for (char i = 0; i < TAMANHO_FILTRO_ACELERACAO; i++)
  {
    mpu.getEvent(&mpuAceleracao, &mpuGiro, &mpuTemperatura);
    filtroAceleracaoZ[i] = mpuAceleracao.acceleration.z;
    mediaAceleracaoZ += filtroAceleracaoZ[i];
  }
  // ========== Fim da configuracao do MPU6050 ==========

  // ========== Iniciando cartoa SD ==========
  if (!SD.begin(5))
  {
    Serial.println("Cartao nao montado");
    while (1) delay(10);
  }

  uint8_t tipoCartao = SD.cardType();

  if (tipoCartao == CARD_NONE)
  {
    Serial.println("Cartao SD nao foi inserido");
    while (1) delay(10);
  }

  Serial.print("SD Card Type: ");
  if(tipoCartao == CARD_MMC){
    Serial.println("MMC");
  } else if(tipoCartao == CARD_SD){
    Serial.println("SDSC");
  } else if(tipoCartao == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Cartao SD de tamanho: %lluMB\n", cardSize);

  // ========== Fim da configuracao do cartao SD ==========

  delay(10000); // Pausa para iniciar leituras dos dados
}

void loop() 
{
  Serial.println("Az: " + String(aceleracaoZ()) + " m/s^2");

  String msg = "Az: " + String(aceleracaoZ()) + " m/s^2\n";

  appendFile(SD, "/output.txt", msg.c_str());
}

/*
  Funcao de callback. 
  OBJETIVO: acionar (HIGH) o LED embutido na placa quando
  a mesma estiver conectada com o bluetooth e desligar (LOW)
  caso contrario
*/
void callback(esp_spp_cb_event_t evento, esp_spp_cb_param_t * param)
{
  switch (evento)
  {
    case ESP_SPP_SRV_OPEN_EVT: digitalWrite(LED_BUILTIN, HIGH); break;
    case ESP_SPP_CLOSE_EVT: digitalWrite(LED_BUILTIN, LOW); break;
    default: break;
  }
}

// Funcao para enviar mensaguens via bluetooth
void printBT(String msg)
{
  SerialBT.write('{');
  for (char i = 0; i < msg.length(); i++) SerialBT.write(msg[i]);
  SerialBT.write('}');
}

// Funcao para obter aceleracao no eixo z com filtro
float aceleracaoZ()
{
  static char inicio = 0;

  mpu.getEvent(&mpuAceleracao, &mpuGiro, &mpuTemperatura);
  float novaMedida = mpuAceleracao.acceleration.z;

  float novaMedia = mediaAceleracaoZ - filtroAceleracaoZ[inicio] + novaMedida;

  mediaAceleracaoZ = novaMedia;
  filtroAceleracaoZ[inicio] = novaMedida;

  inicio++; inicio %= TAMANHO_FILTRO_ACELERACAO;

  return novaMedia/((float) TAMANHO_FILTRO_ACELERACAO);
}


void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}