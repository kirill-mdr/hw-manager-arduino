//PCdata[0] - is const speed
//PCdata[1] - const speed
//PCdata[2] - min temperature limit
//PCdata[3] - max temperature limit
//PCdata[4] - min speed limit
//PCdata[5] - max speed limit
//PCdata[6] - is dynamic light
//PCdata[7] - brightness
//PCdata[8] - red
//PCdata[10] - blue
byte speedMIN = 5;
byte speedMAX = 95;
byte tempMIN = 23;
byte tempMAX = 40;

// Pins
#define FAN_PIN 10     // fans pin
#define R_PIN 5        // red color
#define G_PIN 6        // green color
#define B_PIN 3        // blue color
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

byte temp1;

char inData[82];  // array for parsing
int PCdata[12];   // settings from PC
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
  if (hasSettingsFlag) {
    brightness = PCdata[7];
    LEDcolor = map(temp1, PCdata[2], PCdata[3], 0, 1000);
  } else {
    brightness = 100;
    LEDcolor = map(temp1, tempMIN, tempMAX, 0, 1000);
  }

  if (hasSettingsFlag && PCdata[6] == 0) {
    R = PCdata[8];
    G = PCdata[9];
    B = PCdata[10];
  } else {
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
  }

  Rf = (brightness * R / 100);
  Gf = (brightness * G / 100);
  Bf = (brightness * B / 100);

  analogWrite(R_PIN, Rf);
  analogWrite(G_PIN, Gf);
  analogWrite(B_PIN, Bf);
}

void dutyCalculate() {
  if (hasSettingsFlag) {
    if (PCdata[0] == 1) {
      duty = PCdata[1];
    } else {
      duty = map(temp1, PCdata[2], PCdata[3], PCdata[4], PCdata[5]);

      if (temp1 < PCdata[2]) duty = PCdata[4];
      else if (temp1 >= PCdata[3]) duty = PCdata[5];
    }
  } else {
    duty = map(temp1, tempMIN, tempMAX, speedMIN, speedMAX);

    if (temp1 < tempMin) duty = speedMIN;
    else if (temp1 >= tempMAX) duty = speedMAX;
  }
}

void timeoutTick() {
  if (millis() - timeout > 500) {
    updateTempFlag = 1;
    getTemperature();
    Serial.print("temp = ");
    Serial.println(temp1);
    Serial.print("duty = ");
    Serial.println(duty);
    Serial.print("led = ");
    Serial.println(LEDcolor);
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
        index++;
      }
      index = 0;
      hasSettingsFlag = 1;
    }
  }
}
