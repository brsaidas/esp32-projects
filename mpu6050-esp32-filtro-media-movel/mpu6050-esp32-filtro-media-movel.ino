/*
 Teste do sensor MPU6050 com o microcontrolador ESP32 e 
 filtragem dos dados pelo filtro Média Móvel.

 OBS: Nesse código só utilizamos um dados (aceleracao no eixo x)
 mas o código pode ser facilmente alterado para todos os 7 dados que
 o sensor pode nos enviar.

  Testado: Não
*/

#include <Wire.h> // Biblioteca para o protocolo de cominicação I2C

#define MPU_endereco 0x68 // Endereço do sensor

#define TAMANHO_FILTRO 50 // Quantidade de amostras consideradas pelo filtro

// Variável para armazenar a leitura do valor da aceleração no 
// eixo x feita pelo sensor
int16_t ax, axf; 

// Protótipos das funções
void iniciarMPU();
void leituraMPU();
void leituraFiltroMPU();

void setup() 
{
  delay(10000); // Atraso para limpar Monitor Serial e preparar teste

  Serial.begin(115200); // Iniciando porta Serial

  iniciarMPU(); // Rotina para iniciar sensor
}

void loop() {
  leituraFiltroMPU(); // Lendo o valor do sensor com o filtro de Média Móvel

  // Para obter o valor em m/s^2 é necessário fazer a conversão de sinal analógico
  // para aceleração (esse sensor está calibrado em +/- 2g)
  Serial.print("ax = ");
  Serial.print(axf);
}

void iniciarMPU()
{
  Wire.begin(); // Inicia a comunicação I2C
  Wire.beginTransmission(MPU_endereco); // Começa a transmitir os dados para o sensor
  Wire.write(0x6B); // Registrador (PWR_MGMT_1)
  Wire.write(0); // Manda 0 e "acorda" o MPU6050
  Wire.endTransmission(true); // Finaliza a transmição
}

void leituraMPU()
{
  Wire.beginTransmission(MPU_endereco); // Começa a transmitir os dados para o sensor
  Wire.write(0x3B); // Registrador dos dados medidos (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_endereco, 14, true); // Faz um "pedido" para ler 14 registradores, 
  // que serão os registrados com os dados medidos

  ax = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
}

/*
  Função que implementa a Média Móvel.
  Média Móvel: é uma tecnica de filtragem de sinal que dinimui o ruído existente nos dados
  brutos do sensor. Foi implementada a Média Móvel Simples a qual utiliza a média das últimas
  N medidas para calcular o valor real mais provével.
*/
void leituraFiltroMPU()
{
  static int i = 0, j = TAMANHO_FILTRO - 1; // Algoritmo de Two-Pointers (i, j)
  static int16_t aux[TAMANHO_FILTRO], soma = 0; // Vetor com valores intermediários e soma com somas intermediárias

  leituraMPU(); // Leitura bruta do sensor (possivelmente com ruído de sinal)

  axf = soma + ax - aux[i]; // Atualização da soma dos N = TAMANHO_FILTRO últimas medidadas

  aux[j] = ax; // Adicionando a mais nova medida na soma
  j++; j %= TAMANHO_FILTRO; // Atualizando j, note que é um vetor circular
  i++; i %= TAMANHO_FILTRO; // Atualizando j, note que é um vetor circular
  soma = axf; // Atualizando a soma
}
