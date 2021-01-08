#include <ArduinoJson.h> //https://arduinojson.org/
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include "DHT.h"

#define plant_1_analog_pin A0
#define plant_2_analog_pin A1
#define plant_3_analog_pin A2
#define plant_4_analog_pin A3
#define plant_5_analog_pin A4

#define DHTPIN 7        // digital pin

// Remember to create a file "arduino_secrets.h" in the same folder as this code.
// Add the following lines:
//#define SECRET_SSID "YOURSSID"
//#define SECRET_PASS "YOURPASSWD"
//#define SECRET_BROKER "YOURBROKER"
//#define SECRET_TOPIC_PUBLISH "YOURPUBTOPIC"

#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;
const char broker[] = SECRET_BROKER;
const int port = 1883;
const char topic_publish[]  = SECRET_TOPIC_PUBLISH;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

//Dynamic variables
float analogueValues[] = {0, 0, 0, 0, 0, 0};
DynamicJsonDocument doc(2048);
String json;
unsigned long currentMillis;
unsigned long timeElapsed = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(500);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  // Initialize DHT sensor
  dht.begin();

  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  // mqttClient.setId("clientId");

  // You can provide a username and password for authentication
  // mqttClient.setUsernamePassword("username", "password");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
}

void loop() {
  mqttClient.poll();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(1000);
    return;
  }

  // Compute heat index
  // Must send in temp in Fahrenheit!
  float hi = dht.computeHeatIndex(f, h);

  doc["HP"] = h;
  doc["TC"] = t;
  doc["TF"] = f;
  doc["HI"] = hi;


  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker

  doc["A0"] = analogRead(A0);
  doc["A1"] = analogRead(A1);
  doc["A2"] = analogRead(A2);
  doc["A3"] = analogRead(A3);
  doc["A4"] = analogRead(A4);
  doc["A5"] = analogRead(A5);

  json = "";
  serializeJson(doc, json);

  Serial.println(json);

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(topic_publish);
  mqttClient.print(json);
  mqttClient.endMessage();

  delay(60000);
}
