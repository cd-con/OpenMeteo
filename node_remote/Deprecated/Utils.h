#pragma once
#include <Arduino.h>

class List {
public:
    byte length;
    byte data[16];
    void append(byte item) {
        if (length < 16) data[length++] = item;
    }
    void remove(byte index) {
        if (index >= length) return;
        memmove(&data[index], &data[index+1], length - index - 1);
        length--;
    }

    /*bool contains(byte item){
      for(uint32_t i = 0; i < data.length){

      }
    }*/
};

bool pinRead(uint8_t pin);

unsigned long minutesToMillis(byte minutes);
unsigned long secondsToMillis(byte minutes);