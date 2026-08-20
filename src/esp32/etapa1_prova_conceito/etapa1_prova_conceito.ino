/*
 * ============================================================================
 * ETAPA 1 - PROVA DE CONCEITO LOCAL (SEM REDE)
 * Cofre Inteligente - Conectividade em Sistemas Ciberfisicos - PUCPR
 * ============================================================================
 *
 * Objetivo desta etapa, conforme secao 5 do regulamento:
 * validar a leitura dos sensores e o acionamento dos atuadores SEM nenhuma
 * camada de rede. Nenhuma linha de WiFi aparece aqui de proposito.
 *
 * A camada de servidor HTTP entra apenas na Etapa 2, sobre esta base.
 *
 * SENSORES : teclado matricial 4x4 (credencial)
 *            sensor IR de obstaculo (ocupacao do compartimento)
 * ATUADORES: servo SG90 (trava), buzzer, LED verde, LED vermelho, OLED
 *
 * BIBLIOTECAS:
 *   Keypad, ESP32Servo, Adafruit SSD1306, Adafruit GFX
 * ============================================================================
 */

#include <Keypad.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------------------------------------------------------------------
// PINOS
// ---------------------------------------------------------------------------
#define PINO_SERVO     4
#define PINO_BUZZER    15
#define PINO_LED_VERDE 2
#define PINO_LED_VERM  5
#define PINO_IR        18

#define OLED_LARGURA  128
#define OLED_ALTURA    64
#define OLED_ENDERECO 0x3C

const byte LINHAS = 4;
const byte COLUNAS = 4;
char teclas[LINHAS][COLUNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pinosLinhas[LINHAS]   = {13, 12, 14, 27};
byte pinosColunas[COLUNAS] = {26, 25, 33, 32};

Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);
Servo servoTrava;
Adafruit_SSD1306 display(OLED_LARGURA, OLED_ALTURA, &Wire, -1);

// ---------------------------------------------------------------------------
// PARAMETROS
// ---------------------------------------------------------------------------
#define ANGULO_TRANCADO    0
#define ANGULO_ABERTO     90
#define MAX_TENTATIVAS     3
#define TEMPO_BLOQUEIO 30000UL
#define TEMPO_ABERTO   10000UL

enum EstadoCofre { TRANCADO, DIGITANDO, ABERTO, BLOQUEADO };
EstadoCofre estado = TRANCADO;

const String SENHA = "1234";
String senhaDigitada = "";
int    tentativasErradas = 0;

bool  ocupado     = false;

unsigned long marcoBloqueio = 0;
unsigned long marcoAbertura = 0;
unsigned long marcoBuzzer   = 0;
unsigned long marcoDisplay  = 0;
bool          buzzerLigado  = false;

// ---------------------------------------------------------------------------
// ACOES FISICAS
// ---------------------------------------------------------------------------
void destrancar() {
  servoTrava.write(ANGULO_ABERTO);
  digitalWrite(PINO_LED_VERDE, HIGH);
  digitalWrite(PINO_LED_VERM, LOW);
  estado = ABERTO;
  marcoAbertura = millis();
  senhaDigitada = "";
  tentativasErradas = 0;
  Serial.println("[EVENTO] Cofre aberto pelo teclado");
}

void trancar(const char* origem) {
  servoTrava.write(ANGULO_TRANCADO);
  digitalWrite(PINO_LED_VERDE, LOW);
  digitalWrite(PINO_LED_VERM, HIGH);
  estado = TRANCADO;
  senhaDigitada = "";
  Serial.print("[EVENTO] Cofre trancado - origem: ");
  Serial.println(origem);
}

void entrarEmBloqueio() {
  estado = BLOQUEADO;
  marcoBloqueio = millis();
  marcoBuzzer = millis();
  senhaDigitada = "";
  Serial.println("[EVENTO] Bloqueio por tentativas invalidas");
}

// ---------------------------------------------------------------------------
// LEITURA DO SENSOR DE OCUPACAO
// ---------------------------------------------------------------------------
void lerSensores() {
  // Modulo IR: saida em LOW quando ha objeto na frente
  bool ocupadoAgora = (digitalRead(PINO_IR) == LOW);
  if (ocupadoAgora != ocupado) {
    ocupado = ocupadoAgora;
    Serial.print("[SENSOR] compartimento ");
    Serial.println(ocupado ? "ocupado" : "vazio");
  }
}

// ---------------------------------------------------------------------------
// DISPLAY
// ---------------------------------------------------------------------------
void atualizarDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Etapa 1 - local");
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 16);

  switch (estado) {
    case TRANCADO:
      display.println("TRANCADO");
      display.setTextSize(1);
      display.setCursor(0, 38);
      display.println("Digite a senha");
      break;

    case DIGITANDO: {
      String mascara = "";
      for (unsigned int i = 0; i < senhaDigitada.length(); i++) mascara += "*";
      display.println(mascara);
      display.setTextSize(1);
      display.setCursor(0, 38);
      display.print("# confirma  * apaga");
      break;
    }

    case ABERTO:
      display.println("ABERTO");
      display.setTextSize(1);
      display.setCursor(0, 38);
      display.print("Fecha em ");
      display.print((TEMPO_ABERTO - (millis() - marcoAbertura)) / 1000);
      display.println("s");
      break;

    case BLOQUEADO:
      display.println("BLOQUEADO");
      display.setTextSize(1);
      display.setCursor(0, 38);
      display.print("Aguarde ");
      display.print((TEMPO_BLOQUEIO - (millis() - marcoBloqueio)) / 1000);
      display.println("s");
      break;
  }

  // Rodape com a ocupacao do compartimento
  display.setTextSize(1);
  display.setCursor(0, 48);
  display.print("Compartimento: ");
  display.print(ocupado ? "cheio" : "vazio");

  display.setCursor(0, 57);
  display.print("Erros: ");
  display.print(tentativasErradas);
  display.print("/");
  display.print(MAX_TENTATIVAS);
  display.display();
}

// ---------------------------------------------------------------------------
// LEITURA DO TECLADO - nao bloqueante
// ---------------------------------------------------------------------------
void lerTeclado() {
  char tecla = teclado.getKey();
  if (!tecla) return;

  Serial.print("[TECLA] ");
  Serial.println(tecla);

  if (estado == BLOQUEADO) return;   // teclado inerte durante o bloqueio

  if (estado == ABERTO) {
    if (tecla == '#') trancar("teclado");
    return;
  }

  if (tecla == '*') {
    senhaDigitada = "";
    estado = TRANCADO;
    return;
  }

  if (tecla == '#') {
    if (senhaDigitada == SENHA) {
      destrancar();
    } else {
      tentativasErradas++;
      Serial.println("[EVENTO] Senha incorreta");
      senhaDigitada = "";
      estado = TRANCADO;
      if (tentativasErradas >= MAX_TENTATIVAS) entrarEmBloqueio();
    }
    return;
  }

  if (tecla >= '0' && tecla <= '9' && senhaDigitada.length() < 8) {
    senhaDigitada += tecla;
    estado = DIGITANDO;
  }
}

// ---------------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Cofre Inteligente - prova de conceito local ===");
  Serial.println("Senha de teste: 1234   |   # confirma   |   * apaga");

  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_VERM, OUTPUT);
  pinMode(PINO_IR, INPUT);

  servoTrava.attach(PINO_SERVO);
  servoTrava.write(ANGULO_TRANCADO);
  digitalWrite(PINO_LED_VERM, HIGH);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ENDERECO)) {
    Serial.println("[ERRO] Display OLED nao respondeu. Confira SDA/SCL e o endereco I2C.");
  }
  display.clearDisplay();
  display.display();
}

// ---------------------------------------------------------------------------
// LOOP
// Sem delay(): a temporizacao usa millis(), preparando o codigo para a
// Etapa 2, quando o servidor HTTP passa a exigir um loop sempre livre.
// ---------------------------------------------------------------------------
void loop() {
  lerTeclado();
  lerSensores();

  unsigned long agora = millis();

  if (estado == ABERTO && agora - marcoAbertura >= TEMPO_ABERTO) {
    trancar("automatico");
  }

  if (estado == BLOQUEADO) {
    if (agora - marcoBloqueio >= TEMPO_BLOQUEIO) {
      tentativasErradas = 0;
      estado = TRANCADO;
      Serial.println("[EVENTO] Bloqueio encerrado");
    }
  }

  // Buzzer intermitente durante o bloqueio
  if (estado == BLOQUEADO) {
    if (agora - marcoBuzzer >= 500) {
      buzzerLigado = !buzzerLigado;
      digitalWrite(PINO_BUZZER, buzzerLigado ? HIGH : LOW);
      marcoBuzzer = agora;
    }
  } else if (buzzerLigado) {
    digitalWrite(PINO_BUZZER, LOW);
    buzzerLigado = false;
  }

  if (agora - marcoDisplay >= 200) {
    atualizarDisplay();
    marcoDisplay = agora;
  }
}
