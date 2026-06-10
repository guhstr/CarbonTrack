# 🌱 CarbonTrack IoT

> Monitoramento inteligente do solo para o mercado de créditos de carbono  

---

## 👨‍💻 Integrantes


Erik Naoki Miyasato | 565771
Gustavo Arthur Carvalho Sartori | 561650
Gutemberg Rocha Silva | 562267
João Henrique Batista Leal | 564361
Juliana da Silva Stigliani | 561171

---

## 🔗 Links

> 🔗 https://youtu.be/9_T-jbL8OfU
> 🔗 https://github.com/guhstr/CarbonTrack

---

## 📌 Sobre o Projeto

O **CarbonTrack IoT** é um dispositivo físico espetado no solo de propriedades rurais que monitora continuamente as condições ambientais determinantes para o sequestro de carbono pelas plantas.

O mercado de créditos de carbono exige que propriedades provem, com dados concretos, que estão absorvendo carbono da atmosfera. Hoje esse processo é feito com vistorias manuais, caras e esporádicas. O CarbonTrack automatiza essa coleta, enviando dados em tempo real para a nuvem, tornando o processo confiável, rastreável e auditável.

### Conexão com o tema — Economia Espacial

Satélites monitoram desmatamento e emissões em larga escala, mas precisam de **dados locais no solo** para calibrar suas medições orbitais. O CarbonTrack é exatamente esse elo — conecta o sensor físico no campo com os dados de satélite, tornando o mercado de créditos de carbono mais preciso e confiável.

---

## 🏗️ Arquitetura do Sistema

```
ESP32 (Wokwi)
  → coleta temperatura, umidade e nível de água
  → exibe no LCD local
  → sinaliza via LED e buzzer
  → publica via MQTT (TLS/8883)
      ↓
HiveMQ Cloud (broker MQTT)
      ↓
Node-RED
  → recebe e processa os dados
  → exibe no dashboard em tempo real
  → salva no banco de dados
      ↓
MySQL — Clever Cloud
  → histórico completo das leituras
```

---

## 🛠️ Tecnologias Utilizadas

| Tecnologia | Função |
|---|---|
| **ESP32 DevKit V1** | Microcontrolador principal |
| **Wokwi** | Simulação do circuito |
| **HiveMQ Cloud** | Broker MQTT com TLS |
| **Node-RED** | Processamento e dashboard |
| **MySQL — Clever Cloud** | Banco de dados na nuvem |

---

## 🔌 Hardware — Componentes e Pinos

| Componente | Função | Pino ESP32 |
|---|---|---|
| DHT22 | Temperatura e umidade do ar | 15 |
| HC-SR04 | Nível de água simulado | TRIG=13, ECHO=12 |
| LCD 16x2 I2C | Display local | SDA=21, SCL=22 |
| LED Verde | Solo saudável / captura alta | 25 |
| LED Vermelho | Solo seco / risco degradação | 26 |
| Buzzer | Alerta sonoro | 27 |

---

## 📡 Tópicos MQTT

| Tópico | Tipo | Descrição | Exemplo |
|---|---|---|---|
| `carbontrack/temperatura` | float | Temperatura do ar em °C | `24.0` |
| `carbontrack/umidade` | float | Umidade do ar em % | `40.0` |
| `carbontrack/nivel_agua` | float | Distância HC-SR04 em cm | `15.3` |
| `carbontrack/alerta` | string | Status do solo | `SAUDAVEL` ou `SECO` |

---

## ⚙️ Lógica de Funcionamento

- **Nível de água abaixo de 20cm** → solo com água → LED Verde → publica `SAUDAVEL`
- **Nível de água acima de 20cm** → solo seco → LED Vermelho → Buzzer → publica `SECO`
- A cada **5 segundos** os dados são lidos e publicados automaticamente
- O LCD exibe temperatura, umidade e status em tempo real para o operador de campo

---

## 🌍 ODS da ONU Relacionados

- **ODS 13** — Ação contra a mudança global do clima
- **ODS 15** — Vida terrestre e proteção dos ecossistemas
- **ODS 9** — Indústria, inovação e infraestrutura

---