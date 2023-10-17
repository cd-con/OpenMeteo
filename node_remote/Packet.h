#pragma once
#include <Arduino.h>
#include "FixedPoint.h"

struct ClientPacket{
    // Размер пакета = 14 байт
    // При скорости 9600 бод (960 байт/сек)
    // Передача 1 пакета займёт ~15мс
    FixedPoint temperature;
    FixedPoint moisture;
    FixedPoint windSpeed;
    FixedPoint pressure;
};

struct ServerPacket{
  byte packetType;
  uint32_t measurementInterval;
};