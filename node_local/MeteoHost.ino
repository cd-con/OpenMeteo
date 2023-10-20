#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <SoftwareSerial.h>
#include <DHT.h>

RTC_DS1307 rtc;
SoftwareSerial _radio = SoftwareSerial(2,3);
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
DHT am2303(12, DHT22);

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

struct ClientPacket{
    // Размер пакета = 14 байт
    // При скорости 9600 бод (960 байт/сек)
    // Передача 1 пакета займёт ~15мс
    float temperature;
    float moisture;
    float windSpeed;
    float pressure;
};

struct ServerPacket{
  uint32_t measurementInterval;
};

class MenuNumber{
  public:
    String ItemName = "Item";
    int8_t value = 0;
    int8_t maxVal = 59;
    int8_t minVal = 0;

    MenuNumber(String name, int8_t val = 0, int8_t maxx = 59, int8_t minn = 0){
      ItemName = name;
      value = val;
      maxVal = maxx;
      minVal = minn;
    }

    void Increment(){
      value++;
      if (value > maxVal){
        value = minVal;
      }
    }

    void Decrement(){
      value--;
      if (value < minVal){
        value = maxVal;
      }
    }

    String Print(){
      return String((byte)(value / 10)) + String(value % 10);
    }
};
MenuNumber empty(" ");
MenuNumber menu[] = {
  MenuNumber("Hours", 0, 23, 0),
  MenuNumber("Minutes"),
  MenuNumber("Refresh rate", 30, 60, 5)
};

MenuNumber selected = empty;
byte cursor = 255;

bool inMenu = false; 
bool r = false;

void setup()
{
  lcd.init();
  lcd.backlight();
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  DateTime now = rtc.now();
  menu[0].value = now.hour();
  menu[1].value = now.minute();

  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);

  
  lcd.createChar(0, moist);
  lcd.createChar(1, celsius);

  lcd.setCursor(7,0);
  lcd.print("Meteo");
  delay(1000);
  am2303.begin();
  r = true;
}


uint32_t dht_timer = 310;
uint32_t dht_rate = 300;
float temp = 0;
float hum = 0;
ClientPacket last;

void loop()
{
  if(_radio.available() && _radio.readBytes((byte*)&last, sizeof(last))){
    r = true;
  }

  dht_timer++;

  if (dht_timer >= dht_rate){
    dht_timer = 0;
    temp = am2303.readTemperature();
    hum = am2303.readHumidity();
    r = true;
  }
  if (digitalRead(9) == LOW){
    r = true;
    if (selected.ItemName != " "){
      selected.Decrement();
    }else{
      cursor--;
      inMenu = cursor >= 0 && cursor < 3;    
    }
  }

  if (digitalRead(10) == LOW && inMenu){
    r = true;
    if (selected.ItemName != " "){
      menu[cursor] = selected;
      if(cursor == 2){
        ServerPacket packet;
        packet.measurementInterval = selected.value * 1000;
        dht_rate = selected.value * 10;
        _radio.write((byte*)&packet, sizeof(packet));
      }

      if (cursor > 0 && cursor < 3){
            // January 21, 2014 at 3am you would call:
            rtc.adjust(DateTime(2023, 1, 1, menu[0].value, menu[1].value, 0));
      }
      selected = empty;
    }else{
      selected = menu[cursor];
    }
  }


  if (digitalRead(11) == LOW){
    r = true;
    if (selected.ItemName != " "){
      selected.Increment();
    }else{
      cursor++;
      inMenu = cursor < 3;    
    }
  }

  if (cursor == 4){
    r = true;
    cursor = 255;
  }
  if (cursor == 254){
    r = true;
    cursor = 3;
  }

  if(r){
    lcd.clear();
    if (inMenu){
      lcd.print("Settings");
      lcd.setCursor(0,1);
      if (selected.ItemName != " "){
        lcd.print("> " + selected.ItemName + "  <" + selected.Print() + ">");
      }else{
        lcd.print("> " + menu[cursor].ItemName + " = " + menu[cursor].Print());
      }
      if (cursor + 1 <= sizeof(menu)){
        lcd.setCursor(0,2);
        lcd.print(menu[cursor + 1].ItemName);
      }
    }else{
      DateTime now = rtc.now();
      menu[0].value = now.hour();
      menu[1].value = now.minute();
      if (cursor == 3){
        lcd.print("Local | " + menu[0].Print() + ":" + menu[1].Print());
        lcd.setCursor(0,1);
        lcd.print(temp);
        lcd.write(1);
        lcd.setCursor(9,1);
        lcd.write(0);
        lcd.print(String(hum) + "%");
      }else{
        lcd.print("Remote | " + menu[0].Print() + ":" + menu[1].Print());
        lcd.setCursor(0,1);
        lcd.print(last.temperature);
        lcd.write(1);
        lcd.setCursor(9,1);
        lcd.write(0);
        lcd.print(String(last.moisture) + "%");
        lcd.setCursor(0,2);
        lcd.print(String(last.pressure * 7.5006f) + " mmhg");
        lcd.setCursor(0,3);
        byte lvl = bofortConvert(last.windSpeed);
        lcd.print(String(last.windSpeed) + " m/s " + String(msgBofort(lvl)));
      }

    }
    r = false;
  }
  delay(100);
}

float wLevel[] = {0.2f, 1.5f, 3.3f, 5.4f, 7.9f, 10.7f, 13.8f, 17.1f, 20.7f, 24.4f, 28.4f, 32.6f};
byte bofortConvert(float w){
  for (byte i = 0; i < 12; i++){
    if (w <= wLevel[i]){
      return i;
    }
  }
  return 12;
}

String msgBofort(byte lvl){
  switch (lvl){
    case 0:
      return "Calm     ";
      break;
    case 1:
      return "Quiet    ";
      break;
    case 2:
      return "Light    ";
      break;
    case 3:
      return "Weak     ";
      break;
    case 4:
      return "Moderate ";
      break;
    case 5:
      return "Strong   ";
      break;
    case 6:
      return "Strong   ";
      break;
    case 7:
      return "Strong   ";
      break;
    case 8:
      return "V. Strong";
      break;
    case 9:
      return "Storm    ";
      break;
    case 10:
      return "S.Storm  ";
      break;
    case 11:
      return "H.Storm  ";
      break;
    case 12:
      return "Hurricane";
      break;
    default:
      return "N/A";
      break;
  }
}
