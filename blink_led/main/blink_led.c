#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led.h"

void app_main(void)
{
  led_init();

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
