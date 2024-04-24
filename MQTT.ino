#include <WiFi.h>  
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
StaticJsonDocument<200> doc;

const int DHT_PIN = 25;  
const int LED_PIN = 5; // Menggunakan pin 5 untuk mengendalikan LED
const char* ssid = "P"; ///  wifi ssid 
const char* password = "juarasatu";
const char* mqtt_server = "192.168.27.179";// mosquitto server url

DHT sensor_dht(DHT_PIN,DHT22);
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float temp = 0;
float hum = 0;

void setup_wifi() { 
  delay(10);
  Serial.println();
  Serial.print("Wifi terkoneksi ke : ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi berhasil terkoneksi");
  Serial.print("Alamat IP : ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) { 
    Serial.print((char)payload[i]);
  }}
void reconnect() { 
  while (!client.connected()) {
    Serial.print("Baru melakukan koneksi MQTT ...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Serial.print(clientId);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected"); 
      digitalWrite(LED_PIN, HIGH); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }}
}
void setup() {
  //pinMode(2, OUTPUT);     
  Serial.begin(115200);
  sensor_dht.begin();
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); 
  pinMode(LED_PIN, OUTPUT); // Inisialisasi pin LED sebagai output
  digitalWrite(LED_PIN, LOW); // Matikan LED secara default
}
void loop() {
  if (!client.connected()) {
    digitalWrite(LED_PIN, LOW);
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) { //perintah publish data
    lastMsg = now;
    
    float temp = sensor_dht.readTemperature();
    float hum = sensor_dht.readHumidity();

    doc["Temperature"] = temp;
    doc["Humidity"] = hum;
    String jsonString;
    serializeJson(doc, jsonString);
    char charArray[jsonString.length() + 1]; // Add one for null terminator
    jsonString.toCharArray(charArray, jsonString.length() + 1);
    client.publish("/input_data", charArray);   // publish hum topic /ThinkIOT/hum

    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(hum);

    delay(2000);
  }
}