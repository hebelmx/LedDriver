
#include <WiFi.h>
#include "time.h"
#include <Arduino.h>


#define GPIO_BLINK_PIN_1 (gpio_num_t)13  // GPIO pin for the blinking LED
#define GPIO_BLINK_PIN_2 (gpio_num_t)6  // GPIO pin for the blinking LED
#define GPIO_PWM_PIN (uint8_t )14    // GPIO pin for the PWM-controlled LED


// WiFi credentials and NTP servers
const char* ssid = "FBI";
const char* password = "1492745399";
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;


const  uint32_t frequency= 5000;
const uint8_t  resolution = 10;
const  int8_t channel = 0;

void printLocalTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}



void setup() {
    Serial.begin(115200);
    pinMode(GPIO_BLINK_PIN_1, OUTPUT);
    pinMode(GPIO_BLINK_PIN_2, OUTPUT);
  
    ledcAttachChannel(GPIO_PWM_PIN, frequency, resolution,channel);

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
    static int level = 0;
    static uint32_t start_duty =0; 
    static int target_duty =0; 
    static int max_fade_time_ms = 1000 ;

    digitalWrite(GPIO_BLINK_PIN_1, level);
    digitalWrite(GPIO_BLINK_PIN_2, !level);
    level = !level;

    start_duty = level ? 500 : 25 ;
    target_duty = level ? 25 : 500 ;

    ledcFade(GPIO_PWM_PIN, start_duty,target_duty,max_fade_time_ms);

    Serial.printf("Blinking LED %d GPIO %d\n", target_duty, GPIO_PWM_PIN);
    Serial.printf("PWM Duty Cycle is now %d%%\n", level ? 500 : 25);

    delay(1000);
    printLocalTime(); // Print local time every second
}
