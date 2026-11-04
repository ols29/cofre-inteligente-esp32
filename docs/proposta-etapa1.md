# Proposta de Projeto — Vault32

**Disciplina:** Conectividade em Sistemas Ciberfísicos — PUCPR  
**Título:** Vault32 — Sistema Inteligente de Controle de Acesso

## Integrantes

- Luiz
- Gustavo
- Nicolas
- Oliver
- Enzo

## 1. Problema e objetivo

O projeto Vault32 consiste no desenvolvimento de um sistema de controle de acesso físico utilizando o microcontrolador ESP32.

A solução oferece mecanismos de autenticação por senha e identificação RFID, permitindo controlar o acionamento de uma trava física e acompanhar o estado do sistema por meio de uma interface web.

O objetivo é integrar sistemas embarcados, componentes eletrônicos e comunicação em rede em uma solução funcional de controle de acesso.

## 2. Solução proposta

O ESP32 atua como controlador principal e servidor HTTP, integrando os componentes físicos e a comunicação em rede.

O sistema oferece duas formas de autenticação física:

- Digitação de senha por meio de um teclado matricial 3×3.
- Identificação de cartões ou tags autorizados por meio do módulo RFID RC522.

Uma interface web hospedada diretamente no ESP32 permite consultar o estado do sistema e solicitar operações de controle por meio de uma conexão Wi-Fi local.

## 3. Componentes e tecnologias

| Componente | Aplicação |
|---|---|
| ESP32 | Processamento, controle e conectividade |
| Teclado matricial 3×3 | Entrada de senha |
| RFID RC522 | Identificação de cartões e tags |
| Servo motor SG90 | Acionamento da trava |
| LCD 16×2 I²C | Exibição de informações |
| LED | Indicação visual do estado |

**Tecnologias utilizadas:** C++, Arduino IDE, Wi-Fi, HTTP, TCP/IP e servidor WebServer.

## 4. Funcionamento do sistema

O sistema possui três estados principais:

- **FECHADO:** condição normal, com a trava fechada e aguardando autenticação.
- **ABERTO:** condição em que o servo motor aciona a abertura da trava.
- **BLOQUEADO:** condição ativada após três tentativas inválidas de autenticação.

Após uma autenticação válida, a trava permanece aberta por aproximadamente 10 segundos e retorna automaticamente ao estado fechado.

Depois de três tentativas inválidas, o sistema realiza um bloqueio temporário de aproximadamente 30 segundos.

O desbloqueio administrativo também pode ser solicitado pela interface web.

## 5. Comunicação em rede

O ESP32 opera em modo Access Point, criando uma rede Wi-Fi própria para permitir a conexão de dispositivos externos.

| Parâmetro | Configuração |
|---|---|
| Modo | Access Point |
| Nome da rede | Vault32 |
| Endereço IP | 192.168.4.1 |
| Protocolo | HTTP |
| Porta | 80 |

A interface web utiliza as seguintes rotas:

| Rota | Método | Função |
|---|---|---|
| `/` | GET | Disponibiliza a interface web |
| `/estado` | GET | Consulta o estado do sistema |
| `/abrir?s=...` | GET | Solicita abertura mediante autenticação |
| `/desbloquear?s=...` | GET | Solicita desbloqueio administrativo |

A consulta de estado permite acompanhar a situação da trava, a quantidade de tentativas inválidas e o tempo restante de bloqueio.

## 6. Escopo e limitações

O projeto contempla autenticação física, acionamento da trava, monitoramento local e comunicação HTTP por meio do servidor hospedado no ESP32.

O firmware registra informações de funcionamento pela comunicação serial. Na versão atual, não há armazenamento persistente de eventos nem histórico de eventos disponível na interface web.

A comunicação HTTP não utiliza criptografia e o acesso à interface ocorre por meio da rede local criada pelo ESP32.

Também não estão implementados banco de dados externo, comunicação HTTPS ou notificações por serviços externos.

## 7. Melhorias futuras

- Implementar armazenamento persistente de eventos.
- Aprimorar a segurança da autenticação e da comunicação.
- Desenvolver notificações remotas.
- Melhorar a interface de controle.
- Avaliar novas possibilidades de integração de hardware.

## 8. Considerações finais

O Vault32 integra sistemas embarcados, acionamento físico e comunicação em rede. O projeto utiliza o ESP32 como controlador e servidor HTTP, oferecendo autenticação por senha e RFID, acionamento automatizado da trava e monitoramento por uma interface web.

A solução demonstra a integração entre hardware, firmware e comunicação em rede em um sistema de controle de acesso físico.
