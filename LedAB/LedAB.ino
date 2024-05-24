#include <WiFi.h>
#include "time.h"
#include <Arduino.h>

#define GPIO_BLINK_PIN_1 (gpio_num_t)13  // GPIO pin for the blinking LED
#define GPIO_BLINK_PIN_2 (gpio_num_t)6  // GPIO pin for the blinking LED
#define GPIO_PWM_PIN 14    // GPIO pin for the PWM-controlled LED

// WiFi credentials and NTP servers
const char* ssid = "FBI";
const char* password = "1492745399";
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

#define FREQUENCY 5000
#define RESOLUTION 10
#define CHANNEL 0
#define FADE_TIME 1000
#define HIGH_DUTY_CYCLE 500
#define LOW_DUTY_CYCLE 25

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
    pinMode(GPIO_BLINK_PIN_1, OUTPUT);
    pinMode(GPIO_BLINK_PIN_2, OUTPUT);


    ledcAttachChannel(GPIO_PWM_PIN,FREQUENCY,RESOLUTION,CHANNEL);

    // Connect to WiFi
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");

    // Initialize SNTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

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
}

void loop() {
    static bool level = false;

    digitalWrite(GPIO_BLINK_PIN_1, level);
    digitalWrite(GPIO_BLINK_PIN_2, !level);

    fadeLED(GPIO_PWM_PIN, level);

    Serial.printf("Blinking LED %s\n", level ? "ON" : "OFF");
    Serial.printf("PWM Duty Cycle is now %d%%\n", level ? HIGH_DUTY_CYCLE : LOW_DUTY_CYCLE);

    level = !level;
    delay(1000);
    printLocalTime(); // Print local time every second
}
