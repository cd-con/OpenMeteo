#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

#include <SoftwareSerial.h>

struct FixedPoint{
    short value; 
    short scale; 

    short getIntegral(FixedPoint num) const {
        return value / (10 ^ scale);
    }

    short getFractional() const {
        return value % (10 ^ scale);
    }

    double getValue() const {
        return (double)value / pow(10.0, scale);
    }
};

struct ClientPacket{
    FixedPoint temperature;
    FixedPoint moisture;
    FixedPoint windSpeed;
    FixedPoint pressure;
};

struct ServerPacket{
  byte packetType;
  uint32_t measurementInterval;
};

bool menuMode = false;
bool dialogMode = false;
bool editingNum = false;
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

Screen screenCursorPosition = Screen::REMOTE;
MenuItem menuCursorPosition;


#define NEXT 8
#define PREV 9
#define OK  10

LiquidCrystal_I2C lcd(0x27, 20, 4);
RTC_DS1307 watch;
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

class CoolInput2{
  public:
    byte a;
    byte maxA;
    byte b;
    byte maxB;

    CoolInput2(uint8_t initNum = 0, uint8_t maxA = 0, uint8_t maxB = 0){
      a = (int)(initNum / 10);
      b = initNum % 10;
    }

    void Increment(){
      a++;
      if (a > maxA){
        b++;
        a = 0;
      }
      if (b > maxB){
        a = 0;
        b = 0;
      }
    }

    void Decrement(){
      a--;
      if (a == 0){
        b--;
        a = maxA;
      }
      if (b == 0){
        a = maxA;
        b = maxB;
      }
    }

    uint8_t toInt(){
      return a * 10 + b;
    }
};

#define TIME_MAX_CUR 2;
#define DATE_MAX_CUR 3;
#define INTERNAL_MAX_CUR 2;

CoolInput2 minute;
CoolInput2 hour;
CoolInput2 day;
CoolInput2 month;
CoolInput2 year;

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

  watch.begin();
  watch.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

ClientPacket lastPacket;

unsigned long backlightTimer = 15000;
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
        if (dialogMode){
          if(editingNum){
            
          }
        }
        if (menuCursorPosition < 3){
          menuCursorPosition = (int)menuCursorPosition + 1;
        }
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
      }else if(dialogMode){
        switch(menuCursorPosition){
            case MenuItem::TIME:
              if ((int)menuCursorPosition == TIME_MAX_CUR){
                dialogMode = false;
              }     
            break;  
            case MenuItem::DATE:
              if ((int)menuCursorPosition == DATE_MAX_CUR){
                dialogMode = false;
              }     
            break;  

        }
        editingNum = !editingNum;
      }
      else{
        if((int)menuCursorPosition != 3){
            menuCursorPosition = 0;
            dialogMode = true;
        }else{
            menuMode = false;
        }
      }
    }
  }

  if(digitalRead(PREV) == LOW){
    needRedraw = true;
    ResetBacklight();
    // Код реакции на кнопку прев
    if (!isSleep){
      if (menuMode){
        if (menuCursorPosition > 0){
          menuCursorPosition = (int)menuCursorPosition - 1;
        }
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
  DateTime now = watch.now();
  if (!menuMode){
    switch(screenCursorPosition){
      case Screen::REMOTE:
          lcd.setCursor(0,0);
          lcd.print("Outdoors | " + now.hour() + ':' + now.minute());
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
        lcd.print("Indoors");
        break;
    }
  }else{
    if (dialogMode){
      lcd.blink();
      switch (menuCursorPosition){
          case MenuItem::TIME:
            lcd.print("Set time");
            // Вывод времени
            
            break;
          case MenuItem::DATE:
            break;
          case MenuItem::UPDATE_RATE:
            break;
          case MenuItem::EXIT:
            lcd.noBlink();
            dialogMode = false;
            break;
        }
          lcd.setCursor(18,3);
          lcd.print("x");
          //lcd.setCursor(3,18);
    }else{
    lcd.print("Settings");
    lcd.setCursor(0,1);
    lcd.print('>' + getLocale(menuCursorPosition));
    lcd.setCursor(1,2);
    lcd.print(getLocale(menuCursorPosition + 1));
    }
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
      return "";
      break;
  }
}