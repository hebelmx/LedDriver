#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
/*
#include "BlynkState.h"
#include "ConfigMode.h"
#include "ConfigStore.h"
#include "Console.h"
#include "Indicator.h"
#include "OTA.h"
#include "ResetButton.h"
#include "Settings.h"
*/

#define GPIO_BLINK_PIN 13  // GPIO pin for the blinking LED
#define GPIO_PWM_PIN 14    // GPIO pin for the PWM-controlled LED

void app_main() {
    // Configure blinking LED
    gpio_reset_pin(GPIO_BLINK_PIN);
    gpio_set_direction(GPIO_BLINK_PIN, GPIO_MODE_OUTPUT);
    int level = 0;
    gpio_set_level(GPIO_BLINK_PIN, level);  // Initialize the pin to be LOW initially


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
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = GPIO_PWM_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0
    };
    ledc_channel_config(&ledc_channel);

    // Initialize fade functionality (if necessary, depending on usage)
    ledc_fade_func_install(0);

    while (1) {
        // Toggle blinking LED
        level = 1 - level;
        gpio_set_level(GPIO_BLINK_PIN, level);
        ESP_LOGI("LED_CTRL", "Blinking LED %d GPIO %d", level, GPIO_BLINK_PIN);

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

    }
}
