#include "Utils.h"

bool pinRead(uint8_t pin) {
  if (pin < 8) {
    return bitRead(PIND, pin);
  } else if (pin < 14) {
    return bitRead(PINB, pin - 8);
  } else if (pin < 20) {
    return bitRead(PINC, pin - 14);
  }
}

unsigned long minutesToMillis(byte minutes){
  return minutes * 60000;
}

unsigned long secondsToMillis(byte seconds){
  return seconds * 1000;
}