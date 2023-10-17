#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define NAV_CHAR {0x00,0x08,0x04,0x02,0x04,0x08,0x00,0x00};

#define NAV_CHAR_CHOOSEN {0x00,0x08,0x04,0x02,0x04,0x08,0x00,0x00}

struct Interactable // or public interface struct IMediaPlayer 
{
    void Open(LiquidCrystal_I2C* instance);
    void Next();
    void Back();
    void Click(LiquidCrystal_I2C* instance);
};

struct Screen : Interactable // or public interface struct IMediaPlayer 
{
    void Open(LiquidCrystal_I2C* instance) {

    };
    void Next() {};
    void Back() {};
    void Click(LiquidCrystal_I2C* instance) {};
};

struct Menu : Interactable
{
    char header[20];
    byte cursorPosition;
    Interactable items[3] = malloc(sizeof(Interactable) * 3);
    
    Interactable parent;
    
    virtual void Open(LiquidCrystal_I2C instance){
        instance.clear();
    }

    virtual void Next(){
        //if (items[cursorPosition] != NULL && items[(cursorPosition + 1) % 3] != NULL){
            cursorPosition++;
        //}
    }

    virtual void Back(){
        //if (items[cursorPosition] != NULL && items[(cursorPosition - 1) % 3] != NULL){
            cursorPosition--;
        //}
    }

    virtual void Click(LiquidCrystal_I2C instance){
        items[cursorPosition].Open();
    }
};

struct TimeInputDialog : Interactable
{
    char header[20];
    Interactable parent;

    byte cursorPosition;

    byte hours_container;
    byte mins_container;


    uint8_t GetHours(){
        return getNumberFromByte(hours_container);
    }

    uint8_t GetMinutes(){
        return getNumberFromByte(mins_container);
    }


    virtual void Open(LiquidCrystal_I2C* instance){
        instance.clear();
    }

    virtual void Next(){
        if (items[cursorPosition] != NULL && items[(cursorPosition + 1) % 3] != NULL){
            cursorPosition++;
        }
    }

    virtual void Back(){
        if (items[cursorPosition] != NULL && items[(cursorPosition - 1) % 3] != NULL){
            cursorPosition--;
        }
    }

    virtual void Click(LiquidCrystal_I2C* instance){

    }
};


struct MenuItem : Interactable
{
    char text[19] = "Item";
    ItemAction action = ItemAction.NO_ACTION;
    Interactable item;
    bool isChecked = false;

    virtual void Click(LiquidCrystal_I2C* instance){
        switch (action)
        {
        case ItemAction.OPEN_INPUT || ItemAction.OPEN_MENU:
            item.Open(instance);
            break;
        case ItemAction.SWITCH:
            isChecked = !isChecked;
            break;
        }
    }

    virtual void Open(LiquidCrystal_I2C* instance){
        instance.clear();
    }

    virtual void Next(){
        if (items[cursorPosition] != NULL && items[(cursorPosition + 1) % 3] != NULL){
            cursorPosition++;
        }
    }

    virtual void Back(){
        if (items[cursorPosition] != NULL && items[(cursorPosition - 1) % 3] != NULL){
            cursorPosition--;
        }
    }
};

enum class ItemAction {
    NO_ACTION,
    OPEN_INPUT,
    OPEN_MENU,
    SWITCH
}
