#define ANALOG_PIN_0 4 // PINO 4 - ENTRADA DO SINAL DE ECG

const unsigned long INTERVALO_US = 4000; // Mantém o filtro rodando exatamente a 250 Hz
unsigned long tempoAnterior = 0;

// Controle de velocidade na tela:
// 4 = envia 62,5 pontos/seg (ideal para ver 2 a 3 batimentos na tela)
// 5 = envia 50 pontos/seg (mais lento ainda, parecido com o seu gráfico anterior, mas liso)
const int FATOR_EXIBICAO = 4; 
int contadorAmostras = 0;

// Filtro Notch 60 Hz (IIR Biquad para fs = 250 Hz)
float notch_x1 = 0, notch_x2 = 0, notch_y1 = 0, notch_y2 = 0;
const float nb0 = 0.9695, nb1 = -0.1217, nb2 = 0.9695;
const float na1 = -0.1217, na2 = 0.9391;

// Filtro Passa-Baixa Butterworth 2ª Ordem - Corte em 40 Hz (fs = 250 Hz)
float lp_x1 = 0, lp_x2 = 0, lp_y1 = 0, lp_y2 = 0;
const float lb0 = 0.1551, lb1 = 0.3101, lb2 = 0.1551;
const float la1 = -0.6202, la2 = 0.2404;

void setup()
{
  Serial.begin(230400);
}

void loop()
{
  unsigned long tempoAtual = micros();

  if (tempoAtual - tempoAnterior >= INTERVALO_US)
  {
    tempoAnterior = tempoAtual;

    // 1. Leitura bruta do ADC no Pino 4
    float entrada = (float)analogRead(ANALOG_PIN_0);

    // 2. Aplica Filtro Notch 60 Hz
    float saidaNotch = nb0 * entrada + nb1 * notch_x1 + nb2 * notch_x2 - na1 * notch_y1 - na2 * notch_y2;
    notch_x2 = notch_x1;
    notch_x1 = entrada;
    notch_y2 = notch_y1;
    notch_y1 = saidaNotch;

    // 3. Aplica Filtro Passa-Baixa Butterworth 40 Hz
    float saidaFinal = lb0 * saidaNotch + lb1 * lp_x1 + lb2 * lp_x2 - la1 * lp_y1 - la2 * lp_y2;
    lp_x2 = lp_x1;
    lp_x1 = saidaNotch;
    lp_y2 = lp_y1;
    lp_y1 = saidaFinal;

    // 4. Envia apenas 1 a cada FATOR_EXIBICAO amostras para desacelerar o gráfico
    contadorAmostras++;
    if (contadorAmostras >= FATOR_EXIBICAO)
    {
      contadorAmostras = 0;
      Serial.println(saidaFinal);
    }
  }
}
