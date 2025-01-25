#include "oledManager.h"

OLED_MANAGER manager;

// Queue and mutex handles
QueueHandle_t actionQueue;
SemaphoreHandle_t actionMutex;

bool displaying = false;
bool waitingToDisplay = false;
bool fading = false;
bool waitingToFadein = false;
bool waitingToFadeout = false;

void OLED_MANAGER::oledDisplay()
{
    Action action = OLED_DISPLAY;
    manager.finishedDisplaying = false;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void OLED_MANAGER::oledFadeOut()
{
    Action action = OLED_FADE_OUT;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void OLED_MANAGER::oledFadeIn()
{
    Action action = OLED_FADE_IN;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void OLED_MANAGER::oledDisable()
{
    if (manager.ScreenEnabled == true)
    {
        Action action = OLED_DISABLE;
        xQueueSend(actionQueue, &action, portMAX_DELAY);
    }
}

void OLED_MANAGER::oledEnable()
{
    if (manager.ScreenEnabled == false)
    {
        Action action = OLED_ENABLE;
        xQueueSend(actionQueue, &action, portMAX_DELAY);
    }
}

void OLED_MANAGER::sendCustomCommand(uint8_t command)
{
    ActionData actionData;
    actionData.action = OLED_CUSTOM_COMMAND;
    actionData.param1 = command;
    xQueueSend(actionQueue, &actionData, portMAX_DELAY);
}

void OLED_MANAGER::stopScrolling()
{
    ActionData actionData;
    actionData.action = OLED_STOP_SCROLL;
    xQueueSend(actionQueue, &actionData, portMAX_DELAY);
}

void OLED_MANAGER::startScrollingLeft(uint8_t startPage, uint8_t endPage, uint8_t speed)
{
    ActionData actionData;
    actionData.action = OLED_SCROLL_LEFT;
    actionData.param1 = startPage;
    actionData.param2 = endPage;
    actionData.param3 = speed;
    xQueueSend(actionQueue, &actionData, portMAX_DELAY);
}

void OLED_MANAGER::startScrollingRight(uint8_t startPage, uint8_t endPage, uint8_t speed)
{
    ActionData actionData;
    actionData.action = OLED_SCROLL_RIGHT;
    actionData.param1 = startPage;
    actionData.param2 = endPage;
    actionData.param3 = speed;
    xQueueSend(actionQueue, &actionData, portMAX_DELAY);
}

// Communication to the display
void disableImplementation()
{
    if (manager.ScreenEnabled == true)
    {
        manager.ScreenEnabled = false;
        delay(100);
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        delay(100);
    }
}

void oledEnableImplementation()
{
    if (manager.ScreenEnabled == false)
    {
        manager.ScreenEnabled = true;
        delay(100);
        display.ssd1306_command(SSD1306_DISPLAYON);
        delay(100);
    }
}

void oledDisplayImplementation()
{
    vTaskDelay(pdMS_TO_TICKS(1));
    display.display();
    manager.finishedDisplaying = true;
    vTaskDelay(pdMS_TO_TICKS(1));
}

void oledFadeOutImplementation()
{
    if (fading || displaying)
        return; // Check if already fading or displaying

    fading = true; // Set fading flag to true

    delay(100);
    // Fade out
    for (int dim = 150; dim >= 1; dim -= 10)
    {
        display.ssd1306_command(0x81); // Contrast control command
        display.ssd1306_command(dim);  // Set contrast value
        delay(10);
    }

    delay(50);

    for (int dim2 = 34; dim2 >= 1; dim2 -= 17)
    {
        display.ssd1306_command(0xD9); // Pre-charge period command
        display.ssd1306_command(dim2); // Set pre-charge period
        delay(20);
    }
    delay(100);

    fading = false; // Reset fading flag
}

void oledFadeInImplementation()
{
    fading = true; // Set fading flag to true

    delay(100);
    // Fade in
    for (uint8_t dim = 1; dim <= 160; dim += 10)
    {
        display.ssd1306_command(0x81);
        display.ssd1306_command(dim);
        delay(10);
    }

    delay(50);

    for (uint8_t dim2 = 1; dim2 <= 34; dim2 += 17)
    {
        display.ssd1306_command(0xD9);
        display.ssd1306_command(dim2);
        delay(30);
    }
    delay(100);

    fading = false; // Reset fading flag
}

void sendCommand(uint8_t command)
{
    delay(10);
    display.ssd1306_command(command);
    delay(10);
}

// Function to start horizontal scroll left with customizable speed
void startScrollLeftImplementation(uint8_t startPage, uint8_t endPage, uint8_t speed)
{
    uint8_t interval;
    switch (speed)
    {
    case 5:
        interval = 0b000;
        break;
    case 64:
        interval = 0b001;
        break;
    case 128:
        interval = 0b010;
        break;
    case 256:
        interval = 0b011;
        break;
    case 3:
        interval = 0b100;
        break;
    case 4:
        interval = 0b101;
        break;
    case 25:
        interval = 0b110;
        break;
    case 2:
        interval = 0b111;
        break;
    default:
        interval = 0b000;
        break;
    }
    delay(10);

    // Command for horizontal scroll left
    sendCommand(0x27);      // Command for left horizontal scroll
    sendCommand(0x00);      // Dummy byte
    sendCommand(startPage); // Start page address
    sendCommand(interval);  // Time interval
    sendCommand(endPage);   // End page address
    sendCommand(0x00);      // Dummy bytes
    sendCommand(0xFF);
    sendCommand(0x2F); // Activate scroll
    delay(10);
}

// Function to start horizontal scroll right with customizable speed
void startScrollRightImplementation(uint8_t startPage, uint8_t endPage, uint8_t speed)
{
    uint8_t interval;
    switch (speed)
    {
    case 5:
        interval = 0b000;
        break;
    case 64:
        interval = 0b001;
        break;
    case 128:
        interval = 0b010;
        break;
    case 256:
        interval = 0b011;
        break;
    case 3:
        interval = 0b100;
        break;
    case 4:
        interval = 0b101;
        break;
    case 25:
        interval = 0b110;
        break;
    case 2:
        interval = 0b111;
        break;
    default:
        interval = 0b000;
        break;
    }
    delay(10);

    // Command for horizontal scroll right
    sendCommand(0x26);      // Command for right horizontal scroll
    sendCommand(0x00);      // Dummy byte
    sendCommand(startPage); // Start page address
    sendCommand(interval);  // Time interval
    sendCommand(endPage);   // End page address
    sendCommand(0x00);      // Dummy bytes
    sendCommand(0xFF);
    sendCommand(0x2F); // Activate scroll
    delay(10);
}

void stopScrollImplementation()
{
    delay(1);
    display.stopscroll();
    delay(1);
}

void customCommandImplementation(uint8_t command)
{
    delay(100);
    display.ssd1306_command(command);
    delay(100);
}

#define DEBOUNCE_TIME_MS 1

unsigned long lastActionTimestamp = 0;
Action lastAction = OLED_NO_ACTION;

void OLED_MANAGERTask(void *pvParameters)
{
    ActionData actionData;
    while (true)
    {
        if (xQueueReceive(actionQueue, &actionData, portMAX_DELAY))
        {
            unsigned long currentMillis = millis();

            if (currentMillis - lastActionTimestamp >= DEBOUNCE_TIME_MS || actionData.action != lastAction)
            {
                xSemaphoreTake(actionMutex, portMAX_DELAY);

                switch (actionData.action)
                {
                case OLED_FADE_IN:
                    if (!fading && !displaying)
                    {
                        Serial.println("Processing OLED Fade In");
                        oledFadeInImplementation();
                    }
                    break;
                case OLED_FADE_OUT:
                    if (!fading && !displaying)
                    {
                        Serial.println("Processing OLED Fade Out");
                        oledFadeOutImplementation();
                    }
                    break;
                case OLED_DISPLAY:
                    if (!fading && !displaying)
                    {
                        oledDisplayImplementation();
                    }
                    break;
                case OLED_ENABLE:
                    if (!fading && !displaying)
                    {
                        Serial.println("Processing OLED Enable");
                        oledEnableImplementation();
                    }
                    break;
                case OLED_DISABLE:
                    if (!fading && !displaying)
                    {
                        Serial.println("Processing OLED Disable");
                        disableImplementation();
                    }
                    break;
                case OLED_SCROLL_LEFT:
                    Serial.print("Scrolling left with parameters: ");
                    Serial.print(actionData.param1, HEX);
                    Serial.print(", ");
                    Serial.print(actionData.param2, HEX);
                    Serial.print(", ");
                    Serial.println(actionData.param3, HEX);
                    startScrollLeftImplementation(actionData.param1, actionData.param2, actionData.param3);
                    break;
                case OLED_SCROLL_RIGHT:
                    Serial.print("Scrolling right with parameters: ");
                    Serial.print(actionData.param1, HEX);
                    Serial.print(", ");
                    Serial.print(actionData.param2, HEX);
                    Serial.print(", ");
                    Serial.println(actionData.param3, HEX);
                    startScrollRightImplementation(actionData.param1, actionData.param2, actionData.param3);
                    break;
                case OLED_STOP_SCROLL:
                    stopScrollImplementation();
                    break;
                case OLED_CUSTOM_COMMAND:
                    // Handle custom command if needed
                    break;
                default:
                    Serial.println("Unknown action");
                    break;
                }

                xSemaphoreGive(actionMutex);
                lastAction = actionData.action;
                lastActionTimestamp = currentMillis;
            }
            else
            {
                Serial.println("Action debounced");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Adjust delay as needed
    }
}

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1, 400000); // Initialize display object here

void initOLED_MANAGER()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    display.clearDisplay();

    actionQueue = xQueueCreate(1, sizeof(ActionData));
    actionMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(
        OLED_MANAGERTask, /* Task function. */
        "OLED_MANAGER",   /* String with name of task. */
        10000,            /* Stack size in words. */
        NULL,             /* Parameter passed as input of the task */
        5,                /* Priority of the task. */
        NULL,             /* Task handle. */
        1                 /* Core where the task should run. */
    );
}

void OLED_MANAGER::createTask()
{
    initOLED_MANAGER();
}
