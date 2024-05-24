

#include <WiFi.h>
#include "time.h"
#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL2HrbKzQ-z"
#define BLYNK_TEMPLATE_NAME "LED"
//#define BLYNK_AUTH_TOKEN "FQ9U0zWMJPOJyzw82vjgFEvX3LR6kJbW"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_ESP32_DEV_MODULE

//#define USE_ESP32S2_DEV_KIT
//#define USE_WROVER_BOARD
//#define USE_TTGO_T7
//#define USE_TTGO_T_OI

#define USE_ESP32S3_DEV_MODULE

#include "BlynkEdgent.h"

#define GPIO_BLINK_PIN_1 (gpio_num_t)13  // GPIO pin for the blinking LED
#define GPIO_BLINK_PWM 6  // GPIO pin for the blinking LED
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
#define CHANNEL2 1
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
    delay(100);

    

    pinMode(GPIO_BLINK_PIN_1, OUTPUT);


    ledcAttachChannel(GPIO_PWM_PIN,FREQUENCY,RESOLUTION,CHANNEL);
    ledcAttachChannel(GPIO_BLINK_PWM,FREQUENCY,RESOLUTION,CHANNEL2);

    // Connect to WiFi
    Serial.printf("Connecting to %s ", ssid, password);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");
    
    //BlynkEdgent.begin(BLYNK_AUTH_TOKEN,ssid,password);
    BlynkEdgent.begin();

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

BLYNK_WRITE(V0)
{
  // any code you place here will execute when the virtual pin value changes
  Serial.print("Blynk.Cloud is writing something to V1");
  int value = param.asInt();
  ledcWrite(GPIO_BLINK_PWM, value);

}


BLYNK_WRITE_DEFAULT()
{
  int pin = request.pin;      // Which exactly pin is handled?
  int value = param.asInt();  // Use param as usual.  
  ledcWrite(GPIO_BLINK_PWM, value);
}
void loop() {

    BlynkEdgent.run();


    static bool level = false;

    digitalWrite(GPIO_BLINK_PIN_1, level);

    fadeLED(GPIO_PWM_PIN, level);

    Serial.printf("Blinking LED %s\n", level ? "ON" : "OFF");
    Serial.printf("PWM Duty Cycle is now %d%%\n", level ? HIGH_DUTY_CYCLE : LOW_DUTY_CYCLE);

    level = !level;

    Blynk.syncAll();
    Blynk.syncVirtual(V0);
    delay(1000);
    printLocalTime(); // Print local time every second
}
