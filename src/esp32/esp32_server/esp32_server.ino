#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Adafruit_Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

// ===== CONFIGURAÇÕES =====
const char* SENHA        = "4567";     // use só 0,4,5,6,7,8,9
const char* SENHA_ADMIN  = "9090";     // desbloqueia pelo navegador
const char* WIFI_SSID    = "Vault32";
const char* WIFI_PASS    = "vault1234";
const int   MAX_ERROS    = 3;
const unsigned long TEMPO_ABERTO   = 10000;  // 10 s
const unsigned long TEMPO_BLOQUEIO = 30000;  // 30 s
const int ANG_FECHADO = 0;
const int ANG_ABERTO  = 90;

// UIDs autorizados (formato: bytes em HEX, maiúsculos, separados por espaço)
// Só o cartão branco abre. A tag azul (8B 1A BE 06) fica de fora de propósito: é negada.
const char* UIDS_AUTORIZADOS[] = {
  "84 17 D8 06"    // cartão branco (autorizado)
};
const int N_UIDS = sizeof(UIDS_AUTORIZADOS) / sizeof(UIDS_AUTORIZADOS[0]);

// ===== PINOS =====
const int PINO_LED   = 12;
const int PINO_SERVO = 13;
const int PINO_SDA   = 21;
const int PINO_SCL   = 22;
const int RFID_SS    = 5;
const int RFID_RST   = 4;
const int RFID_SCK   = 18;
const int RFID_MISO  = 19;
const int RFID_MOSI  = 23;

const byte LINHAS = 3, COLUNAS = 3;
char teclas[LINHAS][COLUNAS] = {
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte pinosLinhas[LINHAS]   = {32, 14, 27};
byte pinosColunas[COLUNAS] = {26, 25, 33};

Adafruit_Keypad teclado = Adafruit_Keypad(
  makeKeymap(teclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);
Servo servo;
WebServer server(80);
LiquidCrystal_I2C *lcd = nullptr;
MFRC522 rfid(RFID_SS, RFID_RST);
bool rfidOk = false;

enum Estado { FECHADO, ABERTO, BLOQUEADO };
Estado estado = FECHADO;
int erros = 0;
unsigned long tAberto = 0, tBloqueio = 0;
String digitado = "";

const int NLOG = 8;
String logs[NLOG];
int nlogs = 0;

String ultimaL1 = "", ultimaL2 = "";
String msgL1 = "", msgL2 = "";
unsigned long tMsg = 0;

void registrar(String msg) {
  String linha = String(millis() / 1000) + "s: " + msg;
  if (nlogs < NLOG) logs[nlogs++] = linha;
  else { for (int i = 1; i < NLOG; i++) logs[i-1] = logs[i]; logs[NLOG-1] = linha; }
  Serial.println(linha);
}

String pad16(String s) {
  while (s.length() < 16) s += ' ';
  return s.substring(0, 16);
}

void mostrarTemp(String a, String b) {
  msgL1 = a; msgL2 = b; tMsg = millis() + 2000;
}

void iniciarLCD() {
  Wire.begin(PINO_SDA, PINO_SCL);
  byte enderecos[2] = {0x27, 0x3F};
  for (int i = 0; i < 2; i++) {
    Wire.beginTransmission(enderecos[i]);
    if (Wire.endTransmission() == 0) {
      lcd = new LiquidCrystal_I2C(enderecos[i], 16, 2);
      lcd->init();
      lcd->backlight();
      Serial.print("LCD encontrado em 0x");
      Serial.println(enderecos[i], HEX);
      return;
    }
  }
  Serial.println("LCD nao encontrado (o Vault32 segue sem display)");
}

void iniciarRFID() {
  SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
  rfid.PCD_Init();
  delay(50);
  byte v = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  rfidOk = (v != 0x00 && v != 0xFF);
  Serial.print("RC522 versao: 0x");
  Serial.println(v, HEX);
  if (!rfidOk) Serial.println("RC522 nao respondeu (o Vault32 segue sem RFID)");
}

void desenharLCD() {
  if (!lcd) return;
  String l1, l2;
  if (millis() < tMsg) {
    l1 = msgL1;
    l2 = msgL2;
  } else if (millis() < 3000) {
    l1 = "Vault32";
    l2 = "192.168.4.1";
  } else if (estado == ABERTO) {
    long r = (TEMPO_ABERTO - (millis() - tAberto)) / 1000 + 1;
    l1 = "ACESSO LIBERADO";
    l2 = "Fecha em " + String(r) + "s";
  } else if (estado == BLOQUEADO) {
    long r = (TEMPO_BLOQUEIO - (millis() - tBloqueio)) / 1000 + 1;
    l1 = "BLOQUEADO";
    l2 = "Aguarde " + String(r) + "s";
  } else {
    l1 = "Senha ou cartao";
    l2 = "";
    for (unsigned int i = 0; i < digitado.length(); i++) l2 += '*';
    if (erros > 0) l2 += " E" + String(erros) + "/" + String(MAX_ERROS);
  }
  l1 = pad16(l1);
  l2 = pad16(l2);
  if (l1 != ultimaL1) { lcd->setCursor(0, 0); lcd->print(l1); ultimaL1 = l1; }
  if (l2 != ultimaL2) { lcd->setCursor(0, 1); lcd->print(l2); ultimaL2 = l2; }
}

void abrir(String origem) {
  estado = ABERTO; erros = 0; tAberto = millis(); tMsg = 0;
  servo.write(ANG_ABERTO);
  digitalWrite(PINO_LED, HIGH);
  registrar("ABERTO (" + origem + ")");
}

void fechar() {
  estado = FECHADO;
  servo.write(ANG_FECHADO);
  digitalWrite(PINO_LED, LOW);
  registrar("FECHADO");
}

void errar(String origem) {
  erros++;
  registrar("Acesso negado (" + origem + ") " + String(erros) + "/" + String(MAX_ERROS));
  if (erros >= MAX_ERROS) {
    estado = BLOQUEADO; tBloqueio = millis();
    servo.write(ANG_FECHADO);
    digitalWrite(PINO_LED, LOW);
    registrar("BLOQUEADO");
  }
}

void desbloquear(String origem) {
  estado = FECHADO; erros = 0;
  registrar("DESBLOQUEADO (" + origem + ")");
}

String nomeEstado() {
  if (estado == ABERTO) return "ABERTO";
  if (estado == BLOQUEADO) return "BLOQUEADO";
  return "FECHADO";
}

bool uidAutorizado(String uid) {
  for (int i = 0; i < N_UIDS; i++) {
    if (uid == String(UIDS_AUTORIZADOS[i])) return true;
  }
  return false;
}

void lerRFID() {
  if (!rfidOk) return;
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    String h = String(rfid.uid.uidByte[i], HEX);
    h.toUpperCase();
    if (h.length() < 2) h = "0" + h;
    if (i > 0) uid += " ";
    uid += h;
  }
  Serial.println("UID: " + uid);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  if (estado == BLOQUEADO) {
    mostrarTemp("BLOQUEADO", "Cartao ignorado");
    return;
  }
  if (uidAutorizado(uid)) {
    abrir("cartao");
  } else {
    mostrarTemp("Cartao negado", uid);
    errar("cartao");
  }
  desenharLCD();
}

const char PAGINA[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Vault32</title>
<style>
body{font-family:sans-serif;max-width:420px;margin:20px auto;padding:0 12px;text-align:center}
#st{font-size:2em;font-weight:bold;padding:14px;border-radius:10px;color:#fff;background:#888}
input,button{font-size:1.1em;padding:10px;margin:6px;width:90%}
pre{text-align:left;background:#eee;padding:8px;font-size:.85em}
</style></head><body>
<h2>Vault32</h2>
<div id="st">...</div>
<p id="inf"></p>
<input id="s" type="password" placeholder="Senha">
<button onclick="acao('abrir')">Abrir</button>
<button onclick="acao('desbloquear')">Desbloquear (senha admin)</button>
<p id="msg"></p>
<h4>Histórico</h4><pre id="log"></pre>
<script>
function acao(a){
  fetch('/'+a+'?s='+encodeURIComponent(document.getElementById('s').value))
   .then(r=>r.text()).then(t=>{document.getElementById('msg').innerText=t;atualiza();});
}
function atualiza(){
  fetch('/estado').then(r=>r.json()).then(d=>{
    var e=document.getElementById('st');
    e.innerText=d.estado;
    e.style.background=d.estado=='ABERTO'?'#2a9d3a':(d.estado=='BLOQUEADO'?'#c0392b':'#555');
    document.getElementById('inf').innerText='Erros: '+d.erros+'/'+d.max+(d.restante>0?'  |  '+d.restante+'s restantes':'');
    document.getElementById('log').innerText=d.log.join('\n');
  }).catch(()=>{});
}
setInterval(atualiza,1000);atualiza();
</script></body></html>
)rawliteral";

void hEstado() {
  long restante = 0;
  if (estado == ABERTO) restante = (TEMPO_ABERTO - (millis() - tAberto)) / 1000 + 1;
  if (estado == BLOQUEADO) restante = (TEMPO_BLOQUEIO - (millis() - tBloqueio)) / 1000 + 1;
  String j = "{\"estado\":\"" + nomeEstado() + "\",\"erros\":" + String(erros) +
             ",\"max\":" + String(MAX_ERROS) + ",\"restante\":" + String(restante) + ",\"log\":[";
  for (int i = 0; i < nlogs; i++) { if (i) j += ","; j += "\"" + logs[i] + "\""; }
  j += "]}";
  server.send(200, "application/json", j);
}

void hAbrir() {
  if (estado == BLOQUEADO) { server.send(200, "text/plain", "Sistema bloqueado."); return; }
  if (server.arg("s") == SENHA) { abrir("web"); server.send(200, "text/plain", "Aberto!"); }
  else { errar("web"); server.send(200, "text/plain", "Senha errada."); }
}

void hDesbloquear() {
  if (server.arg("s") == SENHA_ADMIN) { desbloquear("web"); server.send(200, "text/plain", "Desbloqueado."); }
  else server.send(200, "text/plain", "Senha admin errada.");
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_LED, OUTPUT);
  servo.setPeriodHertz(50);
  servo.attach(PINO_SERVO, 500, 2400);
  servo.write(ANG_FECHADO);
  teclado.begin();
  iniciarLCD();
  iniciarRFID();

  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  server.on("/", []() { server.send_P(200, "text/html", PAGINA); });
  server.on("/estado", hEstado);
  server.on("/abrir", hAbrir);
  server.on("/desbloquear", hDesbloquear);
  server.begin();

  Serial.println("Vault32 iniciado");
  Serial.print("Wi-Fi: "); Serial.print(WIFI_SSID);
  Serial.print("  IP: "); Serial.println(WiFi.softAPIP());
  registrar("Sistema iniciado");
  desenharLCD();
}

void loop() {
  server.handleClient();

  teclado.tick();
  while (teclado.available()) {
    keypadEvent e = teclado.read();
    if (e.bit.EVENT != KEY_JUST_PRESSED) continue;
    char k = (char)e.bit.KEY;
    Serial.print("Tecla: "); Serial.println(k);
    if (estado == BLOQUEADO) continue;
    if (k == '*') digitado = "";
    else if (k == '#') {
      if (digitado == SENHA) abrir("teclado"); else errar("teclado");
      digitado = "";
    } else if (digitado.length() < 8) digitado += k;
    desenharLCD();
  }

  lerRFID();

  if (estado == ABERTO && millis() - tAberto >= TEMPO_ABERTO) fechar();
  if (estado == BLOQUEADO && millis() - tBloqueio >= TEMPO_BLOQUEIO) desbloquear("tempo");

  static unsigned long tLcd = 0;
  if (millis() - tLcd > 300) { tLcd = millis(); desenharLCD(); }
  delay(5);
}
