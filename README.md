# Cofre Inteligente — Sistema Ciberfísico com ESP32

Trava eletrônica com dois caminhos de autenticação independentes (teclado físico e rede local) e registro de eventos consultável remotamente.

**Disciplina:** Conectividade em Sistemas Ciberfísicos
**Curso:** Engenharia de Software — PUCPR

## Problema

Armários, cofres e salas técnicas trancados por chave física não deixam rastro. Não há registro de quem abriu, em que horário, nem quantas tentativas malsucedidas ocorreram antes. Também não existe forma de liberar o acesso a distância quando alguém precisa entrar e o responsável pela chave não está no local.

Este projeto demonstra, em escala de protótipo, uma trava eletrônica que resolve os três pontos: autentica por senha local, aceita comando remoto pela rede e mantém um registro dos eventos.

## Integrantes da equipe

| Nome completo | RA |
|---|---|
| *(preencher)* | *(preencher)* |
| *(preencher)* | *(preencher)* |
| *(preencher)* | *(preencher)* |

## Arquitetura da solução

```
        +-----------------------------------+
        |        APLICAÇÃO CLIENTE          |
        |   Navegador (celular ou notebook) |
        +-----------------+-----------------+
                          |
              Requisições | Respostas
               / Comandos | JSON
                          v
+-------------------------+-----------------------------+
|                    ESP32 (SERVIDOR)                   |
|          Servidor HTTP escutando na porta 80          |
|                                                       |
|     Leitura |                        | Acionamento    |
|             v                        v                |
|   +--------------------+   +-------------------------+|
|   | Teclado 4x4        |   | Servo SG90 (trava)      ||
|   | (entrada de senha) |   | Buzzer, LEDs, OLED      ||
|   +--------------------+   +-------------------------+|
+-------------------------------------------------------+
```

O ESP32 opera em modo **Access Point**: cria a própria rede Wi-Fi e não depende de roteador externo nem de acesso à internet.

- **Rede:** `CofreESP32`
- **Senha:** `pucpr2026`
- **Endereço:** `http://192.168.4.1`

## Hardware utilizado

| Componente | Função | Preço aprox. |
|---|---|---|
| ESP32 NodeMCU (WROOM-32) | Servidor e controle | R$ 45 |
| Teclado matricial 4x4 | Entrada de senha local | R$ 15 |
| Display OLED 0.96" I2C (SSD1306) | Estado no equipamento | R$ 30 |
| Servo motor SG90 | Trava mecânica | R$ 15 |
| Buzzer ativo 5V | Alerta de bloqueio | R$ 4 |
| LED verde + LED vermelho + 2 resistores 220Ω | Indicação visual | R$ 3 |
| Protoboard 400 pontos + jumpers | Montagem | R$ 30 |
| **Total** | | **~R$ 142** |

### Diagrama de conexões (pinagem)

| Componente | Pino do ESP32 | Observação |
|---|---|---|
| Teclado — linhas R1 a R4 | GPIO 13, 12, 14, 27 | |
| Teclado — colunas C1 a C4 | GPIO 26, 25, 33, 32 | |
| OLED — SDA | GPIO 21 | I2C, endereço `0x3C` |
| OLED — SCL | GPIO 22 | |
| OLED — VCC / GND | 3V3 / GND | |
| Servo — sinal (laranja) | GPIO 4 | |
| Servo — VCC / GND | 5V / GND | GND comum com o ESP32 |
| Buzzer (+) | GPIO 15 | |
| LED verde | GPIO 2 | Resistor 220Ω em série |
| LED vermelho | GPIO 5 | Resistor 220Ω em série |

**Cuidados de montagem:**

- Os GPIO 6 a 11 são usados pela memória flash interna e não podem ser utilizados.
- O servo SG90 provoca um pico de corrente ao iniciar o movimento. Se o ESP32 reiniciar sozinho durante o acionamento, alimente o servo por fonte externa de 5V mantendo o **GND comum** com a placa.
- Se o display não inicializar, confirme o endereço I2C com um sketch de varredura — alguns módulos usam `0x3D`.

## Documentação da API

Todas as respostas são JSON. O cabeçalho `Access-Control-Allow-Origin: *` está habilitado para permitir testes com o cliente rodando fora da placa.

### `GET /`

Serve a página de controle embarcada na flash do ESP32.

### `GET /status`

Retorna o estado atual do sistema.

```json
{
  "estado": "trancado",
  "tentativas": 1,
  "maxTentativas": 3,
  "digitos": 0,
  "uptime": 482
}
```

O campo `estado` assume os valores `trancado`, `digitando`, `aberto` ou `bloqueado`.

### `GET /log`

Retorna os últimos 20 eventos, do mais antigo para o mais recente.

```json
[
  { "t": 12,  "tipo": "inicializacao", "origem": "sistema" },
  { "t": 45,  "tipo": "erro",          "origem": "teclado" },
  { "t": 51,  "tipo": "abertura",      "origem": "rede"    }
]
```

### `POST /trava?estado=abrir` · `POST /trava?estado=fechar`

Aciona o servo. Retorna `423 Locked` se o cofre estiver em estado de bloqueio.

```json
{ "estado": "aberto" }
```

### `POST /senha?nova=4321`

Troca a senha do teclado. Aceita de 4 a 8 dígitos numéricos.

```json
{ "ok": true }
```

### Códigos de erro

| Código | Situação |
|---|---|
| `400` | Parâmetro ausente ou valor inválido |
| `404` | Rota inexistente |
| `423` | Cofre bloqueado por tentativas inválidas |

## Máquina de estados

```
                  senha correta
   TRANCADO ---------------------------> ABERTO
      ^  |                                 |
      |  | dígito                          | 10 s ou comando "fechar"
      |  v                                 |
      | DIGITANDO                          |
      |    |                               |
      |    | senha errada (3x)             |
      |    v                               |
      +-- BLOQUEADO <----------------------+
             (30 s, buzzer intermitente)
```

Três senhas incorretas consecutivas levam ao estado `BLOQUEADO`, no qual o teclado fica inerte e o buzzer apita em intervalos de 500 ms. O bloqueio também recusa comandos vindos da rede.

## Instruções de instalação e execução

### 1. Firmware do ESP32

Instale a Arduino IDE e adicione o suporte ao ESP32 em **Arquivo → Preferências → URLs adicionais para gerenciadores de placas**:

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

Depois instale, pelo Gerenciador de Bibliotecas:

- `Keypad` (Mark Stanley / Alexander Brevig)
- `ESP32Servo` (Kevin Harrington)
- `Adafruit SSD1306`
- `Adafruit GFX Library`

Selecione a placa **ESP32 Dev Module**, abra `src/esp32/esp32_server.ino` e envie para a placa.

### 2. Acesso ao sistema

1. Conecte o celular ou notebook à rede Wi-Fi `CofreESP32` (senha `pucpr2026`).
2. Abra `http://192.168.4.1` no navegador.

### 3. Cliente de desenvolvimento (opcional)

A pasta `src/client/` contém a mesma interface como arquivos separados, útil para editar o layout sem recompilar o firmware. Abra `index.html` no navegador com o computador conectado à rede do cofre.

### 4. Testes por linha de comando

```bash
curl http://192.168.4.1/status
curl http://192.168.4.1/log
curl -X POST "http://192.168.4.1/trava?estado=abrir"
curl -X POST "http://192.168.4.1/trava?estado=fechar"
curl -X POST "http://192.168.4.1/senha?nova=4321"
```

## Uso do teclado

| Tecla | Ação |
|---|---|
| `0` a `9` | Digita a senha (máximo 8 dígitos) |
| `#` | Confirma a senha, ou tranca o cofre quando aberto |
| `*` | Apaga o que foi digitado |

Senha padrão de fábrica: `1234`.

## Decisões de projeto

**Por que HTTP e não TCP ou UDP.** A interação é de requisição e resposta, sem fluxo contínuo: o cliente pergunta o estado, o servidor responde. HTTP entrega isso sem nenhuma camada extra, permite testar cada rota com `curl` e dispensa aplicativo próprio — qualquer navegador serve de cliente. TCP faria sentido se houvesse necessidade de conexão persistente com notificação do servidor para o cliente; UDP, se houvesse telemetria contínua em que perder um pacote fosse tolerável.

**Por que Access Point e não Station.** Em modo AP o sistema funciona em qualquer lugar, sem depender de credenciais de rede de terceiros nem de um roteador disponível. Isso também reproduz o cenário real de uma trava instalada em local sem infraestrutura de rede.

**Ausência de `delay()` no loop.** Qualquer pausa bloqueante impediria `server.handleClient()` de rodar, e o cliente veria a página travar. Toda a temporização (bloqueio de 30 s, fechamento automático de 10 s, intermitência do buzzer, atualização do OLED) é feita comparando `millis()` com marcos salvos.

**Atualização do display a cada 200 ms.** Redesenhar o OLED a cada volta do loop saturaria o barramento I2C e reduziria a responsividade do servidor sem ganho perceptível.

## Limitações conhecidas

- **O registro de eventos vive na RAM.** Os 20 eventos do buffer circular se perdem ao desligar a placa. Persistência exigiria SPIFFS, EEPROM ou cartão SD.
- **A senha também é volátil.** Ao reiniciar, volta ao padrão `1234`.
- **Sem criptografia.** O tráfego é HTTP puro, e a senha da rota `/senha` transita na query string. Um sistema em produção exigiria HTTPS e autenticação por token.
- **A trava é simbólica.** O servo SG90 não tem torque para uma fechadura real; ele demonstra o acionamento, não a resistência mecânica.

Essas limitações são conscientes e delimitadas pelo escopo da disciplina.

## Vídeo demonstrativo

*(inserir link do vídeo mostrando: digitação da senha no teclado, abertura pelo navegador, bloqueio após três erros e fechamento automático)*

## Estrutura do repositório

```
cofre-inteligente-esp32/
├── docs/
│   ├── arquitetura.png
│   ├── esquematico.png
│   └── maquina-estados.png
├── src/
│   ├── esp32/
│   │   └── esp32_server.ino
│   └── client/
│       ├── index.html
│       └── app.js
├── README.md
└── .gitignore
```

## Uso de inteligência artificial

Conforme a seção 9 do regulamento da disciplina, a equipe registra que utilizou ferramentas de IA generativa como apoio na estruturação da documentação e no esclarecimento de dúvidas conceituais. Todo o código foi revisado, testado no hardware físico e é compreendido integralmente pelos integrantes, que se responsabilizam por ele na defesa técnica.

*(Ajustar este parágrafo conforme o uso real da equipe.)*
