#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED0 GPIO_NUM_1
#define LED1 GPIO_NUM_2
#define LED2 GPIO_NUM_42
#define LED3 GPIO_NUM_41

extern "C" void app_main(void)
{
  gpio_config_t config;
  config.pin_bit_mask = (1ULL << LED0) | (1ULL << LED1) | (1ULL << LED2) | (1ULL << LED3);
  config.mode = GPIO_MODE_OUTPUT;
  config.intr_type = GPIO_INTR_DISABLE;
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  config.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&config);

  while (1)
  {
    gpio_set_level(LED0, 1);
    gpio_set_level(LED1, 1);
    gpio_set_level(LED2, 1);
    gpio_set_level(LED3, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(LED0, 0);
    gpio_set_level(LED1, 0);
    gpio_set_level(LED2, 0);
    gpio_set_level(LED3, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
