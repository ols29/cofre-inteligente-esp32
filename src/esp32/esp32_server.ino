/*
 * ============================================================================
 * COFRE INTELIGENTE - Sistema Ciberfisico com ESP32
 * ============================================================================
 * Disciplina: Conectividade em Sistemas Ciberfisicos - PUCPR
 *
 * O ESP32 opera como SERVIDOR HTTP (porta 80) em modo Access Point.
 * Dois caminhos de autenticacao independentes:
 *   1) Teclado matricial fisico (senha local)
 *   2) Rede (requisicoes HTTP do cliente web)
 *
 * SENSOR   : Teclado matricial 4x4 (entrada de dados do mundo fisico)
 * ATUADORES: Servo SG90 (trava), buzzer, LEDs verde/vermelho, display OLED
 *
 * BIBLIOTECAS NECESSARIAS (Gerenciador de Bibliotecas da Arduino IDE):
 *   - Keypad            (Mark Stanley / Alexander Brevig)
 *   - ESP32Servo        (Kevin Harrington)
 *   - Adafruit SSD1306  (Adafruit)
 *   - Adafruit GFX      (Adafruit)
 * WiFi.h e WebServer.h ja vem com o core do ESP32.
 * ============================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------------------------------------------------------------------
// CONFIGURACAO DA REDE (modo Access Point - nao depende do Wi-Fi da faculdade)
// ---------------------------------------------------------------------------
const char* AP_SSID  = "CofreESP32";
const char* AP_SENHA = "pucpr2026";   // minimo 8 caracteres exigido pelo ESP32

WebServer server(80);                  // servidor HTTP escutando na porta 80

// ---------------------------------------------------------------------------
// MAPEAMENTO DE PINOS
// ---------------------------------------------------------------------------
#define PINO_SERVO     4
#define PINO_BUZZER    15
#define PINO_LED_VERDE 2
#define PINO_LED_VERM  5

#define OLED_LARGURA 128
#define OLED_ALTURA   64
#define OLED_ENDERECO 0x3C            // endereco I2C padrao do SSD1306

// Teclado matricial 4x4
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
// PARAMETROS DE OPERACAO
// ---------------------------------------------------------------------------
#define ANGULO_TRANCADO   0
#define ANGULO_ABERTO    90
#define MAX_TENTATIVAS    3
#define TEMPO_BLOQUEIO 30000UL         // 30 s de bloqueio apos 3 erros
#define TEMPO_ABERTO   10000UL         // trava sozinha depois de 10 s aberta
#define TAM_LOG          20            // eventos guardados no buffer circular

// ---------------------------------------------------------------------------
// MAQUINA DE ESTADOS
// ---------------------------------------------------------------------------
enum EstadoCofre { TRANCADO, DIGITANDO, ABERTO, BLOQUEADO };
EstadoCofre estado = TRANCADO;

String senhaCorreta = "1234";
String senhaDigitada = "";
int    tentativasErradas = 0;

// Marcadores de tempo (millis) - nunca usamos delay() no loop principal
unsigned long marcoBloqueio = 0;
unsigned long marcoAbertura = 0;
unsigned long marcoBuzzer   = 0;
bool          buzzerLigado  = false;

// ---------------------------------------------------------------------------
// LOG DE EVENTOS - buffer circular em RAM
// LIMITACAO CONHECIDA: os registros se perdem ao desligar o ESP32.
// Persistencia exigiria SPIFFS/EEPROM ou cartao SD (fora do escopo da Etapa 1).
// ---------------------------------------------------------------------------
struct Evento {
  unsigned long tempo;   // millis() no instante do evento
  String        tipo;    // "abertura", "fechamento", "erro", "bloqueio"
  String        origem;  // "teclado" ou "rede"
};

Evento log[TAM_LOG];
int  logInicio = 0;      // indice do evento mais antigo
int  logTotal  = 0;      // quantos eventos validos existem no buffer

void registrarEvento(String tipo, String origem) {
  int pos = (logInicio + logTotal) % TAM_LOG;
  if (logTotal == TAM_LOG) {
    // buffer cheio: sobrescreve o mais antigo e avanca o inicio
    pos = logInicio;
    logInicio = (logInicio + 1) % TAM_LOG;
  } else {
    logTotal++;
  }
  log[pos].tempo  = millis();
  log[pos].tipo   = tipo;
  log[pos].origem = origem;
}

// ---------------------------------------------------------------------------
// ACOES FISICAS
// ---------------------------------------------------------------------------
void destrancar(String origem) {
  servoTrava.write(ANGULO_ABERTO);
  digitalWrite(PINO_LED_VERDE, HIGH);
  digitalWrite(PINO_LED_VERM, LOW);
  digitalWrite(PINO_BUZZER, LOW);
  buzzerLigado = false;
  estado = ABERTO;
  marcoAbertura = millis();
  senhaDigitada = "";
  tentativasErradas = 0;
  registrarEvento("abertura", origem);
}

void trancar(String origem) {
  servoTrava.write(ANGULO_TRANCADO);
  digitalWrite(PINO_LED_VERDE, LOW);
  digitalWrite(PINO_LED_VERM, HIGH);
  digitalWrite(PINO_BUZZER, LOW);
  buzzerLigado = false;
  estado = TRANCADO;
  senhaDigitada = "";
  registrarEvento("fechamento", origem);
}

void entrarEmBloqueio() {
  estado = BLOQUEADO;
  marcoBloqueio = millis();
  marcoBuzzer = millis();
  senhaDigitada = "";
  registrarEvento("bloqueio", "teclado");
}

// ---------------------------------------------------------------------------
// DISPLAY OLED - mostra o estado no proprio equipamento
// ---------------------------------------------------------------------------
void atualizarDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(AP_SSID);
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 18);

  switch (estado) {
    case TRANCADO:
      display.println("TRANCADO");
      display.setTextSize(1);
      display.setCursor(0, 42);
      display.println("Digite a senha");
      break;

    case DIGITANDO: {
      String mascara = "";
      for (unsigned int i = 0; i < senhaDigitada.length(); i++) mascara += "*";
      display.println(mascara);
      display.setTextSize(1);
      display.setCursor(0, 42);
      display.print("# confirma  * apaga");
      break;
    }

    case ABERTO:
      display.println("ABERTO");
      display.setTextSize(1);
      display.setCursor(0, 42);
      display.print("Fecha em ");
      display.print((TEMPO_ABERTO - (millis() - marcoAbertura)) / 1000);
      display.println("s");
      break;

    case BLOQUEADO:
      display.println("BLOQUEADO");
      display.setTextSize(1);
      display.setCursor(0, 42);
      display.print("Aguarde ");
      display.print((TEMPO_BLOQUEIO - (millis() - marcoBloqueio)) / 1000);
      display.println("s");
      break;
  }

  display.setCursor(0, 56);
  display.print("Erros: ");
  display.print(tentativasErradas);
  display.print("/");
  display.print(MAX_TENTATIVAS);
  display.display();
}

// ---------------------------------------------------------------------------
// LEITURA DO TECLADO (nao bloqueante)
// ---------------------------------------------------------------------------
void lerTeclado() {
  char tecla = teclado.getKey();
  if (!tecla) return;

  // Durante o bloqueio o teclado fica inerte de proposito
  if (estado == BLOQUEADO) return;

  if (estado == ABERTO) {
    if (tecla == '#') trancar("teclado");
    return;
  }

  if (tecla == '*') {                    // apaga o que foi digitado
    senhaDigitada = "";
    estado = TRANCADO;
    return;
  }

  if (tecla == '#') {                    // confirma a senha
    if (senhaDigitada == senhaCorreta) {
      destrancar("teclado");
    } else {
      tentativasErradas++;
      registrarEvento("erro", "teclado");
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
// ROTAS HTTP
// ---------------------------------------------------------------------------
String nomeEstado() {
  switch (estado) {
    case TRANCADO:  return "trancado";
    case DIGITANDO: return "digitando";
    case ABERTO:    return "aberto";
    case BLOQUEADO: return "bloqueado";
  }
  return "desconhecido";
}

void enviarJson(int codigo, String corpo) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(codigo, "application/json", corpo);
}

void rotaStatus() {
  String json = "{";
  json += "\"estado\":\"" + nomeEstado() + "\",";
  json += "\"tentativas\":" + String(tentativasErradas) + ",";
  json += "\"maxTentativas\":" + String(MAX_TENTATIVAS) + ",";
  json += "\"digitos\":" + String(senhaDigitada.length()) + ",";
  json += "\"uptime\":" + String(millis() / 1000);
  json += "}";
  enviarJson(200, json);
}

void rotaLog() {
  String json = "[";
  for (int i = 0; i < logTotal; i++) {
    int pos = (logInicio + i) % TAM_LOG;
    if (i > 0) json += ",";
    json += "{\"t\":" + String(log[pos].tempo / 1000) + ",";
    json += "\"tipo\":\"" + log[pos].tipo + "\",";
    json += "\"origem\":\"" + log[pos].origem + "\"}";
  }
  json += "]";
  enviarJson(200, json);
}

void rotaTrava() {
  if (!server.hasArg("estado")) {
    enviarJson(400, "{\"erro\":\"parametro estado ausente\"}");
    return;
  }
  String acao = server.arg("estado");

  if (estado == BLOQUEADO) {
    enviarJson(423, "{\"erro\":\"cofre bloqueado por tentativas invalidas\"}");
    return;
  }

  if (acao == "abrir") {
    destrancar("rede");
    enviarJson(200, "{\"estado\":\"aberto\"}");
  } else if (acao == "fechar") {
    trancar("rede");
    enviarJson(200, "{\"estado\":\"trancado\"}");
  } else {
    enviarJson(400, "{\"erro\":\"valor invalido: use abrir ou fechar\"}");
  }
}

void rotaSenha() {
  if (!server.hasArg("nova")) {
    enviarJson(400, "{\"erro\":\"parametro nova ausente\"}");
    return;
  }
  String nova = server.arg("nova");
  if (nova.length() < 4 || nova.length() > 8) {
    enviarJson(400, "{\"erro\":\"a senha deve ter de 4 a 8 digitos\"}");
    return;
  }
  for (unsigned int i = 0; i < nova.length(); i++) {
    if (nova[i] < '0' || nova[i] > '9') {
      enviarJson(400, "{\"erro\":\"use apenas digitos\"}");
      return;
    }
  }
  senhaCorreta = nova;
  registrarEvento("troca-senha", "rede");
  enviarJson(200, "{\"ok\":true}");
}

void rotaNaoEncontrada() {
  enviarJson(404, "{\"erro\":\"rota inexistente\"}");
}

// Pagina cliente servida pelo proprio ESP32 (armazenada na flash via PROGMEM)
const char PAGINA[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="pt-BR"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Cofre inteligente</title><style>
*{box-sizing:border-box;margin:0}
body{background:#14161a;color:#e6e4dd;font-family:system-ui,sans-serif;padding:20px;max-width:520px;margin:0 auto}
h1{font-size:15px;font-weight:500;letter-spacing:.12em;text-transform:uppercase;color:#8a8880;margin-bottom:18px}
.painel{border:1px solid #2c2f36;border-radius:10px;padding:22px;margin-bottom:14px}
.estado{font-family:ui-monospace,monospace;font-size:34px;letter-spacing:.06em}
.aberto{color:#63c98a}.trancado{color:#e0a44a}.bloqueado{color:#e05a5a}.digitando{color:#5a9fe0}
.meta{color:#8a8880;font-size:13px;margin-top:8px}
.botoes{display:flex;gap:10px;margin-bottom:14px}
button{flex:1;padding:14px;border:1px solid #2c2f36;border-radius:8px;background:#1c1f25;
color:#e6e4dd;font-size:15px;cursor:pointer}
button:hover{border-color:#4a4e57}button:disabled{opacity:.35;cursor:not-allowed}
h2{font-size:12px;font-weight:500;letter-spacing:.1em;text-transform:uppercase;color:#8a8880;margin-bottom:10px}
li{list-style:none;font-family:ui-monospace,monospace;font-size:13px;padding:6px 0;
border-bottom:1px solid #22252b;display:flex;justify-content:space-between;color:#b8b6ae}
.vazio{color:#5c5a55;font-size:13px}
#conexao{font-size:12px;color:#8a8880;margin-top:14px}
</style></head><body>
<h1>Cofre inteligente</h1>
<div class="painel">
  <div id="estado" class="estado">--</div>
  <div class="meta" id="meta">conectando...</div>
</div>
<div class="botoes">
  <button id="btnAbrir">Abrir</button>
  <button id="btnFechar">Fechar</button>
</div>
<div class="painel">
  <h2>Registro de eventos</h2>
  <ul id="log"><li class="vazio">Nenhum evento ainda.</li></ul>
</div>
<div id="conexao">Aguardando o servidor</div>
<script>
const $=id=>document.getElementById(id);
let falhas=0;
async function atualizar(){
  try{
    const s=await(await fetch('/status')).json();
    $('estado').textContent=s.estado.toUpperCase();
    $('estado').className='estado '+s.estado;
    $('meta').textContent='Erros: '+s.tentativas+'/'+s.maxTentativas+
      '  |  Digitos: '+s.digitos+'  |  Ativo ha '+s.uptime+'s';
    $('btnAbrir').disabled=(s.estado==='aberto'||s.estado==='bloqueado');
    $('btnFechar').disabled=(s.estado!=='aberto');
    const ev=await(await fetch('/log')).json();
    $('log').innerHTML=ev.length?ev.slice().reverse().map(e=>
      '<li><span>'+e.tipo+'</span><span>'+e.origem+' &middot; '+e.t+'s</span></li>').join('')
      :'<li class="vazio">Nenhum evento ainda.</li>';
    falhas=0;$('conexao').textContent='Conectado ao ESP32';
  }catch(e){
    falhas++;
    $('conexao').textContent='Sem resposta do ESP32 ('+falhas+' tentativas). Verifique a rede CofreESP32.';
  }
}
async function comando(acao){
  try{await fetch('/trava?estado='+acao,{method:'POST'});atualizar();}
  catch(e){$('conexao').textContent='Falha ao enviar o comando. Tente novamente.';}
}
$('btnAbrir').onclick=()=>comando('abrir');
$('btnFechar').onclick=()=>comando('fechar');
atualizar();setInterval(atualizar,1000);
</script></body></html>)HTML";

void rotaRaiz() {
  server.send_P(200, "text/html", PAGINA);
}

// ---------------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_LED_VERDE, OUTPUT);
  pinMode(PINO_LED_VERM, OUTPUT);

  servoTrava.attach(PINO_SERVO);
  servoTrava.write(ANGULO_TRANCADO);
  digitalWrite(PINO_LED_VERM, HIGH);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ENDERECO)) {
    Serial.println("Falha ao iniciar o display OLED. Confira SDA/SCL e o endereco I2C.");
  }
  display.clearDisplay();
  display.display();

  // ESP32 como ponto de acesso: nao depende de roteador externo
  WiFi.softAP(AP_SSID, AP_SENHA);
  Serial.print("Rede criada: ");
  Serial.println(AP_SSID);
  Serial.print("Acesse em http://");
  Serial.println(WiFi.softAPIP());     // 192.168.4.1

  server.on("/",       HTTP_GET,  rotaRaiz);
  server.on("/status", HTTP_GET,  rotaStatus);
  server.on("/log",    HTTP_GET,  rotaLog);
  server.on("/trava",  HTTP_POST, rotaTrava);
  server.on("/senha",  HTTP_POST, rotaSenha);
  server.onNotFound(rotaNaoEncontrada);
  server.begin();

  registrarEvento("inicializacao", "sistema");
}

// ---------------------------------------------------------------------------
// LOOP PRINCIPAL
// Sem delay(): qualquer pausa bloquearia handleClient() e derrubaria o servidor.
// Toda temporizacao e feita comparando millis() com os marcos salvos.
// ---------------------------------------------------------------------------
void loop() {
  server.handleClient();      // atende requisicoes HTTP
  lerTeclado();               // le a entrada fisica

  unsigned long agora = millis();

  // Fecha sozinho depois do tempo de abertura
  if (estado == ABERTO && agora - marcoAbertura >= TEMPO_ABERTO) {
    trancar("automatico");
  }

  // Sai do bloqueio quando o tempo expira
  if (estado == BLOQUEADO) {
    if (agora - marcoBloqueio >= TEMPO_BLOQUEIO) {
      tentativasErradas = 0;
      estado = TRANCADO;
      digitalWrite(PINO_BUZZER, LOW);
      buzzerLigado = false;
    } else if (agora - marcoBuzzer >= 500) {
      // buzzer intermitente sem travar o loop
      buzzerLigado = !buzzerLigado;
      digitalWrite(PINO_BUZZER, buzzerLigado ? HIGH : LOW);
      marcoBuzzer = agora;
    }
  }

  // Redesenha o OLED a cada 200 ms para nao saturar o barramento I2C
  static unsigned long marcoDisplay = 0;
  if (agora - marcoDisplay >= 200) {
    atualizarDisplay();
    marcoDisplay = agora;
  }
}
