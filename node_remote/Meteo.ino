#include <Adafruit_BME280.h>
#include <GyverPower.h>
#include <powerConstants.h>
#include <SoftwareSerial.h>
#include "Packet.h"

unsigned long nextMeasurementAt;
unsigned int measurementInterval = 5000;

SoftwareSerial radio(2,3);
Adafruit_BME280 bme;


void setup() {
  pinMode(13, OUTPUT);

  radio.begin(9600);
  bme.begin(0x76);

  Serial.begin(9600);

  power.autoCalibrate();

  digitalWrite(13, 0);
}

void loop() {   
    if (millis() >= nextMeasurementAt){
      digitalWrite(13, 1);

      ClientPacket packet;

      packet.moisture.value = bme.readHumidity() * 100;
      packet.moisture.scale = 2;

      packet.pressure.value = bme.readPressure() * 0.75f;
      packet.pressure.scale = 2;

      packet.temperature.value = bme.readTemperature() * 10;
      packet.temperature.scale = 1;

      packet.windSpeed.value = 10;
      packet.windSpeed.scale = 1;

      radio.write((byte*)&packet, sizeof(packet));
      nextMeasurementAt += measurementInterval;
      digitalWrite(13, 0);
    }
    //power.sleep(SLEEP_1024MS);
}
