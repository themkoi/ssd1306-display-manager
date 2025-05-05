#ifndef OLEDMANAGER_H
#define OLEDMANAGER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

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
class OLED_MANAGER
{
public:
    void sendOledAction(Action action, uint8_t param1 = 0, uint8_t param2 = 0, uint8_t param3 = 0);
    void createTask();
    bool ScreenEnabled;
    bool dimmed;
    bool finishedDisplaying;
    bool scrolling;

private:
    void handleAction(const ActionData &actionData);
};

extern Adafruit_SSD1306 display;

extern OLED_MANAGER manager;

#endif // OLEDMANAGER_H
