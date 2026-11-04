# Vault32 — Sistema Inteligente de Controle de Acesso com ESP32

O **Vault32** é um protótipo de sistema ciberfísico para controle de acesso físico, desenvolvido com ESP32. O projeto combina autenticação local por senha ou cartão RFID, acionamento eletrônico de uma trava e monitoramento por meio de uma interface web na rede local.

**Disciplina:** Conectividade em Sistemas Ciberfísicos  
**Curso:** Engenharia de Software — PUCPR

## Problema

Cofres, armários e salas técnicas protegidos exclusivamente por chaves físicas apresentam limitações de segurança e monitoramento. Normalmente, não oferecem registro de tentativas de acesso, identificação dos meios de autenticação utilizados ou possibilidade de controle remoto.

O Vault32 propõe uma alternativa baseada em sistemas embarcados, permitindo autenticação eletrônica, acionamento automatizado e consulta de informações pela rede local.

## Solução proposta

O sistema utiliza um ESP32 como unidade central de processamento e controle. O acesso pode ser solicitado por dois meios locais:

- **Teclado de membrana:** permite inserir uma senha numérica. A tecla `#` confirma a senha e `*` apaga os caracteres digitados.
- **Leitor RFID RC522:** realiza a leitura do cartão ou tag e permite verificar se sua identificação está autorizada.

Quando a autenticação é aceita, o servo motor SG90 realiza o movimento de abertura da trava e o LED indica a liberação do acesso. O sistema também prevê o fechamento automático após 10 segundos.

Após três tentativas consecutivas inválidas, o acesso é bloqueado temporariamente por 30 segundos.

## Arquitetura do sistema

O ESP32 centraliza o processamento das entradas, a lógica de autenticação, o controle dos atuadores e a comunicação com a interface web.

```text
              APLICAÇÃO CLIENTE
          Navegador em celular ou PC
                      |
                      | Wi-Fi / HTTP
                      |
                      v
          +-----------------------+
          |         ESP32         |
          |                       |
          |  Processamento central|
          |  Autenticação         |
          |  Servidor web         |
          +-----------------------+
              |             |
       ENTRADAS          SAÍDAS
              |             |
        +-----------+   +-----------+
        | Teclado   |   | Servo SG90|
        | RFID RC522|   | LED       |
        +-----------+   +-----------+
              |
         LCD 16x2
      Exibição de estados
```

## Hardware utilizado

| Componente | Função |
|---|---|
| ESP32 DevKit (30 pinos) | Processamento, conectividade e controle |
| Teclado de membrana | Entrada de senha |
| RFID RC522 | Leitura de cartões e tags |
| LCD 16x2 I2C | Exibição de mensagens e estados |
| Servo motor SG90 | Acionamento da trava |
| LED | Indicação visual do estado de acesso |
| Placa de expansão, protoboard e jumpers | Montagem e conexões |

**Controlador:** ESP32  
**Linguagem:** C++  
**Ambiente de desenvolvimento:** Arduino IDE

*Os detalhes de pinagem e alimentação devem ser consultados no esquema elétrico atualizado do protótipo.*

## Funcionamento e autenticação

O sistema utiliza uma lógica central de controle para processar as solicitações de acesso.

### Autenticação por senha

1. O usuário digita a senha no teclado.
2. Confirma a entrada utilizando `#`.
3. O ESP32 verifica a senha informada.
4. Se estiver correta, o acesso é liberado.
5. Se estiver incorreta, uma tentativa inválida é contabilizada.

### Autenticação por RFID

1. O usuário aproxima o cartão ou tag do leitor RC522.
2. O ESP32 realiza a leitura da identificação.
3. A identificação é comparada com os cartões autorizados.
4. O acesso é liberado ou negado conforme a validação.

### Controle de acesso e bloqueio

- Três tentativas inválidas consecutivas provocam bloqueio temporário.
- O bloqueio tem duração de 30 segundos.
- Durante a liberação, o servo realiza a abertura da trava.
- O fechamento automático ocorre após 10 segundos.

O LCD apresenta mensagens para orientar o usuário e informar o estado do equipamento.

## Conectividade e interface web

O ESP32 opera como um ponto de acesso Wi-Fi próprio (**Access Point**), identificado pela rede `Vault32`.

Essa configuração permite que celulares e computadores se conectem diretamente ao equipamento, sem depender de um roteador externo.

A interface web permite acompanhar o estado do sistema e interagir com as funcionalidades de controle disponíveis no firmware.

Entre as funcionalidades previstas para a aplicação estão:

- Consulta do estado atual do cofre.
- Solicitação de abertura pela rede.
- Consulta do histórico de eventos.
- Controle administrativo do sistema.

**Endereço de acesso:** `http://192.168.4.1`

*As rotas HTTP, os parâmetros e os formatos de resposta serão documentados conforme a implementação efetivamente presente no firmware.*

## Tecnologias utilizadas

| Tecnologia | Aplicação |
|---|---|
| C++ | Programação do ESP32 |
| Arduino IDE | Desenvolvimento e gravação do firmware |
| Wi-Fi | Comunicação entre ESP32 e cliente |
| HTTP | Comunicação entre navegador e servidor embarcado |
| I2C | Comunicação com o LCD |
| SPI | Comunicação com o leitor RFID RC522 |

## Limitações conhecidas

O Vault32 é um protótipo acadêmico, não um dispositivo de segurança pronto para uso comercial.

Entre os pontos que precisam ser considerados estão:

- A resistência mecânica do servo SG90 é limitada.
- A proteção da comunicação HTTP precisa ser aprimorada para aplicações reais.
- A persistência dos registros e das credenciais depende da implementação de armazenamento não volátil.
- O controle de usuários e permissões pode ser expandido.
- A interface e os mecanismos de autenticação podem receber melhorias adicionais.

## Melhorias futuras

- Aprimoramento da interface web.
- Organização e expansão do histórico de eventos.
- Integração com Telegram para notificações.
- Melhorias nos mecanismos de autenticação e segurança.
- Gerenciamento de usuários e permissões.

## Repositório

Código-fonte, documentação e histórico de desenvolvimento:

[GitHub — cofre-inteligente-esp32](https://github.com/ols29/cofre-inteligente-esp32)

## Uso de inteligência artificial

Ferramentas de inteligência artificial generativa foram utilizadas como apoio à organização da documentação e ao esclarecimento de dúvidas conceituais. A equipe é responsável pela revisão, compreensão e validação das funcionalidades e do código apresentados no projeto.
