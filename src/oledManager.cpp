#include "oledManager.h"

// Queue to hold actions
QueueHandle_t actionQueue;
// Mutex to ensure only one action runs at a time
SemaphoreHandle_t actionMutex;

bool displaying = false;
bool waitingToDisplay = false;
bool fading = false;
bool waitingToFadein = false;
bool waitingToFadeout = false;

void oledDisplay()
{
    Action action = OLED_DISPLAY;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void oledFadeout()
{
    Action action = OLED_FADE_OUT;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void oledFadein()
{
    Action action = OLED_FADE_IN;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void oledDisable()
{
    Action action = OLED_DISABLE;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void oledEnable()
{
    Action action = OLED_ENABLE;
    xQueueSend(actionQueue, &action, portMAX_DELAY);
}

void sendOledCustomCommand(uint8_t command)
{
    ActionData actionData;
    actionData.action = OLED_CUSTOM_COMMAND;
    actionData.param1 = command;
    xQueueSend(actionQueue, &actionData, portMAX_DELAY);
}

void stopScrolling()
{
    ActionData actionData;
    actionData.action = OLED_STOP_SCROLL;

    xQueueSend(actionQueue, &actionData, portMAX_DELAY);
}

void startScrollingLeft(uint8_t startPage, uint8_t endPage, uint8_t speed)
{
    ActionData actionData;
    actionData.action = OLED_SCROLL_LEFT;
    actionData.param1 = startPage;
    actionData.param2 = endPage;
    actionData.param3 = speed;

    xQueueSend(actionQueue, &actionData, portMAX_DELAY);
}

void startScrollingRight(uint8_t startPage, uint8_t endPage, uint8_t speed)
{
    ActionData actionData;
    actionData.action = OLED_SCROLL_RIGHT;
    actionData.param1 = startPage;
    actionData.param2 = endPage;
    actionData.param3 = speed;

    xQueueSend(actionQueue, &actionData, portMAX_DELAY);
}

// Communication to the display below
void oledDisableImplementation()
{
    delay(100);
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    delay(100);
}

void oledEnableImplementation()
{
    delay(100);
    display.ssd1306_command(SSD1306_DISPLAYON);
    delay(100);
}

void oledDisplayImplementation()
{
    vTaskDelay(pdMS_TO_TICKS(1));
    display.display();
    vTaskDelay(pdMS_TO_TICKS(1));
}

void oledFadeoutImplementation()
{
    if (fading || displaying)
        return; // Check if already fading or displaying

    fading = true; // Set fading flag to true

    delay(100);
    // Fade out
    for (int dim = 150; dim >= 0; dim -= 10)
    {
        display.ssd1306_command(0x81); // Contrast control command
        display.ssd1306_command(dim);  // Set contrast value
        delay(10);
    }

    delay(50);

    for (int dim2 = 34; dim2 >= 0; dim2 -= 17)
    {
        display.ssd1306_command(0xD9); // Pre-charge period command
        display.ssd1306_command(dim2); // Set pre-charge period
        delay(20);
    }
    delay(100);

    fading = false; // Reset fading flag
}
void oledFadeinImplementation()
{
    fading = true; // Set fading flag to true

    delay(100);
    // Fade out
    for (uint8_t dim = 0; dim <= 160; dim += 10)
    {
        display.ssd1306_command(0x81);
        display.ssd1306_command(dim);
        delay(10);
    }

    delay(50);

    for (uint8_t dim2 = 0; dim2 <= 34; dim2 += 17)
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
    // Map speed to C[2:0] values
    uint8_t interval;
    switch (speed)
    {
    case 5:
        interval = 0b000;
        break; // 5 frames
    case 64:
        interval = 0b001;
        break; // 64 frames
    case 128:
        interval = 0b010;
        break; // 128 frames
    case 256:
        interval = 0b011;
        break; // 256 frames
    case 3:
        interval = 0b100;
        break; // 3 frames
    case 4:
        interval = 0b101;
        break; // 4 frames
    case 25:
        interval = 0b110;
        break; // 25 frames
    case 2:
        interval = 0b111;
        break; // 2 frames
    default:
        interval = 0b000;
        break; // Default to 5 frames
    }
    delay(10);

    // Command for horizontal scroll left
    sendCommand(0x27); // Command for left horizontal scroll

    // Dummy byte
    sendCommand(0x00);

    // Set start page address (B[2:0])
    sendCommand(startPage);

    // Set time interval between each scroll step (C[2:0])
    sendCommand(interval);

    // Set end page address (D[2:0])
    sendCommand(endPage);

    // Dummy bytes
    sendCommand(0x00);
    sendCommand(0xFF);

    // Activate scroll
    sendCommand(0x2F);
    delay(10);
}

// Function to start horizontal scroll right with customizable speed
void startScrollRightImplementation(uint8_t startPage, uint8_t endPage, uint8_t speed)
{
    // Map speed to C[2:0] values
    uint8_t interval;
    switch (speed)
    {
    case 5:
        interval = 0b000;
        break; // 5 frames
    case 64:
        interval = 0b001;
        break; // 64 frames
    case 128:
        interval = 0b010;
        break; // 128 frames
    case 256:
        interval = 0b011;
        break; // 256 frames
    case 3:
        interval = 0b100;
        break; // 3 frames
    case 4:
        interval = 0b101;
        break; // 4 frames
    case 25:
        interval = 0b110;
        break; // 25 frames
    case 2:
        interval = 0b111;
        break; // 2 frames
    default:
        interval = 0b000;
        break; // Default to 5 frames
    }

    delay(10);

    // Command for horizontal scroll right
    sendCommand(0x26); // Command for right horizontal scroll

    // Dummy byte
    sendCommand(0x00);

    // Set start page address (B[2:0])
    sendCommand(startPage);

    // Set time interval between each scroll step (C[2:0])
    sendCommand(interval);

    // Set end page address (D[2:0])
    sendCommand(endPage);

    // Dummy bytes
    sendCommand(0x00);
    sendCommand(0xFF);

    // Activate scroll
    sendCommand(0x2F);
    delay(10);
}


// Function to start horizontal scroll right with customizable speed
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

#define DEBOUNCE_TIME_MS 5

unsigned long lastActionTimestamp = 0;

Action lastAction = OLED_NO_ACTION;

void oledManagerTask(void *pvParameters)
{
    ActionData actionData;
    while (true)
    {
        if (xQueueReceive(actionQueue, &actionData, portMAX_DELAY))
        {
            unsigned long currentMillis = millis();

            // Handle actions based on debounce time and state
            if (currentMillis - lastActionTimestamp >= DEBOUNCE_TIME_MS || actionData.action != lastAction)
            {
                xSemaphoreTake(actionMutex, portMAX_DELAY);

                switch (actionData.action)
                {
                case OLED_FADE_IN:
                    if (!fading && !displaying)
                    {
                        Serial.println("Processing OLED Fade In");
                        oledFadeinImplementation();
                    }
                    break;
                case OLED_FADE_OUT:
                    if (!fading && !displaying)
                    {
                        Serial.println("Processing OLED Fade Out");
                        oledFadeoutImplementation();
                    }
                    break;
                case OLED_DISPLAY:
                    if (!fading && !displaying)
                    {
                        Serial.println("Processing OLED Display");
                        vTaskDelay(pdMS_TO_TICKS(5));
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
                        oledDisableImplementation();
                    }
                    break;
                case OLED_SCROLL_LEFT:
                    Serial.print("Scrolling left with parameters: ");
                    Serial.print(actionData.param1, HEX);
                    Serial.print(", ");
                    Serial.println(actionData.param2, HEX);
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
                    Serial.print("Stopping scroll");
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
        vTaskDelay(pdMS_TO_TICKS(5)); // Adjust delay as needed
    }
}

void initOledManager()
{
    display.clearDisplay();

    actionQueue = xQueueCreate(1, sizeof(ActionData));
    actionMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(
        oledManagerTask, /* Task function. */
        "oledManager",   /* String with name of task. */
        10000,           /* Stack size in words. */
        NULL,            /* Parameter passed as input of the task */
        1,               /* Priority of the task. */
        NULL,            /* Task handle. */
        1                /* Core where the task should run. */
    );
}

void createOledManagerTask()
{
    initOledManager();
}
