#include <WiFi.h>
#include <WiFiClient.h>
#include "time.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <SPI.h>
#include "LCD_Driver.h"
#include "GUI_Paint.h"
#include "image.h"



#define BLYNK_TEMPLATE_ID "TMPL2vDz5-cY5"
#define BLYNK_TEMPLATE_NAME "LED"
//#define BLYNK_AUTH_TOKEN "lgUbO-zyXGk16ohpnYSobx1zLQa3WcTq"

#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial
#define BLYNK_DEBUG
#define APP_DEBUG

#define USE_ESP32S3_DEV_MODULE

#include "BlynkEdgent.h"

#define GPIO_LED_1 (gpio_num_t)13  // GPIO pin for the blinking LED
#define GPIO_PWM 6  // GPIO pin for the PWM-controlled LED
#define GPIO_PIN_14 14  // GPIO NOT USED


// WiFi credentials and NTP servers
const char* ssid = "FBI";
const char* password = "1492745399";
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const int daylightOffset_sec = 0;
const long utcOffsetInSeconds = -21600;  // México: UTC -6

#define FREQUENCY 5000
#define RESOLUTION 10
#define CHANNEL 0
#define FADE_TIME 1000
#define HIGH_DUTY_CYCLE 500
#define LOW_DUTY_CYCLE 25

int outputIntensity = 0;

void printLocalTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void fadeLED(int pin, bool level) {
    uint32_t startDuty = level ? LOW_DUTY_CYCLE : HIGH_DUTY_CYCLE;
    uint32_t targetDuty = level ? HIGH_DUTY_CYCLE : LOW_DUTY_CYCLE;    
    ledcWrite(pin, startDuty);
    ledcFade(pin, startDuty, targetDuty, FADE_TIME);
}

void setup() {
    Serial.begin(115200);
    delay(100);

    pinMode(GPIO_LED_1, OUTPUT);

    ledcAttachChannel(GPIO_PWM, FREQUENCY, RESOLUTION, CHANNEL);

    //Connect to WiFi
    Serial.printf("Connecting to %s ", ssid, password);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");

    BlynkEdgent.begin();

    // Initialize SNTP
    configTime(utcOffsetInSeconds, daylightOffset_sec, ntpServer1, ntpServer2);

    // Wait for time to be set
    long startTime = millis();
    while (millis() - startTime < 10000) {
        time_t now;
        if (time(&now)) {
            printLocalTime();
            break;
        }
        delay(500);
        Serial.print(".");
    }

    // LCD initialization
    Config_Init();
    LCD_Init();
    LCD_SetBacklight(100);
    Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 90, WHITE);
    Paint_SetRotate(90);
    LCD_Clear(BLACK);
    delay(1000);

    Paint_DrawString_EN(20, 50, "Wifi Connected", &Font20, BLACK, GREEN);
    Serial.println("Connected to WiFi");
}

BLYNK_WRITE(V0) {
    int input = param.asInt();  // Input from 0 to 1000
    double gamma = 2.2;  // Common gamma value for perceptual linearity
    double normalizedInput = input / 1000.0;  // Normalize input to range [0, 1]
    double correctedOutput = pow(normalizedInput, gamma);  // Apply gamma correction
    outputIntensity = static_cast<int>(correctedOutput * 800);  // Scale back to [0, 1000]

    ledcWrite(GPIO_PWM, outputIntensity);
    
}

void removeNewlineCharacters(char* str) {
    size_t len = strlen(str);
  // Search forward from the end of the string, dropping the ending '\r' and '\n'
    for (int i = len - 1; i >= 0; --i) {
        if (str[i] == '\r' || str[i] == '\n') {
            str[i] = '\0';  // Replace '\r' or '\n' with '\0'
        } else {
            break;  // Stop after finding the first character that is not '\r' or '\n'
        }
    }
}

void extractDateAndTime(const char* timeString, char* dateTimeStr, char* timeStr) {
  // Use the sscanf function to extract the week, month, date, and year from the string
    sscanf(timeString, "%s %s %s %s %s", dateTimeStr + 14, dateTimeStr + 6, dateTimeStr + 10, timeStr, dateTimeStr);
}

void displayTimeAndPWM() {
    time_t now = time(nullptr);
    char* timeString = ctime(&now);
    removeNewlineCharacters(timeString);

    char date[20];
    char time[9];

    extractDateAndTime(timeString, date, time);

    LCD_Clear(BLACK);

    Paint_DrawString_EN(20, 32, time, &Font24, BLACK, GREEN);
    Paint_DrawString_EN(20, 82, date, &Font20, BLACK, GREEN);

    char pwmString[30];
    sprintf(pwmString, "PWM: %d", outputIntensity);
    Paint_DrawString_EN(20, 132, pwmString, &Font20, BLACK, GREEN);

    Serial.print("Current time is: ");
    Serial.println(timeString);  // Print time
    Serial.print("PWM Output is: ");
    Serial.println(outputIntensity);
}

void loop() {
    BlynkEdgent.run();  
    static bool level = false;

    digitalWrite(GPIO_LED_1, level);
    level = !level;

    Blynk.syncVirtual(V0);
    delay(1000);

    displayTimeAndPWM();
}
