#include <LiquidCrystal_I2C.h>

#include <SoftwareSerial.h>

bool menuMode = false;
bool needRedraw = false;

enum Screen{
  REMOTE,
  LOCAL
};

enum MenuItem{
  TIME,
  DATE,
  UPDATE_RATE,
  EXIT
};

Screen screenCursorPosition;
MenuItem menuCursorPosition;

#define NEXT 8
#define PREV 9
#define OK  10

// функция для настройки PCINT
uint8_t attachPCINT(uint8_t pin) {
  if (pin < 8) {            // D0-D7 (PCINT2)
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << pin); return 2; } else if (pin > 13) {    //A0-A5 (PCINT1)
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << pin - 14);
    return 1;
  } else {                  // D8-D13 (PCINT0)
    PCICR |= (1 << PCIE0);
    PCMSK0 |= (1 << pin - 8);
    return 0;
  }
}
// быстрый digitalRead для опроса внутри ISR
// пригодится для проверки конкретного пина
bool pinRead(uint8_t pin) {
  if (pin < 8) {
    return bitRead(PIND, pin);
  } else if (pin < 14) {
    return bitRead(PINB, pin - 8);
  } else if (pin < 20) {
    return bitRead(PINC, pin - 14);
  }
}

#include "Packet.h"
//#include "CustomMenu.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);
SoftwareSerial _radio = SoftwareSerial(2,3);

// Chars

byte moist[] = {
  0x04,
  0x04,
  0x0A,
  0x0A,
  0x13,
  0x11,
  0x19,
  0x0E
};

byte celsius[] = {
  0x03,
  0x03,
  0x00,
  0x0C,
  0x10,
  0x10,
  0x10,
  0x0C
};

byte min_a = 0;
byte min_b = 0;
byte hour = 0;

void setup(){
  _radio.begin(9600);
  Serial.begin(115200);

  pinMode(NEXT, INPUT_PULLUP);
  pinMode(PREV, INPUT_PULLUP);
  pinMode(OK, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, moist);
  lcd.createChar(1, celsius);
}

ClientPacket lastPacket;

unsigned long backlightTimer = 0;
uint32_t backlightTime = 10000;

void loop(){
  bool isSleep = millis() >= backlightTimer;
  if (isSleep){
    lcd.noBacklight();
  }
  if(digitalRead(NEXT) == LOW){
    needRedraw = true;
    ResetBacklight();
    // Код реакции на кнопку некст
    if (!isSleep){
      if (menuMode){
        menuCursorPosition = ((int)menuCursorPosition + 1) % 4;
      }else{
        screenCursorPosition = ((int)screenCursorPosition + 1) % 2;
      }
    }
  }

  if(digitalRead(OK) == LOW){
    needRedraw = true;
    ResetBacklight();
    if (!isSleep){
      if (!menuMode){
        menuMode = true;
        menuCursorPosition = MenuItem::TIME;
      }
      
      if(menuCursorPosition == MenuItem::EXIT){
        menuMode = false;
      }
    }
  }

  if(digitalRead(PREV) == LOW){
    needRedraw = true;
    ResetBacklight();
    // Код реакции на кнопку прев
    if (!isSleep){
      if (menuMode){
        //menuCursorPosition = abs(((int)menuCursorPosition - 5) % 4);
      }else{
        screenCursorPosition = abs(((int)screenCursorPosition - 3) % 2);
      }
    }
  }

  if (needRedraw){
    needRedraw = false;
    ScreenDisplay();
  }

  needRedraw = _radio.readBytes((byte*)&lastPacket, sizeof(lastPacket));  
}

void ResetBacklight(){
    lcd.backlight();
    backlightTimer = millis() + backlightTime;
}

void ScreenDisplay(){
  lcd.clear();
  if (!menuMode){
    switch(screenCursorPosition){
      case Screen::REMOTE:
          FixedPoint val;
          for (byte i = 0; i < 2; i++){
            switch(i){
              case 0:
                val = lastPacket.temperature;
                lcd.setCursor(0,1);
                break;
              case 1:
                val = lastPacket.moisture;
                lcd.setCursor(0,2);
                break;
            }
            if (i == 1){
              lcd.write(0);
            }
            lcd.print(val.getValue());
            if (i == 0){
              lcd.write(1);
            }
            if (i == 1){
              lcd.print("%");
            }
          }
        break;
      case Screen::LOCAL:
        lcd.print("Local screen");
        break;
    }
  }else{
    lcd.clear();
    lcd.print("Settings");
    lcd.setCursor(0,1);
    lcd.print(getLocale(menuCursorPosition));
  }
}

String getLocale(MenuItem item){
  switch(item){
    case MenuItem::TIME:
      return "Set time";
      break;
    case MenuItem::DATE:
      return "Set date";
      break;
    case MenuItem::UPDATE_RATE:
      return "Set sensor d/rate";
      break;
    case MenuItem::EXIT:
      return "Exit menu";
      break;
    default:
      return "NO_LOCALE";
      break;
  }
}