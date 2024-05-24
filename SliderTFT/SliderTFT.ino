#include <WiFi.h>
#include <time.h>
#include <SPI.h>
#include "LCD_Driver.h"
#include "GUI_Paint.h"
#include "image.h"
#include <Arduino.h>


#define BLYNK_TEMPLATE_ID "TMPL2vDz5-cY5"
#define BLYNK_TEMPLATE_NAME "LED"

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

bool isLCDInitialized = false;
void initializeLCD() {
    Config_Init();
    LCD_Init();
    LCD_SetBacklight(100);
    Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 90, WHITE);
    Paint_SetRotate(90);
    LCD_Clear(BLACK);

    Paint_DrawString_EN(20, 50, "Level...", &Font20, BLACK, GREEN);
    Paint_DrawString_EN(80, 82, "0", &Font20, BLACK, GREEN);
    
    isLCDInitialized = true;
}

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
  Serial.print("Level... ");
  Serial.println(input);
  double gamma = 2.2;  // Common gamma value for perceptual linearity
  double normalizedInput = input / 1000.0;  // Normalize input to range [0, 1]
  double correctedOutput = pow(normalizedInput, gamma);  // Apply gamma correction
  int outputIntensity = static_cast<int>(correctedOutput * 700);  // Scale back to [0, 1000]
  int start_duty =ledcRead(GPIO_BLINK_PWM_1);

  ledcFade(GPIO_BLINK_PWM_1 , start_duty,  outputIntensity,  MAX_FADE_TIME_MS);

  if (isLCDInitialized) {
        Paint_DrawString_EN(20, 50, "Level...", &Font20, BLACK, GREEN);
        Paint_DrawString_EN(140, 50, String(input).c_str(), &Font20, BLACK, GREEN);  // Update display with string conversion

    }
  
  
}


void loop() {
    BlynkEdgent.run();
    Blynk.syncVirtual(V0);

     // Initialize the LCD only after Blynk is connected
    if (Blynk.connected() && !isLCDInitialized) {
        initializeLCD();
    }

    delay(200);    
}
