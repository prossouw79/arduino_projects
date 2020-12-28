//https://www.circuitbasics.com/arduino-basics-controlling-led/
#include <ArduinoJson.h>

const int red = 8;
const int orange = 9;
const int green = 10;

bool redState = false;
bool orangeState = false;
bool greenState = false;

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  pinMode(red, OUTPUT);
  pinMode(orange, OUTPUT);
  pinMode(green, OUTPUT);
  allOn();
}

void loop() {
  // send data only when you receive data:
  if (Serial.available() > 0) {
    String input = Serial.readString();
    input.trim();
    input.toUpperCase();

    if (input.equals("RED"))
      toggleRed();
    else if (input.equals("ORANGE"))
      toggleOrange();
    else if (input.equals("GREEN"))
      toggleGreen();
    else if (input.equals("OFF"))
      allOff();
    else if (input.equals("ON"))
      allOn();
    else
      Serial.println("Error 1: No mapping for input: " + input);

    //StaticJsonDocument<200> doc;
  
    // StaticJsonObject allocates memory on the stack, it can be
    // replaced by DynamicJsonDocument which allocates in the heap.
    //
    DynamicJsonDocument  doc(500);
  
    // Add values in the document
    doc["red"] = redState;
    doc["orange"] = orangeState;
    doc["green"] = greenState;
  
    // Generate the minified JSON and send it to the Serial port.
    //
    serializeJson(doc, Serial);
    Serial.println();//important
  }
}

void allOff() {
  digitalWrite(red, LOW);
  redState = false;
  digitalWrite(orange, LOW);
  orangeState = false;
  digitalWrite(green, LOW);
  greenState = false;
}

void allOn() {
  digitalWrite(red, HIGH);
  redState = true;
  digitalWrite(orange, HIGH);
  orangeState = true;
  digitalWrite(green, HIGH);
  greenState = true;
}

void toggleRed() {
  if (redState) {
    digitalWrite(red, LOW);
  }
  else {
    digitalWrite(red, HIGH);
  }
  redState = redState ? false : true;
}

void toggleOrange() {
  if (orangeState)
    digitalWrite(orange, LOW);
  else
    digitalWrite(orange, HIGH);
  orangeState = orangeState ? false : true;
}

void toggleGreen() {
  if (greenState)
    digitalWrite(green, LOW);
  else
    digitalWrite(green, HIGH);
  greenState = greenState ? false : true;
}
