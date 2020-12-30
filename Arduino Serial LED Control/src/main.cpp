#include <Arduino.h>
#include <ArduinoJson.h> //https://arduinojson.org/

const int redPin = 8;
const int orangePin = 9;
const int greenPin = 10;

bool redState = false;
bool orangeState = false;
bool greenState = false;

void allOff()
{
  digitalWrite(redPin, LOW);
  redState = false;
  digitalWrite(orangePin, LOW);
  orangeState = false;
  digitalWrite(greenPin, LOW);
  greenState = false;
}

void allOn()
{
  digitalWrite(redPin, HIGH);
  redState = true;
  digitalWrite(orangePin, HIGH);
  orangeState = true;
  digitalWrite(greenPin, HIGH);
  greenState = true;
}

void toggle(int pin)
{
  switch (pin)
  {
  case redPin:
  {
    if (redState)
    {
      digitalWrite(redPin, LOW);
    }
    else
    {
      digitalWrite(redPin, HIGH);
    }
    redState = redState ? false : true;
    break;
  }
  case greenPin:
  {
    if (greenState)
      digitalWrite(greenPin, LOW);
    else
      digitalWrite(greenPin, HIGH);
    greenState = greenState ? false : true;
    break;
  }
  case orangePin:
  {
    if (orangeState)
      digitalWrite(orangePin, LOW);
    else
      digitalWrite(orangePin, HIGH);
    orangeState = orangeState ? false : true;

    break;
  }
  default:
    break;
  }
}

void setup()
{
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  pinMode(redPin, OUTPUT);
  pinMode(orangePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  allOn();
}

void loop()
{
  // send data only when you receive data:
  if (Serial.available() > 0)
  {
    String input = Serial.readString();
    input.trim();
    input.toUpperCase();

    if (input.equals("RED"))
      toggle(redPin);
    else if (input.equals("ORANGE"))
      toggle(orangePin);
    else if (input.equals("GREEN"))
      toggle(greenPin);
    else if (input.equals("OFF"))
      allOff();
    else if (input.equals("ON"))
      allOn();
    else
      Serial.println("Error 1: No mapping for input: " + input);

    DynamicJsonDocument doc(500);

    // Add values in the document
    doc["red"] = redState;
    doc["orange"] = orangeState;
    doc["green"] = greenState;

    // Generate the minified JSON and send it to the Serial port.
    serializeJson(doc, Serial);
    Serial.println(); //important
  }
}