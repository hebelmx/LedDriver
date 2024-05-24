#include <WiFi.h>
#include <time.h>
#include <SPI.h>
#include "LCD_Driver.h"
#include "GUI_Paint.h"
#include "image.h"
#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL2vDz5-cY5"
#define BLYNK_TEMPLATE_NAME "LED"
#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_DEBUG
#define APP_DEBUG
#define USE_ESP32S3_DEV_MODULE

#include "BlynkEdgent.h"

#define GPIO_BLINK_PWM_1 6  // GPIO pin for the blinking LED

#define FREQUENCY 5000
#define RESOLUTION 10
#define CHANNEL 0
#define FADE_TIME 1000
#define HIGH_DUTY_CYCLE 500
#define LOW_DUTY_CYCLE 25
#define MAX_FADE_TIME_MS 1200

const char* ntpServer = "time.google.com";
const long utcOffsetInSeconds = -21600; // México: UTC -6 - Set the Eastern -6 Zone time (by default, the prime meridian of the Greenwich Observatory is the base line)
// 21600 = -6 * 60 * 60

bool isLCDInitialized = false;
bool isTimeInitialized = false;
unsigned long lastTimeCheck = 0;
const unsigned long timeCheckInterval = 60000; // Check time every 60 seconds

void initializeLCD() {
    Config_Init();
    LCD_Init();
    LCD_SetBacklight(100);
    Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 90, WHITE);
    Paint_SetRotate(90);
    LCD_Clear(BLACK);

    isLCDInitialized = true;
}

void initializeTime() {
    configTime(utcOffsetInSeconds, 0, ntpServer);
    isTimeInitialized = false;
}

void checkTimeSync() {
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);

    // If the time is not synced or year is less than 2022, try to sync again
    if (!isTimeInitialized || (timeInfo->tm_year + 1900) < 2022) {
        Serial.println("Time not synchronized or year < 2022, attempting to sync...");
        initializeTime();
    }

    // Check if time is now synchronized
    if (timeInfo->tm_year + 1900 >= 2022) {
        isTimeInitialized = true;
        Serial.println("Time synchronized.");
    }
}

void setup() {
    Serial.begin(115200);
    delay(100);

    // Initialize PWM channel
    ledcAttachChannel(GPIO_BLINK_PWM_1, FREQUENCY, RESOLUTION, CHANNEL);   
    ledcWrite(GPIO_BLINK_PWM_1, 0);  

    // Initialize Blynk
    BlynkEdgent.begin();
    initializeTime();  // Initialize time at startup
}

void removeNewlineCharacters(char* str) {
    size_t len = strlen(str);

    // Search forward from the end of the string, dropping the ending '\r' and '\n'
    for (int i = len - 1; i >= 0; --i) {
        if (str[i] == '\r' || str[i] == '\n') {
            str[i] = '\0';  // Replace '\r' or '\n' with '\0'
        } else {
            break;          // Stop after finding the first character that is not '\r' and '\n'
        }
    }
}

void extractDateAndTime(const char* timeString, char* dateTimeStr, char* timeStr) {
    // Use the sscanf function to extract the week, month, date, and year from the string
    sscanf(timeString, "%s %s %s %s %s", dateTimeStr + 14, dateTimeStr + 6, dateTimeStr + 10, timeStr, dateTimeStr);
}

void printTimeAndDate() {
    // Check the year and reinitialize time if needed
    checkTimeSync();

    time_t now = time(nullptr);
    char* timeString = ctime(&now);
    removeNewlineCharacters(timeString);

    char date[20];  // Save a buffer for the date, such as "2023 Jan 01 Tue"
    char time[9];   // Save time buffer, such as "12:34:56"

    // Extract date and time
    extractDateAndTime(timeString, date, time);

    // Display the time and date
    Paint_DrawString_EN(55, 50, time, &Font24, BLACK, GREEN);  // Reduced font size
    Paint_DrawString_EN(15, 90, date, &Font24, BLACK, GREEN);  // Reduced font size
    Paint_DrawString_EN(90, 90, (date + 6), &Font24, BLACK, GREEN);  // Display the day of the week
    Paint_DrawString_EN(150, 90, (date + 10), &Font24, BLACK, GREEN);  // Display the month
    Paint_DrawString_EN(185, 90, (date + 14), &Font24, BLACK, GREEN);  // Display the day of the month
}

void printLevel(int input) {
    // Display the level
    Paint_DrawString_EN(20, 10, "Level:", &Font24, BLACK, BLUE);
    Paint_DrawString_EN(120, 10, "     ", &Font24, BLACK, BLACK);  // Adjusted to be on the same line as "Level:"
    Paint_DrawString_EN(120, 10, String(input).c_str(), &Font24, RED, WHITE);  // Adjusted to be on the same line as "Level:"
}

BLYNK_WRITE(V0) { 
    int input = param.asInt();  // Input from 0 to 1000
    Serial.print("Level... ");
    Serial.println(input);

    double gamma = 2.2;  // Common gamma value for perceptual linearity
    double normalizedInput = input / 1000.0;  // Normalize input to range [0, 1]
    double correctedOutput = pow(normalizedInput, gamma);  // Apply gamma correction
    int outputIntensity = static_cast<int>(correctedOutput * 700);  // Scale back to [0, 1000]
    int start_duty = ledcRead(GPIO_BLINK_PWM_1);

    ledcFade(GPIO_BLINK_PWM_1, start_duty, outputIntensity, MAX_FADE_TIME_MS);

    if (isLCDInitialized) {    
        printLevel(input);
    }  
}

void loop() {
    BlynkEdgent.run();
    Blynk.syncVirtual(V0);

    // Initialize the LCD only after Blynk is connected
    if (Blynk.connected() && !isLCDInitialized) {
        initializeLCD();
    }

    // Periodically check time synchronization
    if (millis() - lastTimeCheck >= timeCheckInterval) {
        lastTimeCheck = millis();
        checkTimeSync();
    }

    // Print time and date on the LCD
    if (Blynk.connected() && isLCDInitialized && isTimeInitialized) {
        printTimeAndDate();
    }

    delay(1000);    
}
