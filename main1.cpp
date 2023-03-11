//------ Library ------------
#include <Arduino.h>
#include <LiquidCrystal.h>

//------- Define port -----------
//  output
LiquidCrystal lcd(10, 9, 7, 6, 5, 4);
#define ledTemp A5
#define ledLight A3
#define speaker 3
//  input
#define buttonUp 13
#define buttonDown 12
#define buttonEnter 11
#define buttonExit 8
#define photoresistor A0

//---------- Global variable ---------
int mainScreen = 0, sumLight = 0, sumTemp = 0, mode = 0, test = 0;
int index1 = 0, index2 = 0, readM = 0;
int up, down, enter, exitM, tmpUp, tmpDown, tmpEnter, tmpExit;
int avgLight, avgTemp, temp, light, wait = 1, start = 1;
unsigned long time, overHeat, heater;
int alpha = 1, setTemp = 27, setLight = 250;
int arrayLight[20], arrayTemp[20];

// ---------- Support function -------------
void printValue(int number, int numOfDigit) {
    int n = number, countDigit = 0;
    if (n == 0)
        countDigit = 1;
    else
        while (n != 0) {
            n /= 10;
            ++countDigit;
        }
    lcd.print(number);
    // print (blank) to delete digit from previous output
    for (int i = 0; i < numOfDigit - countDigit; ++i)
        lcd.print(" ");
}

void lcdPrint() {
    lcd.clear();
    switch (mode) {
    case 1:
        lcd.setCursor(0, 0);
        lcd.print("CaiDatNhietDo");
        break;
    case 2:
        lcd.setCursor(0, 0);
        lcd.print("CaiDatAnhSang");
        break;
    case 3:
        lcd.setCursor(0, 0);
        lcd.print("KiemTraThietBi");
        break;
    default:
        switch (mainScreen) {
        case 1:
            lcd.setCursor(2, 0);
            lcd.print("CaiDatNhietDo");
            lcd.setCursor(0, 1);
            lcd.print(">>CaiDatAnhSang");
            break;
        case 2:
            lcd.setCursor(2, 0);
            lcd.print("CaiDatAnhSang");
            lcd.setCursor(0, 1);
            lcd.print(">>KiemTraThietBi");
            break;
        default:
            lcd.setCursor(0, 0);
            lcd.print("T:");
            lcd.setCursor(8, 0);
            lcd.print("L:");
            lcd.setCursor(0, 1);
            lcd.print(">>CaiDatNhietDo");
            break;
        }
        break;
    }
}

//------- Setup function --------
void setup() {
    time = millis();

    pinMode(ledTemp, OUTPUT);
    pinMode(ledLight, OUTPUT);
    pinMode(speaker, OUTPUT);

    pinMode(buttonUp, INPUT_PULLUP);
    pinMode(buttonDown, INPUT_PULLUP);
    pinMode(buttonEnter, INPUT_PULLUP);
    pinMode(buttonExit, INPUT_PULLUP);
    pinMode(photoresistor, INPUT_PULLUP);

    lcd.begin(16, 2);
    lcdPrint();
    Serial.begin(9600);
}

//------------ Main ---------
void loop() {
    // real time
    if ((unsigned long)(millis() - time) >= 1000) {
        time += 1000;
    }
    // 1. Collecting input data
    up = digitalRead(buttonUp);
    down = digitalRead(buttonDown);
    enter = digitalRead(buttonEnter);
    exitM = digitalRead(buttonExit);

    if (Serial.available() > 0) {
        temp = Serial.parseInt();
        // Add new temp value
        sumTemp += temp;
        // Remove oldest temp value
        sumTemp -= arrayTemp[index1];
        arrayTemp[index1++] = temp;
        // Average temperature to handle noise
        avgTemp = sumTemp / 20;
        index1 %= 20; // The value of index will be in the range 0->19,
                      // Similar: if ( index>20 ) index = 0;
        Serial.println(temp);
    }

    light = analogRead(photoresistor);
    light = (light / 1023.0) * 500;
    sumLight += light;
    sumLight -= arrayLight[index2];
    arrayLight[index2++] = light;
    avgLight = sumLight / 20;
    index2 %= 20;

    // 2. Processing
    //  handle feature access
    if (enter == 0)
        tmpEnter = 1;
    // Handle button press noise by taking the time of button release
    if (tmpEnter == 1 && enter == 1) {
        mode = mainScreen + 1;
        tmpEnter = 0;
        lcdPrint();
    }
    if (exitM == 0)
        tmpExit = 1;
    if (tmpExit == 1 && exitM == 1) {
        mode = 0;
        test = 0;
        digitalWrite(ledTemp, 0);
        digitalWrite(ledLight, 0);
        digitalWrite(speaker, 0);

        // Press exit to turn off the overheat alarm
        overHeat = time;
        wait = 1;

        tmpExit = 0;
        lcdPrint();
    }

    // button function for each mode
    switch (mode) {
    case 1:
        if (up == 0)
            tmpUp = 1;
        if (tmpUp == 1 && up == 1) {
            setTemp = setTemp + 1 % 100;
            tmpUp = 0;
        }
        if (down == 0)
            tmpDown = 1;
        if (tmpDown == 1 && down == 1) {
            --setTemp;
            if (setTemp < 0)
                setTemp = 99;
            tmpDown = 0;
        }
        break;
    case 2:
        if (up == 0)
            tmpUp = 1;
        if (tmpUp == 1 && up == 1) {
            setLight = ++setLight % 100;
            tmpUp = 0;
        }
        if (down == 0)
            tmpDown = 1;
        if (tmpDown == 1 && down == 1) {
            --setLight;
            if (setLight < 0)
                setLight = 500;
            tmpDown = 0;
        }
        break;
    case 3:
        if (up == 0)
            tmpUp = 1;
        if (tmpUp == 1 && up == 1) {
            --test;
            if (test < 0)
                test = 3;
            tmpUp = 0;
        }
        if (down == 0)
            tmpDown = 1;
        if (tmpDown == 1 && down == 1) {

            test = ++test % 4;
            tmpDown = 0;
        }
        break;

    default:
        if (up == 0)
            tmpUp = 1;
        if (tmpUp == 1 && up == 1) {
            if (mainScreen == 0) {
                mainScreen = 2;
            } else {
                --mainScreen;
            }
            tmpUp = 0;
            lcdPrint();
        }
        if (down == 0)
            tmpDown = 1;
        if (tmpDown == 1 && down == 1) {
            if (mainScreen == 2) {
                mainScreen = 0;
            } else {
                ++mainScreen;
            }
            tmpDown = 0;
            lcdPrint();
        }
        break;
    }

    // 3. ouput
    lcd.setCursor(0, 1);
    switch (mode) {
    case 1:
        printValue(setTemp, 3);
        break;
    case 2:
        printValue(setLight, 3);
        break;
    case 3:
        if (test == 0) {
            lcd.print("ChonThietBi   ");
            digitalWrite(ledTemp, 0);
            digitalWrite(ledLight, 0);
            digitalWrite(speaker, 0);
        }
        if (test == 1) {
            lcd.print("DenMaySuoi    ");
            digitalWrite(ledTemp, 1);
            digitalWrite(ledLight, 0);
            digitalWrite(speaker, 0);
        }
        if (test == 2) {
            lcd.print("DenAnhSang    ");
            digitalWrite(ledTemp, 0);
            digitalWrite(ledLight, 1);
            digitalWrite(speaker, 0);
        }
        if (test == 3) {
            lcd.print("LoaCanhBao    ");
            digitalWrite(ledTemp, 0);
            digitalWrite(ledLight, 0);
            digitalWrite(speaker, 1);
        }
        break;

    default:
        if (mainScreen == 0) {
            lcd.setCursor(2, 0);
            printValue(avgTemp, 3);
            lcd.setCursor(11, 0);
            printValue(avgLight, 3);
        }

        // Check day night
        if (avgLight > setLight) {
            // Turn off light
            digitalWrite(ledLight, 0);

            // Check temp
            if (avgTemp <= setTemp - alpha) {
                // Turn on heater
                digitalWrite(ledTemp, 1);
            }
            if (avgTemp >= setTemp + alpha) {
                // Turn off heater
                digitalWrite(ledTemp, 0);
            }
        } else {
            // Turn on light
            digitalWrite(ledLight, 1);

            // if temp is low then turn on heater in 5m then turn off after that
            // wait 5m then check temp again
            if (avgTemp < setTemp) {
                if (start == 1) {
                    heater = time;
                    Serial.println(heater);
                    start = 0;
                }
                if (wait == 0) {
                    digitalWrite(ledTemp, 1);
                    if (time - heater > 5000) {
                        digitalWrite(ledTemp, 0);
                        wait = 1;
                        start = 1;
                    }
                } else {
                    if (time - heater > 5000) {
                        wait = 0;
                        start = 1;
                    }
                }
            } else {
                if (time - heater > 5000) {
                    digitalWrite(ledTemp, 0);
                }
                wait = 0;
            }
        }

        // Check if overheat
        if (avgTemp < setTemp + 2) {
            overHeat = time;
        }
        // After 5 minute overheating turn on alarm
        if (time - overHeat > 5000) {
            digitalWrite(speaker, 1);
        }
        break;
    }
}