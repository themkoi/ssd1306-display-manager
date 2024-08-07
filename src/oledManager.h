#ifndef OLEDMANAGER_H
#define OLEDMANAGER_H

#include <Arduino.h>

#include <OLED_SSD1306_Chart.h>

// Oled display constants
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SSD1306_I2C_ADDRESS 0x3C

// Enum for actions
typedef enum
{
    OLED_NO_ACTION,
    OLED_FADE_IN,
    OLED_FADE_OUT,
    OLED_DISPLAY,
    OLED_ENABLE,
    OLED_DISABLE,
    OLED_CUSTOM_COMMAND,
    OLED_SCROLL_LEFT,
    OLED_SCROLL_RIGHT,
    OLED_STOP_SCROLL
} Action;

// Struct for action data
typedef struct
{
    Action action;
    uint8_t param1;
    uint8_t param2;
    uint8_t param3;
} ActionData;

// OledManager class definition
class OledManager {
public:
    void oledDisplay();
    void oledFadeOut();
    void oledFadeIn();
    void oledDisable();
    void oledEnable();
    void startScrollingLeft(uint8_t startPage, uint8_t endPage, uint8_t speed);
    void startScrollingRight(uint8_t startPage, uint8_t endPage, uint8_t speed);
    void stopScrolling();
    void sendCustomCommand(uint8_t command);
    void createTask();

private:

    void handleAction(const ActionData& actionData);
};

extern OLED_SSD1306_Chart display;

extern OledManager manager;


#endif // OLEDMANAGER_H
