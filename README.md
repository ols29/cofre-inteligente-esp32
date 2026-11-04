# Vault32 — Sistema Inteligente de Controle de Acesso

Projeto desenvolvido para a disciplina de **Conectividade em Sistemas Ciberfísicos**, do curso de Engenharia de Software da PUCPR.

O Vault32 é um sistema de controle de acesso físico baseado no microcontrolador ESP32. O projeto combina autenticação local por senha e identificação RFID com uma interface web para monitoramento e acionamento remoto da trava.

## Equipe

- Luiz
- Gustavo
- Nicolas
- Oliver
- Enzo

## Objetivo do projeto

Desenvolver um protótipo de controle de acesso que integre hardware, firmware e comunicação em rede, permitindo autenticação, monitoramento do estado do sistema e acionamento de um mecanismo físico.

## Funcionalidades

- Autenticação por senha utilizando teclado matricial 3x3.
- Identificação de cartões ou tags RFID autorizados.
- Controle de abertura e fechamento por servo motor.
- Indicação visual do estado por LED.
- Exibição de informações no display LCD 16x2.
- Criação de uma rede Wi-Fi própria pelo ESP32.
- Interface web para consulta do estado e envio de comandos.
- Bloqueio temporário após três tentativas inválidas.
- Desbloqueio administrativo.

## Componentes utilizados

| Componente | Função |
|---|---|
| ESP32 | Unidade central de processamento e comunicação |
| Teclado matricial 3x3 | Entrada de senha |
| RFID RC522 | Identificação de cartões e tags |
| Servo motor SG90 | Acionamento da trava |
| LCD 16x2 I2C | Exibição de informações |
| LED | Indicação visual do estado |

## Arquitetura do sistema

O ESP32 atua como controlador principal e servidor web.

O sistema possui dois meios de interação:

1. **Interação física:** realizada pelo teclado matricial e pelo leitor RFID.
2. **Interação remota:** realizada por meio de uma interface web hospedada no próprio ESP32.

O microcontrolador cria uma rede Wi-Fi própria, permitindo que dispositivos conectados a ela acessem a interface de controle.

## Conectividade

O ESP32 opera em modo Access Point (AP), criando uma rede Wi-Fi chamada `Vault32`.

| Parâmetro | Configuração |
|---|---|
| Modo de operação | Wi-Fi Access Point |
| SSID | Vault32 |
| Endereço IP | 192.168.4.1 |
| Servidor | HTTP |
| Porta | 80 |

Após conectar um dispositivo à rede criada pelo ESP32, a interface pode ser acessada pelo navegador através do endereço:

`http://192.168.4.1`

## Interface web e rotas HTTP

A aplicação web permite consultar o estado do sistema e enviar comandos de controle.

| Rota | Método | Descrição |
|---|---|---|
| `/` | GET | Exibe a interface web |
| `/estado` | GET | Retorna informações do estado atual em JSON |
| `/abrir?s=...` | GET | Solicita abertura mediante autenticação |
| `/desbloquear?s=...` | GET | Solicita desbloqueio administrativo |

A interface consulta periodicamente a rota `/estado` para atualizar as informações exibidas.

## Firmware e lógica de funcionamento

O firmware foi desenvolvido em C++ utilizando a Arduino IDE e bibliotecas para integração dos módulos.

O sistema trabalha com três estados principais:

- **FECHADO:** estado normal, com a trava fechada.
- **ABERTO:** estado em que o servo aciona a abertura.
- **BLOQUEADO:** estado ativado após três tentativas inválidas.

Após uma autenticação válida, o servo é acionado e a abertura permanece ativa por aproximadamente 10 segundos, retornando automaticamente ao estado fechado.

Quando são registradas três tentativas inválidas, o sistema entra em bloqueio temporário por 30 segundos.

A autenticação pode ocorrer por senha no teclado ou por identificação de um cartão RFID autorizado.

## Registro e monitoramento

O firmware utiliza a comunicação serial para apresentar informações e eventos de funcionamento durante a execução.

A interface web disponibiliza a consulta do estado atual, quantidade de tentativas inválidas e tempo restante de bloqueio, quando aplicável.

## Tecnologias e bibliotecas

- C++
- Arduino IDE
- ESP32
- Wi-Fi
- HTTP
- WebServer
- ESP32Servo
- Adafruit Keypad
- LiquidCrystal_I2C
- MFRC522
- SPI
- Wire

## Estrutura do projeto

```text
cofre-inteligente-esp32/
│
├── docs/
│   ├── apresentacao-roteiro.md
│   └── proposta-etapa1.md
│
├── src/
│   └── esp32/
│       ├── esp32_server/
│       │   └── esp32_server.ino
│       │
│       └── etapa1_prova_conceito/
│           └── etapa1_prova_conceito.ino
│
├── .gitignore
└── README.md
```

## Limitações e melhorias futuras

- Implementar armazenamento persistente de eventos.
- Melhorar a segurança da autenticação e da comunicação HTTP.
- Implementar registro de eventos acessível pela interface web.
- Adicionar mecanismos de monitoramento e notificações remotas.
- Aprimorar a interface e a experiência de utilização.

## Considerações finais

O Vault32 demonstra a integração entre sistemas embarcados, controle de hardware e comunicação em rede. O projeto utiliza o ESP32 como controlador e servidor, oferecendo autenticação física e gerenciamento remoto por meio de uma interface web.
