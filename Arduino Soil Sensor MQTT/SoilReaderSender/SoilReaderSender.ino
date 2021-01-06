#include <ArduinoJson.h> //https://arduinojson.org/
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

// Remember to create a file "arduino_secrets.h" in the same folder as this code.
// Add the following 2 lines:
// #define SECRET_SSID "YOURWIFISSID"
// #define SECRET_PASS "YOURWIFIPASSWORD"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "192.168.10.250";
int        port     = 1883;
const char topic_publish[]  = "analogRead";
const char topic_subscribe[]  = "analogReadInterval";

const int default_interval = 10000;
int interval = default_interval;

DynamicJsonDocument doc(500);

float analogueValues[] = {0, 0, 0, 0, 0, 0};
int calibrationWet = 327;
int calibrationDry = 588;

String json;
unsigned long currentMillis;
long timeElapsed = 0;

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
  Serial.print("Further logging will be to:");
  Serial.println(topic_log);

  Serial.print("Subscribing to topic: ");
  Serial.println(topic_subscribe);
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe(topic_subscribe);
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();
  if (interval < 1000) {
    Serial.print("Interval was set below 1000: ");
    Serial.println(interval);

    Serial.print("Resetting to default interval:");
    Serial.println(default_interval);
    interval = default_interval;
  }

  int messageSize = mqttClient.parseMessage();
  if (messageSize) {
    // use the Stream interface to print the contents
    char messageArray[messageSize];
    int offset = 0;
    while (mqttClient.available()) {
      char c = (char)mqttClient.read();
      messageArray[offset++] = c;
    }
    messageArray[messageSize] = '\0';
    int n = atoi(messageArray);
    if (n >= 1000) {
      interval = n;
    } else {
      interval = default_interval;
    }
    Serial.print("Interval set to: ");
    Serial.println(interval);
  }

  // Add values in the json document
  analogueValues[0] = map(analogRead(A0), calibrationDry, calibrationWet, 0, 100);
  analogueValues[1] = map(analogRead(A1), calibrationDry, calibrationWet, 0, 100);
  analogueValues[2] = map(analogRead(A2), calibrationDry, calibrationWet, 0, 100);
  analogueValues[3] = map(analogRead(A3), calibrationDry, calibrationWet, 0, 100);
  analogueValues[4] = map(analogRead(A4), calibrationDry, calibrationWet, 0, 100);
  analogueValues[5] = map(analogRead(A5), calibrationDry, calibrationWet, 0, 100);

  doc["A0"] = analogueValues[0];
  doc["A1"] = analogueValues[1];
  doc["A2"] = analogueValues[2];
  doc["A3"] = analogueValues[3];
  doc["A4"] = analogueValues[4];
  doc["A5"] = analogueValues[5];
  doc["interval"] = interval;

  json = "";
  serializeJson(doc, json);

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(topic_publish);
  mqttClient.print(json);
  mqttClient.endMessage();

  Serial.println(json);
  delay(interval);
}
