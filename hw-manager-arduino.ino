// Configuration
byte speedMIN = 5;
byte speedMAX = 95;
byte tempMIN = 23;
byte tempMAX = 40;


// Pins
#define FAN_PIN 9      // fans pin
#define R_PIN 5        // red color
#define G_PIN 3        // green color
#define B_PIN 6        // blue color
#define SENSOR_PIN 14  // temperature sensor


// Libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimerOne.h>

#define TEMPERATURE_PRECISION 9


// Sensors configuration
OneWire oneWire(SENSOR_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress Thermometer1;


unsigned long timeout;
boolean updateTempFlag = 1;
int duty, LEDcolor;
int brightness, k, R, G, B, Rf, Gf, Bf;
byte mainTemp;

byte temp1;

char inData[82];  // массив входных значений (СИМВОЛЫ)
int PCdata[20];   // массив численных значений показаний с компьютера
byte index = 0;
String string_convert;
boolean hasSettingsFlag = 0;

void setup() {
  Serial.begin(9600);
  Timer1.initialize(40);  // PWM 25 kHz (40 microseconds)
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  digitalWrite(R_PIN, 0);
  digitalWrite(G_PIN, 0);
  digitalWrite(B_PIN, 0);

  sensors.begin();
  sensors.getAddress(Thermometer1, 0);
  sensors.setResolution(Thermometer1, TEMPERATURE_PRECISION);

  Timer1.pwm(FAN_PIN, 400);  // fans to 40%
  delay(2000);
}

void loop() {
  parsing();
  getTemperature();
  dutyCalculate();
  Timer1.pwm(FAN_PIN, duty * 10);
  LEDcontrol();
  timeoutTick();
}

void getTemperature() {
  if (updateTempFlag) {
    sensors.requestTemperatures();
    temp1 = sensors.getTempC(Thermometer1);
    updateTempFlag = 0;
  }
}
void LEDcontrol() {
  brightness = 100;
  LEDcolor = map(mainTemp, tempMIN, tempMAX, 0, 1000);
  LEDcolor = constrain(LEDcolor, 0, 1000);

  if (LEDcolor <= 500) {
    k = map(LEDcolor, 0, 500, 0, 255);
    R = 0;
    G = k;
    B = 255 - k;
  }
  if (LEDcolor > 500) {
    k = map(LEDcolor, 500, 1000, 0, 255);
    R = k;
    G = 255 - k;
    B = 0;
  }


  Rf = (brightness * R / 100);
  Gf = (brightness * G / 100);
  Bf = (brightness * B / 100);

  analogWrite(R_PIN, Rf);
  analogWrite(G_PIN, Gf);
  analogWrite(B_PIN, Bf);
}

void dutyCalculate() {
  if (temp1 < tempMIN) {
    duty = 0;
  } else {
    duty = map(temp1, tempMIN, tempMAX, speedMIN, speedMAX);
  }
}

void timeoutTick() {
  if (millis() - timeout > 500) {
    updateTempFlag = 1;
    // getTemperature();
    // Serial.print("temp = ");
    // Serial.println(temp1);
    // Serial.print("duty = ");
    // Serial.println(duty);
    // Serial.print("led = ");
    // Serial.println(LEDcolor);
    timeout = millis();
  }
}

void parsing() {
  while (Serial.available() > 0) {
    Serial.println("parsing ... ");
    char aChar = Serial.read();
    if (aChar != 'E') {
      inData[index] = aChar;
      index++;
      inData[index] = '\0';
    } else {
      char *p = inData;
      char *str;
      index = 0;
      String value = "";
      while ((str = strtok_r(p, ";", &p)) != NULL) {
        string_convert = str;
        PCdata[index] = string_convert.toInt();
        Serial.print("PCDATA[");
        Serial.print(index);
        Serial.print("] = ");
        Serial.println(PCdata[index]);
        index++;
      }
      index = 0;
      hasSettingsFlag = 1;
    }
  }
}
