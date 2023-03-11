// Use 2nd circuit to measure temperature because of analog port noise error
//------ Library ------------
#include <Arduino.h>

//------- Define port -----------
#define tempSensor A0

//---------- Global variable ---------
int temp;

//------- Setup function --------
void setup() {
    pinMode(tempSensor, INPUT);
    Serial.begin(9600);
}

//------------ Main ---------
void loop() {
    temp = analogRead(tempSensor);
    // Formula change from Volt to Celsius
    temp = (temp / 1023.0) * 500;
    Serial.println(temp);
    // Update temp after 200ms
    delay(200);
}
