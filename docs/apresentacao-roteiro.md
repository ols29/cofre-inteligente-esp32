# Roteiro de apresentação: Cofre Inteligente

Material para montar os slides e para a fala da equipe. Cada bloco traz o conteúdo do slide e, logo abaixo, o roteiro (o que falar). Duração alvo: 8 a 10 minutos.

Divisão sugerida entre os integrantes:
- Slides 1 a 3: Oliver (abertura, problema, solução)
- Slides 4 a 6: Gustavo (arquitetura, hardware, funcionamento)
- Slides 7 a 8: Luiz (rede e segurança)
- Slides 9 a 12: Nicolas (prova de conceito, melhorias, cronograma, encerramento)

---

## Slide 1: Capa

Conteúdo:
- Título: Cofre Inteligente, sistema ciberfísico com ESP32
- Subtítulo: autenticação local e controle em rede
- Integrantes: Gustavo, Luiz, Nicolas, Oliver
- Disciplina: Conectividade em Sistemas Ciberfísicos, PUCPR

Roteiro: "Boa tarde. Somos o grupo do Cofre Inteligente. Nosso projeto é uma trava eletrônica que autentica por senha no próprio equipamento e também aceita comando pela rede, registrando tudo o que acontece."

---

## Slide 2: O problema

Conteúdo:
- Armários, cofres e salas trancados por chave física não deixam rastro
- Não se sabe quem abriu, quando, nem quantas tentativas houve
- Não há como liberar o acesso a distância

Roteiro: "Uma fechadura comum não registra nada. Se algo some de um armário, não há histórico de quem abriu nem quando. E se alguém precisa entrar e alguém está longe com a chave, não existe uma forma simples de liberar. É esse conjunto de problemas que a gente ataca."

---

## Slide 3: A solução

Conteúdo:
- Trava eletrônica com dois caminhos de autenticação: teclado físico e rede local
- Registro de eventos consultável
- Protótipo funcional com ESP32

Roteiro: "A solução resolve três pontos: autentica por senha local no teclado, aceita comando remoto pela rede, e mantém um registro dos eventos. Tudo em um protótipo de baixo custo baseado no ESP32."

---

## Slide 4: Arquitetura

Conteúdo:
- Diagrama: navegador (cliente) sobre HTTP e Wi-Fi local, ESP32 como servidor na porta 80, sensores e atuadores
- Usar a imagem docs/arquitetura.svg

Roteiro: "Na arquitetura, o ESP32 é o servidor. Ele cria a própria rede Wi-Fi em modo Access Point e sobe um servidor HTTP na porta 80. Qualquer navegador vira cliente: pede o estado, consulta o registro e envia comandos. Do outro lado, o ESP32 lê o teclado e o sensor de ocupação e aciona a trava, o buzzer, os LEDs e o display."

---

## Slide 5: Hardware

Conteúdo:
- ESP32 NodeMCU, teclado matricial 4x4, display OLED SSD1306
- Servo SG90 (trava), buzzer, LED verde e vermelho, sensor IR de ocupação
- Custo aproximado: R$ 154

Roteiro: "O hardware é acessível, na faixa de cento e cinquenta reais. O cérebro é o ESP32. A entrada de senha é o teclado 4x4, a trava é o servo, o display OLED mostra o estado, os LEDs e o buzzer dão o retorno, e um sensor infravermelho indica se o compartimento está ocupado."

---

## Slide 6: Como funciona

Conteúdo:
- Máquina de estados: TRANCADO, DIGITANDO, ABERTO, BLOQUEADO
- Senha correta abre; fecha sozinho depois de 10 segundos
- Três erros seguidos levam a bloqueio de 30 segundos com buzzer

Roteiro: "O funcionamento segue uma máquina de estados. Digitando a senha certa, o cofre abre e volta a trancar sozinho depois de dez segundos. Se erra três vezes seguidas, entra em bloqueio por trinta segundos, com o buzzer apitando e o teclado inerte. Toda a temporização usa millis, sem travar o loop, o que já prepara o código para o servidor da etapa 2."

---

## Slide 7: Comunicação em rede

Conteúdo:
- HTTP em modo Access Point, sem depender de roteador ou internet
- Respostas em JSON (rotas REST)
- Rotas: GET /status, GET /log, POST /trava, POST /senha

Roteiro: "A comunicação é feita por rotas REST em JSON. Escolhemos HTTP porque a interação é de pergunta e resposta, sem fluxo contínuo, e porque dá para testar cada rota com o comando cURL. O status devolve o estado atual, o log traz os últimos eventos, e há rotas para acionar a trava e trocar a senha."

---

## Slide 8: Segurança e limitações

Conteúdo:
- Bloqueio por tentativas (lockout) no teclado e na rede
- Limitações conscientes: tráfego HTTP puro, senha e registro voláteis, trava simbólica
- Ponto de atenção: as rotas ainda não exigem autenticação (será corrigido nas melhorias)

Roteiro: "Sobre segurança, já temos o bloqueio por tentativas. Mas somos honestos com as limitações do protótipo: o tráfego é HTTP puro, a senha e o registro vivem na memória e se perdem ao desligar, e o servo é simbólico. O ponto mais importante para a próxima etapa é que as rotas ainda não pedem autenticação, e é justamente isso que as melhorias resolvem."

---

## Slide 9: Prova de conceito (Etapa 1)

Conteúdo:
- Firmware etapa1_prova_conceito.ino roda sem rede
- Valida sensores e atuadores de forma isolada
- Base para o servidor HTTP da etapa 2

Roteiro: "Nesta etapa entregamos a prova de conceito local, sem rede nenhuma. Ela valida a leitura do teclado e do sensor e o acionamento da trava, do display e do buzzer. É a base sobre a qual a camada de servidor entra na etapa 2."

---

## Slide 10: Melhorias planejadas

Conteúdo:
- Cadastro de usuários com papéis (admin e comum)
- Administração: reset de senha, criar e remover usuários, gerenciar funções
- Usuários temporários com validade e liberação por horário
- Interface web protegida por autenticação, com lockout
- Acionamento e controle de acesso pela porta

Roteiro: "A partir daqui, a evolução transforma o cofre em um controle de acesso de verdade: vários usuários, um administrador que cria e remove contas e reseta senhas, usuários temporários com validade, liberação por horário e uma interface web protegida por login. É o que dá sentido prático ao projeto."

---

## Slide 11: Cronograma

Conteúdo:
- Etapa 1: planejamento e arquitetura, proposta, repositório, prova de conceito
- Etapa 2: comunicação e firmware servidor, rotas validadas
- Etapa 3: integração final, interface, testes e apresentação

Roteiro: "O cronograma tem três etapas. A primeira, que apresentamos hoje, é o planejamento, a arquitetura e a prova de conceito. A segunda é o firmware servidor com a rede. A terceira é a integração final, com a interface completa, os testes de falha e a apresentação."

---

## Slide 12: Encerramento

Conteúdo:
- Repositório: github.com/ols29/cofre-inteligente-esp32
- Espaço para demonstração ao vivo ou vídeo
- Agradecimento e perguntas

Roteiro: "Todo o projeto, com documentação e código, está no nosso repositório público. Podemos mostrar uma demonstração rápida agora. Obrigado, e ficamos abertos a perguntas."

---

## Dicas de apresentação

- Levem o protótipo ligado ou um vídeo curto de trinta segundos mostrando: digitar a senha, abrir, o bloqueio após três erros e o fechamento automático.
- Deixem o diagrama de arquitetura em tela enquanto explicam a rede.
- Se a banca perguntar sobre segurança, usem o slide 8: reconhecer as limitações e apontar as melhorias mostra maturidade.
