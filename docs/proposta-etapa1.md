# Proposta de tema: Etapa 1

**Disciplina:** Conectividade em Sistemas Ciberfísicos, PUCPR
**Título:** Cofre inteligente com autenticação local e controle em rede

## Integrantes

Gustavo, Luiz, Nicolas, Oliver.

## As três perguntas da seção 3.1

**1. Que problema real podemos monitorar com um sensor?**

Armários e cofres trancados por chave física não registram nada. Não se sabe quem abriu, quando, nem quantas tentativas malsucedidas houve antes, nem em que condições o conteúdo está guardado. O teclado matricial monitora a entrada de credencial e um sensor infravermelho de obstáculo indica se há algo armazenado.

**2. Que ação corretiva podemos disparar com um atuador?**

O travamento e destravamento por servo motor, o bloqueio temporário após tentativas repetidas, e o estado exibido em display OLED no próprio equipamento, com LEDs indicando a situação da trava.

**3. Como os dados e comandos serão trocados pela rede?**

O ESP32 opera como servidor HTTP na porta 80, em modo Access Point. A aplicação cliente consulta o estado e o registro de eventos, e envia comandos de travamento, tudo por rotas REST em JSON.

## Escopo

**Dentro do escopo:**

- Autenticação por senha em teclado matricial 4x4
- Trava por servo motor com fechamento automático temporizado
- Bloqueio de 30 segundos após três tentativas incorretas
- Detecção de ocupação do compartimento por sensor infravermelho
- Display OLED com o estado local e a ocupação do compartimento
- Servidor HTTP no ESP32 em modo Access Point
- Interface web para consulta de estado, envio de comandos e visualização do registro de eventos
- Registro dos últimos 20 eventos em memória, com origem e instante

**Fora do escopo (declarado deliberadamente):**

- Persistência em banco de dados ou serviço externo
- Placa de circuito impresso dedicada
- HTTPS e autenticação por token
- Cartão de aproximação

## Referências consultadas

O projeto foi desenvolvido pela equipe. Os repositórios abaixo foram consultados como referência de arquitetura e boas práticas de montagem, sem reaproveitamento de código:

- Hotsunlok/ESP32-smart-door-system: organização da documentação e ideia do temporizador visível de fechamento automático
- lamkhanhnha353/IoT_smart-locker: validação independente do mapeamento de pinos, lógica de entrada de PIN e uso de DHT11 com alarme térmico
- MobiXaiph/Door-lock-System-with-Keypad: validação da prova de conceito local com teclado, servo e realimentação visual e sonora
- RohanKini18/ESP32-Servo-Motor-controller-Captive-portal: operação em modo Access Point sem roteador e cuidados de alimentação do servo

Dois projetos foram consultados como contraexemplo deliberado. Em ambos o ESP32 atua apenas como cliente de um serviço externo, o que não atende ao requisito da seção 4.3 do regulamento:

- lamkhanhnha353/IoT_smart-locker: telemetria por MQTT em broker público
- Dubemchukwu/Iot-Door-Lock-System: backend hospedado em nuvem intermediando o painel

## Cronograma

| Etapa | Entrega |
|---|---|
| 1. Planejamento e arquitetura | Proposta, repositório, diagrama, prova de conceito local |
| 2. Comunicação e firmware servidor | Servidor HTTP no ESP32, rotas validadas via cURL |
| 3. Integração final | Interface cliente, testes de falha, apresentação |

## Repositório

https://github.com/ols29/cofre-inteligente-esp32
