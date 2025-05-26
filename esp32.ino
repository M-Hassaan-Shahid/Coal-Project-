#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

// Wi-Fi credentials
#define WIFI_SSID "IOT"
#define WIFI_PASSWORD "123456789"

// MQTT Broker settings
#define MQTT_SERVER "test.mosquitto.org"
#define MQTT_PORT 1883
#define MQTT_TOPIC "TollGateCount"
#define MQTT_CLIENT_ID "ESP32TollGateClient"

// ThingSpeak settings
const char *apiKey = "JGT56NAT7OEQIWEH";
const char *thingSpeakAddress = "http://api.thingspeak.com";

// Clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Vehicle counter
int vehicleCount = 0;
int lastVehicleCount = -1;

// Buffer for serial data
String serialBuffer = "";

void setup() {
  Serial.begin(9600); // For Arduino communication

  setupWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(60);
}

void loop() {
  // Reconnect if needed
  if (WiFi.status() != WL_CONNECTED) {
    setupWiFi();
  }

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // Read serial from Arduino
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      processArduinoMessage(serialBuffer);
      serialBuffer = "";
    } else {
      serialBuffer += c;
    }
  }

  delay(100);
}

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected. IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n❌ Failed to connect to WiFi.");
  }
}

void reconnectMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;

  Serial.println("🔁 Attempting MQTT connection...");
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("✅ MQTT connected");
  } else {
    Serial.print("❌ MQTT failed, rc=");
    Serial.println(mqttClient.state());
    delay(5000);
  }
}

void processArduinoMessage(String message) {
  message.trim();
  Serial.println("📨 From Arduino: " + message);

  if (message == "CAR_DETECTED") {
    vehicleCount++;
    Serial.println("🚗 Car Count: " + String(vehicleCount));

    if (vehicleCount != lastVehicleCount) {
      publishToMQTT(vehicleCount);
      sendToThingSpeak(vehicleCount);
      lastVehicleCount = vehicleCount;
    }
  }
}

void publishToMQTT(int count) {
  if (!mqttClient.connected()) {
    Serial.println("⚠️ MQTT not connected, skipping publish.");
    return;
  }

  char payload[8];
  snprintf(payload, sizeof(payload), "%d", count);
  if (mqttClient.publish(MQTT_TOPIC, payload)) {
    Serial.println("📤 MQTT published: " + String(payload));
  } else {
    Serial.println("⚠️ MQTT publish failed");
  }
}

void sendToThingSpeak(int count) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi not connected, skipping ThingSpeak");
    return;
  }

  String url = String(thingSpeakAddress) + "/update?api_key=" + apiKey + "&field1=" + String(count);
  HTTPClient http;
  http.begin(url);
  int httpResponseCode = http.GET();
  Serial.print("🌐 ThingSpeak response: ");
  Serial.println(httpResponseCode);
  http.end();
}
