#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <LiquidCrystal_I2C.h>

// ── Wi-Fi ─────────────────────────────────────────────
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// ── MQTT HiveMQ Cloud ─────────────────────────────────
const char* MQTT_BROKER   = "3f3a91da234e4351bfbad9f9e86a145c.s1.eu.hivemq.cloud";
const int   MQTT_PORT     = 8883;
const char* MQTT_CLIENT   = "carbontrack-esp32-47291";
const char* MQTT_USER     = "gustavostr";
const char* MQTT_PASS     = "260207Gu";

// ── Topicos ───────────────────────────────────────────
const char* TOPIC_TEMP       = "carbontrack/temperatura";
const char* TOPIC_UMIDADE    = "carbontrack/umidade";
const char* TOPIC_NIVEL      = "carbontrack/nivel_agua";
const char* TOPIC_ALERTA     = "carbontrack/alerta";

// ── Pinos ─────────────────────────────────────────────
#define PIN_DHT22      15
#define PIN_TRIG       13
#define PIN_ECHO       12
#define PIN_LED_VERDE  25
#define PIN_LED_VERM   26
#define PIN_BUZZER     27

// ── Limites ───────────────────────────────────────────
// Distancia acima de 20cm = poco agua = solo seco
#define LIMITE_DISTANCIA  20
#define INTERVALO_PUB     5000

// ── Objetos ───────────────────────────────────────────
WiFiClientSecure  wifiClient;
PubSubClient      mqttClient(wifiClient);
DHTesp            dht;
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long ultimaPublicacao = 0;

// ─────────────────────────────────────────────────────
void conectarWifi() {
  Serial.print("[WiFi] Conectando...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("[WiFi] Conectado! IP: ");
  Serial.println(WiFi.localIP());
}

// ─────────────────────────────────────────────────────
void conectarMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("[MQTT] Conectando ao broker...");
    if (mqttClient.connect(MQTT_CLIENT, MQTT_USER, MQTT_PASS)) {
      Serial.println(" conectado!");
    } else {
      Serial.print(" falhou, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" tentando em 3s");
      delay(3000);
    }
  }
}

// ─────────────────────────────────────────────────────
// HC-SR04: retorna distancia em cm
float lerDistancia() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duracao = pulseIn(PIN_ECHO, HIGH, 30000);
  if (duracao == 0) return 999.0;

  float distancia = (duracao * 0.0343) / 2.0;
  return distancia;
}

// ─────────────────────────────────────────────────────
void atualizarLCD(float temp, float umidade, float distancia, bool seco) {
  lcd.clear();

  // Linha 0: temperatura e umidade
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.print("C U:");
  lcd.print((int)umidade);
  lcd.print("%");

  // Linha 1: nivel agua e status
  lcd.setCursor(0, 1);
  lcd.print("Agua:");
  lcd.print((int)distancia);
  lcd.print("cm ");
  lcd.print(seco ? "SECO" : "OK");
}

// ─────────────────────────────────────────────────────
void atualizarSinalizacao(bool seco) {
  if (seco) {
    digitalWrite(PIN_LED_VERDE, LOW);
    digitalWrite(PIN_LED_VERM, HIGH);
    for (int i = 0; i < 3; i++) {
      tone(PIN_BUZZER, 1000, 200);
      delay(300);
    }
  } else {
    digitalWrite(PIN_LED_VERDE, HIGH);
    digitalWrite(PIN_LED_VERM, LOW);
    noTone(PIN_BUZZER);
  }
}

// ─────────────────────────────────────────────────────
void publicarMQTT(float temp, float umidade, float distancia, bool seco) {
  char buffer[16];

  snprintf(buffer, sizeof(buffer), "%.1f", temp);
  mqttClient.publish(TOPIC_TEMP, buffer);

  snprintf(buffer, sizeof(buffer), "%.1f", umidade);
  mqttClient.publish(TOPIC_UMIDADE, buffer);

  snprintf(buffer, sizeof(buffer), "%.1f", distancia);
  mqttClient.publish(TOPIC_NIVEL, buffer);

  const char* status = seco ? "SECO" : "SAUDAVEL";
  mqttClient.publish(TOPIC_ALERTA, status);
}

// ─────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== CarbonTrack IoT - Guardiao de Solo ===");

  // Pinos de saida
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_VERM,  OUTPUT);
  pinMode(PIN_BUZZER,    OUTPUT);
  pinMode(PIN_TRIG,      OUTPUT);
  pinMode(PIN_ECHO,      INPUT);

  digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_VERM,  LOW);

  // DHT22
  dht.setup(PIN_DHT22, DHTesp::DHT22);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("CarbonTrack IoT");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(2000);

  // Wi-Fi
  conectarWifi();

  // MQTT com SSL
  wifiClient.setInsecure();
  mqttClient.setSocketTimeout(10);
  mqttClient.setKeepAlive(60);
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  conectarMQTT();

  Serial.println("=== Sistema pronto ===");
}

// ─────────────────────────────────────────────────────
void loop() {
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

  unsigned long agora = millis();

  if (agora - ultimaPublicacao >= INTERVALO_PUB) {
    ultimaPublicacao = agora;

    // Leitura DHT22
    TempAndHumidity leitura = dht.getTempAndHumidity();
    float temp    = isnan(leitura.temperature) ? 25.0 : leitura.temperature;
    float umidade = isnan(leitura.humidity)    ? 50.0 : leitura.humidity;

    // Leitura HC-SR04
    float distancia = lerDistancia();

    // Solo seco = distancia grande (pouca agua)
    bool seco = (distancia > LIMITE_DISTANCIA);

    Serial.printf("[Sensor] Temp: %.1f C | Umidade: %.1f%% | Agua: %.1f cm | Status: %s\n",
                  temp, umidade, distancia, seco ? "SECO" : "SAUDAVEL");

    atualizarLCD(temp, umidade, distancia, seco);
    atualizarSinalizacao(seco);
    publicarMQTT(temp, umidade, distancia, seco);
  }
}