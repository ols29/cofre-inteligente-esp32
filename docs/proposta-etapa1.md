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

O projeto Vault32 propõe o desenvolvimento de um sistema de controle de acesso físico utilizando o microcontrolador ESP32.

A solução busca oferecer uma alternativa de autenticação e controle de acesso por meio de senha e identificação RFID, permitindo também o monitoramento do estado do sistema e o envio de comandos através de uma interface web.

## 2. Solução proposta

O ESP32 atua como controlador principal e servidor HTTP, integrando os componentes físicos e a comunicação em rede.

O sistema oferece duas formas de autenticação física:

- Digitação de senha por meio de um teclado matricial 3x3.
- Identificação de cartões ou tags autorizados por meio do módulo RFID RC522.

Uma interface web hospedada no próprio ESP32 permite consultar o estado do sistema e solicitar operações de controle.

## 3. Componentes e tecnologias

| Componente | Aplicação |
|---|---|
| ESP32 | Processamento e conectividade |
| Teclado matricial 3x3 | Entrada de senha |
| RFID RC522 | Identificação de cartões e tags |
| Servo motor SG90 | Acionamento da trava |
| LCD 16x2 I2C | Exibição de informações |
| LED | Indicação visual do estado |

**Tecnologias:** C++, Arduino IDE, Wi-Fi, HTTP e servidor WebServer.

## 4. Funcionamento do sistema

O sistema possui três estados principais:

- **FECHADO:** condição normal, com a trava fechada.
- **ABERTO:** condição em que o servo motor aciona a abertura.
- **BLOQUEADO:** condição ativada após três tentativas inválidas.

Após uma autenticação válida, a trava permanece aberta por aproximadamente 10 segundos e retorna automaticamente ao estado fechado.

Depois de três tentativas inválidas, o sistema realiza um bloqueio temporário de 30 segundos.

O desbloqueio administrativo também pode ser solicitado pela interface web.

## 5. Comunicação em rede

O ESP32 opera em modo Access Point, criando uma rede Wi-Fi própria.

| Parâmetro | Configuração |
|---|---|
| Modo | Access Point |
| Nome da rede | Vault32 |
| Endereço IP | 192.168.4.1 |
| Protocolo | HTTP |
| Porta | 80 |

A interface web utiliza as seguintes rotas:

- `GET /`: disponibiliza a interface web.
- `GET /estado`: consulta o estado do sistema.
- `GET /abrir?s=...`: solicita abertura mediante autenticação.
- `GET /desbloquear?s=...`: solicita desbloqueio administrativo.

A consulta de estado permite acompanhar a situação da trava, as tentativas inválidas e o tempo restante de bloqueio.

## 6. Escopo e limitações

O projeto contempla autenticação física, acionamento da trava, monitoramento local e comunicação HTTP por meio do servidor hospedado no ESP32.

O firmware registra informações de funcionamento pela comunicação serial. Não há, na versão atual, armazenamento persistente de eventos nem histórico de eventos disponível na interface web.

Também não estão implementados banco de dados externo, comunicação HTTPS ou notificações por serviços externos.

## 7. Melhorias futuras

- Implementar armazenamento persistente de eventos.
- Aprimorar a segurança da autenticação e da comunicação.
- Desenvolver notificações remotas.
- Melhorar a interface de controle.
- Avaliar a integração de sensores ambientais, conforme os requisitos da disciplina.

## 8. Considerações finais

O Vault32 integra sistemas embarcados, acionamento físico e comunicação em rede. O projeto utiliza o ESP32 como controlador e servidor HTTP, oferecendo autenticação por senha e RFID, além de gerenciamento por uma interface web.
