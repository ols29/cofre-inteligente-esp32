# Roteiro de Apresentação — Vault32

**Disciplina:** Conectividade em Sistemas Ciberfísicos  
**Curso:** Engenharia de Software — PUCPR

## 1. Introdução e apresentação do projeto

Apresentar o Vault32, um sistema inteligente de controle de acesso físico desenvolvido com ESP32.

Explicar brevemente o problema que o projeto busca resolver: controlar o acesso a um espaço físico por meio de autenticação e gerenciamento eletrônico.

Apresentar os integrantes da equipe:
- Luiz
- Gustavo
- Nicolas
- Oliver
- Enzo

## 2. Objetivo e solução desenvolvida

Explicar que o objetivo foi desenvolver um protótipo funcional que integra:
- Sistemas embarcados.
- Autenticação por senha e RFID.
- Acionamento físico por servo motor.
- Comunicação Wi-Fi.
- Servidor HTTP e interface web.

O ESP32 funciona como unidade central de processamento e servidor da aplicação.

## 3. Arquitetura e componentes

Apresentar os componentes utilizados e suas funções:

| Componente | Função |
|---|---|
| ESP32 | Controle e comunicação |
| Teclado matricial 3x3 | Entrada de senha |
| RFID RC522 | Identificação de cartões |
| Servo SG90 | Acionamento da trava |
| LCD 16x2 I2C | Exibição de informações |
| LED | Indicação visual |

Explicar como os componentes se comunicam com o ESP32 e como o microcontrolador coordena o funcionamento do sistema.

## 4. Funcionamento do firmware

Explicar que o firmware foi desenvolvido em C++ utilizando a Arduino IDE.

Apresentar os três estados principais:

- **FECHADO:** estado normal do sistema.
- **ABERTO:** acionamento do servo após autenticação válida.
- **BLOQUEADO:** ativado após três tentativas inválidas.

Explicar as regras de funcionamento:
- Abertura por aproximadamente 10 segundos.
- Bloqueio temporário de 30 segundos após três tentativas inválidas.
- Autenticação por senha ou cartão RFID autorizado.
- Desbloqueio administrativo.

## 5. Comunicação e interface web

Demonstrar que o ESP32 cria sua própria rede Wi-Fi em modo Access Point.

Informações da rede:
- SSID: Vault32
- Endereço IP: 192.168.4.1
- Protocolo: HTTP
- Porta: 80

Apresentar as rotas disponíveis:

| Rota | Função |
|---|---|
| `/` | Interface web |
| `/estado` | Consulta do estado |
| `/abrir?s=...` | Solicitação de abertura |
| `/desbloquear?s=...` | Desbloqueio administrativo |

Explicar que a interface permite acompanhar o estado do sistema e enviar comandos remotamente.

## 6. Demonstração prática

Realizar a demonstração do protótipo em funcionamento:

1. Ligar o ESP32 e apresentar a inicialização.
2. Mostrar a rede Wi-Fi criada pelo dispositivo.
3. Conectar um celular ou computador à rede.
4. Abrir `http://192.168.4.1` no navegador.
5. Consultar o estado atual do sistema.
6. Demonstrar a autenticação pelo teclado.
7. Demonstrar a identificação de um cartão RFID autorizado.
8. Mostrar o acionamento do servo motor.
9. Demonstrar o bloqueio após três tentativas inválidas, se houver tempo disponível.
10. Mostrar o desbloqueio administrativo.

## 7. Limitações e melhorias futuras

Apresentar honestamente as limitações da implementação atual:

- Ausência de armazenamento persistente de eventos.
- Ausência de histórico de eventos na interface web.
- Necessidade de aprimorar a segurança da comunicação e autenticação.
- Possibilidade de integrar notificações e monitoramento remoto.

Também discutir a necessidade de avaliar a inclusão de um sensor ambiental para atender integralmente aos requisitos da disciplina.

## 8. Conclusão

Retomar o objetivo do projeto e destacar a integração entre hardware, firmware e comunicação em rede.

Concluir mostrando como o Vault32 utiliza o ESP32 para realizar autenticação, controlar um mecanismo físico e disponibilizar gerenciamento por uma interface web.

## Observações para a equipe

- Todos os integrantes devem conhecer a arquitetura geral do projeto.
- Cada integrante deve dominar a parte técnica pela qual é responsável.
- A demonstração deve ser testada previamente.
- Conferir alimentação, conexões, Wi-Fi e funcionamento do servo antes da apresentação.
- Não divulgar senhas administrativas ou credenciais reais durante a apresentação.
