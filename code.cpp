// {"id":"JFMDKAVG","origen":"MQTT","accion":"ON","fecha":"03-09-2025","hora":"21:00:00"}

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>

// ======== CONFIGURA AQUÍ ========
const char* WIFI_SSID   = "iPhone_de_Fernando";
const char* WIFI_PASS   = "tobias1560";
const char* MQTT_HOST   = "test.mosquitto.org";
const uint16_t MQTT_PORT = 1883;

const char* TEAM_ID = "JFMDKAVG";

// Tópicos exactos requeridos
String TOPIC_TX = String(TEAM_ID) + "_TX";
String TOPIC_RX = String(TEAM_ID) + "_RX";

// ======== PINES LED ========
#define LED_ROJO     14
#define LED_AZUL     27
#define LED_VERDE    26
#define LED_AMARILLO 25
#define LED_BLANCO   33
#define LED_NARANJA  32
#define LED_VIOLETA  2

const bool USE_STATE_PIN = true;   
const int  PIN_STATE     = 4;    

HardwareSerial BLE(2);

WiFiClient espClient;
PubSubClient mqtt(espClient);

enum State { WIFI_CONNECTING, WIFI_OK, BLE_WAIT, BLE_OK, MQTT_CONNECTING, RUN };
State state = WIFI_CONNECTING;

unsigned long tBlinkBlue = 0, tBlinkWhite = 0;
bool blueOn = false, whiteOn = false;

unsigned long tPulseGreen = 0, tPulseYellow = 0, tPulseOrange = 0;

String uartLine = "";
String usbLine  = "";

unsigned long lastBleActivity = 0;
const unsigned long BLE_CHUNK_TIMEOUT_MS = 1200;
bool bleConnected = false;

void ledSet(uint8_t pin, bool on) { digitalWrite(pin, on ? HIGH : LOW); }

void ledsOffAll() {
  ledSet(LED_ROJO, LOW);
  ledSet(LED_AZUL, LOW);
  ledSet(LED_VERDE, LOW);
  ledSet(LED_AMARILLO, LOW);
  ledSet(LED_BLANCO, LOW);
  ledSet(LED_NARANJA, LOW);
}

void pulseStart(uint8_t ledPin, unsigned long &tPulseRef) {
  ledSet(ledPin, HIGH);
  tPulseRef = millis() + 1000;
}
void handlePulses() {
  if (tPulseGreen && millis() > tPulseGreen)  { tPulseGreen = 0;  ledSet(LED_VERDE, LOW);   }
  if (tPulseYellow && millis() > tPulseYellow){ tPulseYellow = 0; ledSet(LED_AMARILLO, LOW);}
  if (tPulseOrange && millis() > tPulseOrange){ tPulseOrange = 0; ledSet(LED_NARANJA, LOW); }
}

String two(int v){ return (v<10) ? "0"+String(v) : String(v); }
void currentDateTime(String &fecha, String &hora) {
  time_t now = time(NULL);
  struct tm tmNow;
  localtime_r(&now, &tmNow);
  fecha = two(tmNow.tm_mday) + "-" + two(tmNow.tm_mon+1) + "-" + String(1900+tmNow.tm_year);
  hora  = two(tmNow.tm_hour) + ":" + two(tmNow.tm_min) + ":" + two(tmNow.tm_sec);
}

String buildJSON(const char* origen, const char* accion) {
  StaticJsonDocument<256> doc;
  String fecha, hora;
  currentDateTime(fecha, hora);
  doc["id"]     = TEAM_ID;
  doc["origen"] = origen;
  doc["accion"] = accion;
  doc["fecha"]  = fecha;
  doc["hora"]   = hora;
  String out; serializeJson(doc, out); return out;
}

bool processIncomingJSON(const String& payload, bool fromBLE, const char* origenDefault) {
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.print("Error al parsear JSON: "); Serial.println(payload);
    return false;
  }
  const char* id     = doc["id"]     | "";
  const char* origen = doc["origen"] | origenDefault;
  const char* accion = doc["accion"] | "";

  if (String(id) != String(TEAM_ID)) {
    Serial.print("JSON recibido con ID inválido: "); Serial.println(id);
    pulseStart(LED_NARANJA, tPulseOrange);
    return true;
  }

  Serial.print("JSON válido desde "); Serial.print(origen);
  Serial.print(" | accion=");       Serial.println(accion);

  pulseStart(LED_AMARILLO, tPulseYellow);
  if (String(accion).equalsIgnoreCase("ON"))  ledSet(LED_VIOLETA, HIGH);
  if (String(accion).equalsIgnoreCase("OFF")) ledSet(LED_VIOLETA, LOW);

  mqtt.publish(TOPIC_TX.c_str(), payload.c_str());
  return true;
}

// MQTT callback
void onMqtt(char* topic, byte* payload, unsigned int len) {
  String msg; for (uint16_t i=0;i<len;i++) msg += (char)payload[i];
  Serial.print("MQTT ("); Serial.print(topic); Serial.print("): "); Serial.println(msg);
  processIncomingJSON(msg, false, "MQTT");
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando a WiFi SSID: "); Serial.println(WIFI_SSID);
  configTime(-6 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
  state = WIFI_CONNECTING;
  tBlinkWhite = millis(); whiteOn = false;
}
void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqtt);
  state = MQTT_CONNECTING;
  Serial.println("Preparando conexión MQTT...");
}
void ensureMQTT() {
  if (mqtt.connected()) return;
  Serial.println("Intentando conectar a MQTT...");
  while (!mqtt.connected()) {
    String clientId = String("ESP32-") + TEAM_ID + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("MQTT conectado!");
      mqtt.subscribe(TOPIC_RX.c_str());
      Serial.print("Suscrito a: "); Serial.println(TOPIC_RX);
    } else {
      Serial.print("Fallo MQTT, rc="); Serial.println(mqtt.state());
      delay(500);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("=== Iniciando ESP32 ===");

  BLE.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("UART2 hacia AT-09 inicializado a 9600");

  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_BLANCO, OUTPUT);
  pinMode(LED_NARANJA, OUTPUT);
  pinMode(LED_VIOLETA, OUTPUT);
  if (USE_STATE_PIN) pinMode(PIN_STATE, INPUT_PULLDOWN);

  ledsOffAll();
  ledSet(LED_VIOLETA, LOW);

  // Estado inicial BLE = no conectado
  bleConnected = false;
  ledSet(LED_ROJO, HIGH);
  ledSet(LED_AZUL, LOW);

  connectWiFi();
}

void loop() {
  handlePulses();

  // ====== WiFi
  if (state == WIFI_CONNECTING) {
    if (millis() - tBlinkWhite >= 500) {
      tBlinkWhite = millis(); whiteOn = !whiteOn; ledSet(LED_BLANCO, whiteOn);
    }
    if (WiFi.status() == WL_CONNECTED) {
      ledSet(LED_BLANCO, HIGH);
      state = WIFI_OK;
      Serial.println("WiFi conectado correctamente!");
      connectMQTT();
    }
  }

  // ====== MQTT
  if (state == WIFI_OK || state == MQTT_CONNECTING || state == RUN || state == BLE_OK) {
    ensureMQTT(); mqtt.loop();
  }

  // ====== BLE RX (ensamblado por '}' o timeout)
  while (BLE.available()) {
    char c = BLE.read();
    uartLine += c;
    lastBleActivity = millis();

    if (c == '}' || c == '\n' || c == '\r') {
      uartLine.trim();
      if (uartLine.length()) {
        Serial.print("BLE recibido: "); Serial.println(uartLine);
        processIncomingJSON(uartLine, true, "Smartphone");
      }
      uartLine = "";
    }
  }

  // Timeout para cerrar mensaje si no llegó \n
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 100) {
    lastCheck = millis();
    if (uartLine.length() > 0 && (millis() - lastBleActivity > BLE_CHUNK_TIMEOUT_MS)) {
      uartLine.trim();
      if (uartLine.endsWith("}")) {
        Serial.print("BLE (timeout) recibido: "); Serial.println(uartLine);
        processIncomingJSON(uartLine, true, "Smartphone");
      } else {
        Serial.print("BLE (timeout) descartado: "); Serial.println(uartLine);
      }
      uartLine = "";
    }
  }

  if (USE_STATE_PIN) {
    bleConnected = digitalRead(PIN_STATE); 
  } else {
    bleConnected = (millis() - lastBleActivity) < (BLE_CHUNK_TIMEOUT_MS);
  }

  if (!bleConnected) {
    ledSet(LED_ROJO, HIGH);
    if (millis() - tBlinkBlue >= 500) {
      tBlinkBlue = millis(); blueOn = !blueOn; ledSet(LED_AZUL, blueOn);
    }
  } else {
    ledSet(LED_ROJO, LOW);
    ledSet(LED_AZUL, HIGH);
  }

  // ====== Entrada por USB Serial
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      usbLine.trim();
      if (usbLine.length()) {
        String accion = usbLine; accion.toUpperCase();
        if (accion != "ON" && accion != "OFF") accion = "OFF";
        String json = buildJSON("Consola serial", accion.c_str());

        BLE.println(json);
        mqtt.publish(TOPIC_TX.c_str(), json.c_str());
        pulseStart(LED_VERDE, tPulseGreen);

        Serial.print("USB -> JSON enviado: "); Serial.println(json);
      }
      usbLine = "";
    } else {
      usbLine += c;
    }
  }
}
