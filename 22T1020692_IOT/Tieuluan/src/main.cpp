#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

// ===== WIFI =====
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// ===== MQTT (HiveMQ Cloud) =====
const char* mqtt_server = "a59f8c8028934b1d812cc62e6a2ce31b.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;

const char* mqtt_user = "TranNhat";
const char* mqtt_pass = "Nhattc123456";

// ===== DHT =====
#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== LED =====
#define LED_PIN 5

// ===== OBJECT =====
WiFiClientSecure espClient;
PubSubClient client(espClient);

// ===== CALLBACK =====
void callback(char* topic, byte* payload, unsigned int length) {
  String message;

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Nhan duoc [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  if (String(topic) == "esp32/led") {
    if (message == "ON") {
      digitalWrite(LED_PIN, HIGH);
    } else if (message == "OFF") {
      digitalWrite(LED_PIN, LOW);
    }
  }
}

// ===== WIFI =====
void setup_wifi() {
  Serial.print("Dang ket noi WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Da ket noi WiFi!");
}

// ===== MQTT CONNECT =====
void reconnect() {
  while (!client.connected()) {
    Serial.print("Dang ket noi MQTT...");

    if (client.connect("ESP32_Client", mqtt_user, mqtt_pass)) {
      Serial.println("✅ Da ket noi MQTT!");

      client.subscribe("esp32/led");

    } else {
      Serial.print("❌ Loi: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  dht.begin();

  setup_wifi();

  espClient.setInsecure(); // bỏ verify SSL

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// ===== LOOP =====
void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Loi doc DHT!");
    return;
  }

  String tempStr = String(temp);
  String humStr = String(hum);

  client.publish("esp32/temperature", tempStr.c_str());
  client.publish("esp32/humidity", humStr.c_str());

  Serial.print("Gui -> Nhiet do: ");
  Serial.print(tempStr);
  Serial.print(" | Do am: ");
  Serial.println(humStr);

  delay(3000);
}