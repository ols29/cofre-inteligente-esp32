# Etapa 1: Planejamento e arquitetura

Checklist de execução. Marque conforme concluir.

## 1. Proposta

- [ ] Preencher nomes, RAs e prazos em `docs/proposta-etapa1.md`
- [ ] Enviar pelo Canvas
- [ ] Registrar a data de aprovação do professor

## 2. Repositório

- [ ] Criar no GitHub
- [ ] Subir a estrutura inicial
- [ ] Se privado, adicionar o professor como colaborador
- [ ] Cada integrante fazer ao menos um commit nesta etapa

```bash
git init
git add .
git commit -m "docs: estrutura inicial e proposta do tema"
git remote add origin https://github.com/ols29/cofre-inteligente-esp32.git
git push -u origin main
```

## 3. Lista de materiais

| Item | Função | Preço aprox. |
|---|---|---|
| ESP32 NodeMCU (WROOM-32) | Servidor e controle | R$ 45 |
| Teclado matricial 4x4 | Sensor: credencial | R$ 15 |
| Sensor DHT11 | Sensor: temperatura e umidade | R$ 10 |
| Sensor IR de obstáculo | Sensor: ocupação | R$ 10 |
| Display OLED 0.96" I2C (SSD1306) | Atuador: estado local | R$ 30 |
| Servo motor SG90 | Atuador: trava | R$ 15 |
| Buzzer ativo 5V | Atuador: alerta | R$ 4 |
| 2 LEDs + 2 resistores 220Ω | Atuador: indicação | R$ 3 |
| Capacitor eletrolítico 470 µF / 16V | Estabilização do servo | R$ 2 |
| Protoboard 400 pontos + jumpers | Montagem | R$ 30 |
| **Total** | | **~R$ 164** |

Comprar dois ESP32, se o orçamento permitir. A FAQ 4 trata de falha de hardware na véspera.

Fornecedores em Curitiba: Baú da Eletrônica, Eletrônica Sanches. Online: MakerHero, Casa da Robótica, Usinainfo.

- [ ] Componentes comprados
- [ ] Componentes recebidos e conferidos

## 4. Pinagem consolidada

| Componente | Pino ESP32 | Observação |
|---|---|---|
| Teclado, linhas R1 a R4 | GPIO 13, 12, 14, 27 | |
| Teclado, colunas C1 a C4 | GPIO 26, 25, 33, 32 | |
| OLED, SDA | GPIO 21 | I2C, endereço `0x3C` |
| OLED, SCL | GPIO 22 | |
| OLED, VCC / GND | 3V3 / GND | |
| Servo, sinal (laranja) | GPIO 4 | |
| Servo, VCC / GND | 5V / GND | GND comum, capacitor 470 µF |
| DHT11, dados | GPIO 19 | Resistor pull-up 10 kΩ se o módulo não tiver |
| Sensor IR, saída | GPIO 18 | Saída em LOW quando detecta objeto |
| Buzzer (+) | GPIO 15 | |
| LED verde | GPIO 2 | Resistor 220Ω |
| LED vermelho | GPIO 5 | Resistor 220Ω |

GPIO 6 a 11 são reservados à memória flash e não podem ser usados.

## 5. Teste isolado, um componente por vez

Não monte tudo de uma vez. Cada item abaixo com um sketch de exemplo separado.

- [ ] **Servo:** alternar 0° e 90°. Se o ESP32 reiniciar, instalar o capacitor e conferir o GND comum.
- [ ] **OLED:** exibir texto. Se não acender, rodar um scanner I2C: alguns módulos usam `0x3D`.
- [ ] **Teclado:** imprimir a tecla no Serial Monitor. Se a tecla sair trocada, inverter os arrays de linhas e colunas.
- [ ] **DHT11:** imprimir temperatura e umidade. Leituras `NaN` ocasionais são normais.
- [ ] **Sensor IR:** imprimir o estado do pino. Ajustar o potenciômetro do módulo para a distância desejada.
- [ ] **Buzzer e LEDs:** conferir polaridade.

## 6. Integração local

- [ ] Montar o circuito completo na protoboard
- [ ] Compilar e enviar `src/esp32/etapa1_prova_conceito/etapa1_prova_conceito.ino`
- [ ] Confirmar que o Serial Monitor mostra leituras a cada 2 segundos

Bibliotecas necessárias: `Keypad`, `ESP32Servo`, `Adafruit SSD1306`, `Adafruit GFX`, `DHT sensor library`.

## 7. Critérios de aceite

Sete comportamentos, todos sem rede:

- [ ] Digitar `1234` e apertar `#` → servo gira, LED verde acende, OLED mostra ABERTO
- [ ] Após 10 segundos → tranca sozinho, LED volta a vermelho
- [ ] Digitar senha errada três vezes → OLED mostra BLOQUEADO, buzzer intermitente
- [ ] Após 30 segundos → bloqueio se encerra sozinho
- [ ] Durante o bloqueio, o teclado ignora qualquer tecla
- [ ] Rodapé do OLED mostra temperatura, umidade e ocupação atualizando
- [ ] Aproximar a mão do sensor IR → rodapé muda de "vazio" para "cheio"

Teste extra do alarme térmico: aproximar algo morno do DHT11 até passar de 40°C. O OLED deve mostrar ALERTA e o buzzer apitar mais rápido que no bloqueio.

## 8. Registro

- [ ] Fotografar a protoboard montada e salvar em `docs/`
- [ ] Gravar vídeo curto do sistema funcionando
- [ ] Commit final da Etapa 1

## Atenção

A FAQ 13 do regulamento estabelece que mudanças de tema só são aceitas até a finalização desta etapa. Depois daqui, o escopo está fechado.
