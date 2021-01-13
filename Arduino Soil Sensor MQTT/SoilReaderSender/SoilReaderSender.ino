#include <ArduinoJson.h> //https://arduinojson.org/
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

//Need to add both of these as zips:
//https://github.com/adafruit/DHT-sensor-library/releases
//https://github.com/adafruit/Adafruit_Sensor/releases
#include "DHT.h"

#define plant_1_analog_pin A0
#define plant_2_analog_pin A1
#define plant_3_analog_pin A2
#define plant_4_analog_pin A3
#define plant_5_analog_pin A4

// Remember to create a file "arduino_secrets.h" in the same folder as this code.
// Add the following lines:
//#define SECRET_SSID "YOURSSID"
//#define SECRET_PASS "YOURPASSWD"
//#define SECRET_BROKER "YOURBROKER"
//#define SECRET_TOPIC_PUBLISH "YOURPUBTOPIC"

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTPIN 7        // digital pin

DHT dht(DHTPIN, DHTTYPE);

const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;
const char mqtt_broker[] = SECRET_BROKER;
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
const char dbg = 1;
int connectionLED = 13;
int status = WL_IDLE_STATUS;                     // the Wifi radio's status
unsigned long time1;
unsigned long time2 = 0;

const float analogChangeThreshold = 1024 * 0.05; //5%
const float digitalChangeThreshold = 1;     //1%

float lastRSSI = -1;
float lastHP = -1;
float lastTC = -1;
float lastTF = -1;
float lastHI = -1;

float lastA0f = -1;
float lastA1f = -1;
float lastA2f = -1;
float lastA3f = -1;
float lastA4f = -1;
float lastA5f = -1;

void setup()
{
  //Initialize serial and wait for port to open:
  if (dbg)
    Serial.begin(9600);

  // Initialize DHT sensor
  dht.begin();

  pinMode(connectionLED, OUTPUT);   // set the LED pin mode
  digitalWrite(connectionLED, LOW); //LED OFF to show disconnected
  if (dbg)
    while (!Serial); // wait for serial port to connect. Needed for Leonardo only

  while (!ScanSSIDs()) {
    WiFiConnect();
  }
}

void loop() {
  time1 = millis();
  if ((time1 - time2) > 10000) //every 10s
  {
    time2 = time1;
    TestWiFiConnection(); //test connection, and reconnect if necessary
    long rssi = WiFi.RSSI();

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
      logError("Failed to read from DHT sensor!");
    }

    // Compute heat index
    // Must send in temp in Fahrenheit!
    float hi = dht.computeHeatIndex(f, h);

    float A0f = analogRead(A0);
    float A1f = analogRead(A1);
    float A2f = analogRead(A2);
    float A3f = analogRead(A3);
    float A4f = analogRead(A4);
    float A5f = analogRead(A5);

    doc["RSSI"] = rssi;
    doc["HP"] = h;
    doc["TC"] = t;
    doc["TF"] = f;
    doc["HI"] = hi;

    doc["A0"] = A0f;
    doc["A1"] = A1f;
    doc["A2"] = A2f;
    doc["A3"] = A3f;
    doc["A4"] = A4f;
    doc["A5"] = A5f;

    // comment out irrelevant comparisons:
    char changeGate = 0;
    if (!changeGate && abs(lastA0f - A0f) > analogChangeThreshold) {
      logInfo("A0 opens change gate ");
      Serial.print(lastA0f);
      Serial.print(" ");
      Serial.print(A0f);
      Serial.print(" : ");
      Serial.print(abs(lastA0f - A0f));
      Serial.println();
      lastA0f = A0f;
      changeGate = 1;
    }

    if (!changeGate && abs(lastA5f - A5f) > analogChangeThreshold) {
      logInfo("A5 opens change gate ");
      Serial.print(lastA5f);
      Serial.print(" ");
      Serial.print(A5f);
      Serial.print(" : ");
      Serial.print(abs(lastA5f - A5f));
      Serial.println();
      lastA5f = A5f;
      changeGate = 1;
    }

    if (!changeGate && abs(lastHP - h) > digitalChangeThreshold) {
      logInfo("HP opens change gate ");
      Serial.print(lastHP);
      Serial.print(" ");
      Serial.print(h);
      Serial.print(" : ");
      Serial.print(abs(lastHP - h));
      Serial.println();
      lastHP = h;
      changeGate = 1;
    }

    if (!changeGate && abs(lastTC - t) > digitalChangeThreshold) {
      logInfo("TC opens change gate ");
      Serial.print(lastTC);
      Serial.print(" ");
      Serial.print(t);
      Serial.print(" : ");
      Serial.print(abs(lastTC - t));
      Serial.println();
      lastTC = t;
      changeGate = 1;
    }

    if (!changeGate && abs(lastHI - hi) > digitalChangeThreshold) {
      logInfo("HI opens change gate ");
      Serial.print(lastHI);
      Serial.print(" ");
      Serial.print(hi);
      Serial.print(" : ");
      Serial.print(abs(lastHI - hi));
      Serial.println();
      lastHI = hi;
      changeGate = 1;
    }

    if (changeGate) {
      json = "";
      serializeJson(doc, json);

      logInfo(json);

      // send message, the Print interface can be used to set the message contents
      mqttClient.beginMessage(topic_publish);
      mqttClient.print(json);
      mqttClient.endMessage();
    } else {
      logInfo("Sensor values did not satisfy change threshold");
    }
  }
}

void logInfo(String message) {
  Serial.print("INFO: ");
  Serial.print(message);
  Serial.println();

  mqttClient.beginMessage("log_info");
  mqttClient.print(message);
  mqttClient.endMessage();
}


void logError(String message) {
  Serial.print("ERROR: ");
  Serial.print(message);
  Serial.println();

  mqttClient.beginMessage("log_error");
  mqttClient.print(message);
  mqttClient.endMessage();
}

void TestWiFiConnection()
//test if always connected
{
  Serial.println("TestWiFiConnection: Testing");
  int StatusWiFi = WiFi.status();
  if (StatusWiFi == WL_CONNECTION_LOST || StatusWiFi == WL_DISCONNECTED || StatusWiFi == WL_SCAN_COMPLETED) //if no connection
  {
    Serial.println("TestWiFiConnection: Not Connected");
    digitalWrite(connectionLED, LOW); //LED OFF to show disconnected
    if (ScanSSIDs())
      WiFiConnect(); //if my SSID is present, connect
  } else {
    Serial.println("TestWiFiConnection: Connected");
  }
}

char MQTTConnect()
//connect to my SSID
{
  if (!mqttClient.connect(mqtt_broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    return (0);
  } else {
    return (1);
  }
}


void WiFiConnect()
//connect to my SSID
{
  status = WL_IDLE_STATUS;
  Serial.println("WiFiConnect: Awaiting Connection");
  while (status != WL_CONNECTED)
  {
    status = WiFi.begin(ssid, pass);
    if (status == WL_CONNECTED) {
      Serial.println("WiFiConnect: Connected");
    }
    else {
      Serial.print(".");
      delay(500);
    }
  }

  //mqttClient.setUsernamePassword("username", "password");
  Serial.print("Connecting to MQTT Broker");
  while (!MQTTConnect()) {
    Serial.print(".");
    delay(1000);
  }

  digitalWrite(connectionLED, HIGH); //LED ON to show connected
  Serial.println();

  Serial.println();
  Serial.println("Connected to WiFi and Broker!");
}

char ScanSSIDs()
//scan SSIDs, and if my SSID is present return 1
{
  Serial.println("ScanSSIDs: Scanning networks");
  char score = 0;
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1)
    return (0); //error

  for (int thisNet = 0; thisNet < numSsid; thisNet++)
    if (strcmp(WiFi.SSID(thisNet), ssid) == 0)
      score = 1; //if one is = to my SSID
  return (score);
}
