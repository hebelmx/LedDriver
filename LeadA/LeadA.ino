#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"



#define GPIO_BLINK_PIN_1 (gpio_num_t)13  // GPIO pin for the blinking LED
#define GPIO_BLINK_PIN_2 (gpio_num_t)6  // GPIO pin for the blinking LED
#define GPIO_PWM_PIN (gpio_num_t)14    // GPIO pin for the PWM-controlled LED


const char* ssid       = "FBI";
const char* password   = "1492745399";

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

void setup()
{
   // Configure blinking LED
  gpio_reset_pin(GPIO_BLINK_PIN_1);
  gpio_set_direction(GPIO_BLINK_PIN_1, GPIO_MODE_OUTPUT);

  gpio_reset_pin(GPIO_BLINK_PIN_2);
  gpio_set_direction(GPIO_BLINK_PIN_2, GPIO_MODE_OUTPUT);
    // Configure blinking LED

    int level = 0;
    gpio_set_level(GPIO_BLINK_PIN_1, level);  // Initialize the pin to be LOW initially
    gpio_set_level(GPIO_BLINK_PIN_2,1-level);  // Initialize the pin to be LOW initially


    // Set up and configure the PWM properties
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,  // Frequency in Hz
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);
ledc_channel_config_t ledc_channel = {
    .gpio_num = GPIO_PWM_PIN,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .intr_type = LEDC_INTR_FADE_END, // This might not be necessary depending on your version/configuration
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,
    .hpoint = 0
};
    ledc_channel_config(&ledc_channel);

    // Initialize fade functionality (if necessary, depending on usage)
    ledc_fade_func_install(0);

  Serial.begin(115200);

  // First step is to configure WiFi STA and connect in order to get the current time and date.
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");

  // set notification call-back function
  sntp_set_time_sync_notification_cb( timeavailable );

  /**
   * NTP server address could be aquired via DHCP,
   *
   * NOTE: This call should be made BEFORE esp32 aquires IP address via DHCP,
   * otherwise SNTP option 42 would be rejected by default.
   * NOTE: configTime() function call if made AFTER DHCP-client run
   * will OVERRIDE aquired NTP server address
   */ 
  esp_sntp_servermode_dhcp(1);// (optional)

  /**
   * This will set configured ntp servers and constant TimeZone/daylightOffset
   * should be OK if your time zone does not need to adjust daylightOffset twice a year,
   * in such a case time adjustment won't be handled automagicaly.
   */
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  /**
   * A more convenient approach to handle TimeZones with daylightOffset 
   * would be to specify a environmnet variable with TimeZone definition including daylight adjustmnet rules.
   * A list of rules for your zone could be obtained from https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
   */
  //configTzTime(time_zone, ntpServer1, ntpServer2);
}

int level = 0;
void loop()
{
       // Toggle blinking LED
        level = 1 - level;
        gpio_set_level(GPIO_BLINK_PIN_1, level);
        gpio_set_level(GPIO_BLINK_PIN_2, 1-level);
        Serial.printf("LED_CTRL", "Blinking LED %d GPIO %d", level, GPIO_BLINK_PIN_1);

        if (level == 0) {
            // Fade LED up
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4095, 1000);
            ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
            printf("PWM Duty Cycle is fading to 100%%\n");
        } else {
            // Fade LED down
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 1000);
            ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
            printf("PWM Duty Cycle is fading to 0%%\n");
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);

  printLocalTime();     // it will take some time to sync time :)
}
