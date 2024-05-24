#include <Arduino.h>
#include <WiFi.h>

#define BLYNK_TEMPLATE_ID "TMPL2vDz5-cY5"
#define BLYNK_TEMPLATE_NAME "LED"
//#define BLYNK_AUTH_TOKEN "FQ9U0zWMJPOJyzw82vjgFEvX3LR6kJbW"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
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


void setup() {
    Serial.begin(115200);
    delay(100);


    ledcAttachChannel(GPIO_BLINK_PWM_1,FREQUENCY,RESOLUTION,CHANNEL);   
    ledcWrite(GPIO_BLINK_PWM_1, 0);  

    BlynkEdgent.begin();

}

BLYNK_WRITE(V0)
{
  
  int input = param.asInt();  // Input from 0 to 1000
  Serial.print("level ");
  Serial.println(input);
  double gamma = 2.2;  // Common gamma value for perceptual linearity
  double normalizedInput = input / 1000.0;  // Normalize input to range [0, 1]
  double correctedOutput = pow(normalizedInput, gamma);  // Apply gamma correction
  int outputIntensity = static_cast<int>(correctedOutput * 700);  // Scale back to [0, 1000]
  int start_duty =ledcRead(GPIO_BLINK_PWM_1);


  ledcFade(GPIO_BLINK_PWM_1 , start_duty,  outputIntensity,  MAX_FADE_TIME_MS);
  
}

int i = 0;
void loop() {
    BlynkEdgent.run();
    Blynk.syncVirtual(V0);
    delay(1000);    
}
