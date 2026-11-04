# Checklist — Planejamento, desenvolvimento e validação do Vault32

Este documento registra o planejamento inicial, a organização do repositório e as atividades de desenvolvimento e validação do projeto Vault32.

O checklist foi atualizado para refletir a implementação atual do protótipo físico, preservando a distinção entre a prova de conceito inicial e a versão final desenvolvida.

## 1. Identificação do projeto

- **Projeto:** Vault32 — Sistema de controle de acesso com ESP32
- **Curso:** Engenharia de Software — PUCPR
- **Disciplina:** Conectividade em Sistemas Ciberfísicos
- **Integrantes:** Luiz, Gustavo, Nicolas, Oliver e Enzo
- **Plataforma principal:** ESP32
- **Linguagem:** C++ (Arduino IDE)

## 2. Planejamento inicial — Etapa 1

A primeira proposta utilizava uma arquitetura diferente da implementação atual, com teclado 4×4, sensor infravermelho, display OLED e outros componentes.

Essa configuração pertence ao planejamento e à prova de conceito inicial, não devendo ser confundida com o hardware da versão atual.

- [ ] Registrar os dados dos integrantes e respectivos RAs.
- [ ] Registrar os prazos e a data de aprovação do professor.
- [ ] Enviar a proposta pelo Canvas.
- [ ] Preservar os documentos e registros da etapa inicial.

Documento relacionado: `docs/proposta-etapa1.md`.

## 3. Repositório e organização

Repositório oficial: https://github.com/ols29/cofre-inteligente-esp32

Estrutura atual:

```text
cofre-inteligente-esp32/
├── docs/
│   ├── apresentacao-roteiro.md
│   ├── arquitetura.svg
│   ├── checklist-etapa1.md
│   └── proposta-etapa1.md
├── src/
│   └── esp32/
│       ├── esp32_server/
│       │   └── esp32_server.ino
│       └── etapa1_prova_conceito/
│           └── etapa1_prova_conceito.ino
├── .gitignore
└── README.md
```

- [ ] Conferir se todos os arquivos estão no diretório correto.
- [ ] Verificar se o firmware principal está atualizado.
- [ ] Manter a prova de conceito inicial identificada separadamente.
- [ ] Verificar se não existem arquivos antigos ou não utilizados na estrutura.
- [ ] Conferir as contribuições e os commits dos integrantes.
- [ ] Garantir que credenciais e dados sensíveis não estejam expostos no repositório público.

## 4. Hardware da implementação atual

A versão atual utiliza os seguintes componentes:

| Componente | Função |
|---|---|
| ESP32 | Processamento, controle e servidor HTTP |
| Teclado matricial 3×3 | Entrada de senha |
| Leitor RFID RC522 | Identificação por cartão ou tag |
| Display LCD 16×2 I²C | Exibição de mensagens e estados |
| Servo motor SG90 | Acionamento da trava |
| LED | Indicação visual do funcionamento |

Observações:

- O LCD atual utiliza comunicação I²C e endereço `0x27`.
- O leitor RFID realiza a identificação de cartões e tags autorizados.
- O servo realiza a abertura e o fechamento da trava.
- A interface web é disponibilizada diretamente pelo firmware do ESP32.

Checklist:

- [ ] Conferir os componentes utilizados no protótipo físico.
- [ ] Verificar as conexões e a alimentação elétrica.
- [ ] Conferir o funcionamento individual dos componentes.
- [ ] Registrar fotografias da montagem atual.
- [ ] Documentar eventuais alterações realizadas durante a montagem.

**Atenção:** preços, fornecedores e pinagens da configuração inicial não devem ser reutilizados como se fossem dados da implementação atual. A pinagem definitiva deve ser conferida diretamente no firmware.

## 5. Firmware e controle de acesso

Arquivo principal:

`src/esp32/esp32_server/esp32_server.ino`

O firmware é desenvolvido em C++ utilizando o ambiente Arduino IDE.

Funcionalidades implementadas:

- [ ] Inicialização dos componentes conectados ao ESP32.
- [ ] Leitura do teclado matricial.
- [ ] Leitura e validação de cartões RFID.
- [ ] Autenticação por senha.
- [ ] Controle do servo motor.
- [ ] Exibição de informações no LCD.
- [ ] Indicação visual por LED.
- [ ] Gerenciamento dos estados do sistema.
- [ ] Controle de tentativas inválidas.
- [ ] Bloqueio temporário após três tentativas inválidas.
- [ ] Fechamento automático após o período de abertura.

### Estados do sistema

| Estado | Descrição |
|---|---|
| FECHADO | Sistema bloqueado, aguardando autenticação |
| ABERTO | Acesso autorizado e trava acionada |
| BLOQUEADO | Acesso temporariamente impedido após tentativas inválidas |

### Temporizações implementadas

- Abertura: aproximadamente 10 segundos.
- Bloqueio: aproximadamente 30 segundos após três tentativas inválidas.

## 6. Comunicação e servidor HTTP

O ESP32 funciona como servidor HTTP e cria sua própria rede Wi-Fi em modo Access Point.

| Configuração | Valor |
|---|---|
| Rede Wi-Fi | Vault32 |
| Endereço IP | `192.168.4.1` |
| Porta HTTP | `80` |
| Protocolo | HTTP sobre TCP |

### Rotas disponíveis

| Rota | Método | Função |
|---|---|---|
| `/` | GET | Disponibiliza a interface web |
| `/estado` | GET | Retorna informações do estado do sistema em JSON |
| `/abrir?s=...` | GET | Tenta realizar a abertura utilizando senha |
| `/desbloquear?s=...` | GET | Solicita o desbloqueio administrativo |

Checklist de validação:

- [ ] ESP32 cria corretamente a rede Wi-Fi.
- [ ] Dispositivo externo consegue se conectar à rede.
- [ ] Interface web abre pelo endereço IP do ESP32.
- [ ] Rota `/estado` retorna as informações esperadas.
- [ ] Rota `/abrir` realiza a solicitação de abertura.
- [ ] Rota `/desbloquear` realiza a solicitação administrativa.
- [ ] Interface atualiza as informações de estado.
- [ ] Comunicação permanece funcional durante a demonstração.

## 7. Interface web

A interface web está incorporada ao próprio firmware C++ e é enviada pelo servidor HTTP do ESP32.

Não existe uma aplicação independente em `src/client/` na implementação atual.

Funcionalidades:

- [ ] Disponibilização da interface pelo ESP32.
- [ ] Consulta periódica do estado do sistema.
- [ ] Exibição dos estados FECHADO, ABERTO e BLOQUEADO.
- [ ] Exibição das informações de tentativas e bloqueio.
- [ ] Solicitação de abertura.
- [ ] Solicitação de desbloqueio administrativo.

A interface realiza consultas periódicas à rota `/estado`, aproximadamente uma vez por segundo.

## 8. Testes integrados do protótipo

Realizar os testes com o protótipo físico ligado.

### Autenticação

- [ ] Digitar uma senha válida e verificar a autorização.
- [ ] Aproximar um cartão RFID autorizado e verificar a autorização.
- [ ] Testar uma senha inválida.
- [ ] Testar um cartão RFID não autorizado.
- [ ] Conferir a indicação das mensagens no LCD.

### Acionamento e estados

- [ ] Verificar a abertura da trava após autenticação válida.
- [ ] Verificar o fechamento automático após aproximadamente 10 segundos.
- [ ] Realizar três tentativas inválidas.
- [ ] Confirmar o estado BLOQUEADO.
- [ ] Confirmar o encerramento do bloqueio após aproximadamente 30 segundos.

### Interface e comunicação

- [ ] Conectar um celular ou computador à rede Vault32.
- [ ] Acessar a interface pelo navegador.
- [ ] Conferir a atualização dos estados.
- [ ] Testar os comandos de abertura e desbloqueio pela interface.
- [ ] Comparar o estado apresentado na interface com o comportamento físico do protótipo.

## 9. Registro e evidências

- [ ] Fotografar o protótipo físico montado.
- [ ] Registrar os componentes e suas conexões.
- [ ] Registrar a inicialização do ESP32.
- [ ] Fotografar a interface web em funcionamento.
- [ ] Gravar um vídeo demonstrando a autenticação.
- [ ] Gravar um vídeo demonstrando a comunicação HTTP.
- [ ] Registrar os testes de bloqueio e abertura automática.
- [ ] Organizar as evidências na pasta `docs/`.
- [ ] Atualizar a documentação conforme os resultados dos testes.

## 10. Limitações conhecidas e melhorias futuras

As seguintes limitações devem ser consideradas na avaliação técnica:

- A comunicação HTTP não utiliza criptografia.
- O servidor funciona em uma rede Wi-Fi local criada pelo próprio ESP32.
- Os eventos são enviados pela comunicação Serial, sem armazenamento persistente de histórico.
- A interface web não apresenta um histórico persistente de eventos.
- O sistema não possui integração com Telegram ou notificações externas.
- O protótipo atual utiliza RFID para identificação, mas não possui um sensor ambiental implementado.

**Requisito a verificar:** o regulamento da disciplina exige a utilização de pelo menos um sensor para coleta de uma variável ambiental e um atuador. O leitor RFID não substitui esse requisito. É necessário confirmar com o professor se a configuração atual é aceita ou planejar a inclusão de um sensor ambiental.

## 11. Preparação para apresentação e defesa

- [ ] Apresentar a arquitetura do sistema.
- [ ] Explicar a função do ESP32.
- [ ] Explicar a autenticação por senha e RFID.
- [ ] Explicar os estados do sistema.
- [ ] Demonstrar o funcionamento do servo e do LCD.
- [ ] Explicar a criação da rede Wi-Fi Access Point.
- [ ] Explicar o funcionamento do servidor HTTP e suas rotas.
- [ ] Demonstrar comandos em tempo real.
- [ ] Explicar as limitações e possíveis melhorias.
- [ ] Garantir que todos os integrantes conheçam a implementação e suas respectivas contribuições.

---

**Observação final:** este checklist documenta a arquitetura e as funcionalidades identificadas na implementação atual. Os itens marcados devem ser conferidos e concluídos pela equipe; a presença de uma funcionalidade na documentação não significa, por si só, que seu teste ou validação já foi realizado.
