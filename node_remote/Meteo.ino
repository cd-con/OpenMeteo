#include <Adafruit_BME280.h>
#include <GyverPower.h>
#include <powerConstants.h>
#include <SoftwareSerial.h>

unsigned long nextMeasurementAt;
unsigned int measurementInterval = 5000;

SoftwareSerial radio(2,3);
Adafruit_BME280 bme;


struct ClientPacket{
    // Размер пакета = 14 байт
    // При скорости 9600 бод (960 байт/сек)
    // Передача 1 пакета займёт ~15мс
    float temperature;
    float moisture;
    float windSpeed;
    float windDirection;
    float pressure;
};

struct ServerPacket{
  uint32_t measurementInterval;
};


void setup() {
  pinMode(13, OUTPUT);

  radio.begin(9600);
  bme.begin(0x76);

  Serial.begin(9600);

  power.autoCalibrate();

  digitalWrite(13, 0);
}

long c = 0;
void loop() { 
    ServerPacket last;
    if(_radio.available() && _radio.readBytes((byte*)&last, sizeof(last))){
      nextMeasurementAt = last.measurementInterval;
    }

    if (c >= nextMeasurementAt){
      c = 0;
      digitalWrite(13, 1);

      ClientPacket packet;

      packet.moisture = bme.readHumidity();

      packet.pressure = bme.readPressure() * 0.75f;

      packet.temperature = bme.readTemperature();

      packet.windSpeed= map(analogRead(A0), 0, 1024, 0, 32768) / 10;
      packet.windDirection= map(analogRead(A1), 0, 1024, 0, 360);

      radio.write((byte*)&packet, sizeof(packet));
      nextMeasurementAt += measurementInterval;
      digitalWrite(13, 0);
    }
    //power.sleep(SLEEP_1024MS);
}
