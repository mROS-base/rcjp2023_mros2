#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "pico.h"

void app_main(void)
{
  pico_init();
  while (1)
  {
    while (gpio_get_level(SW_L) & gpio_get_level(SW_C) & gpio_get_level(SW_R))
    {
      delay_ms(1);
      continue;
    }
    delay_ms(1000);

    motor_start();
    gpio_set_level(LED0, HIGH);
    accelerate(100, 350);
    gpio_set_level(LED1, HIGH);
    gpio_set_level(LED2, HIGH);
    constant_speed(100, 350);
    gpio_set_level(LED3, HIGH);
    decelerate(100, 350);
    gpio_set_level(LED0, LOW);
    gpio_set_level(LED1, LOW);
    gpio_set_level(LED2, LOW);
    gpio_set_level(LED3, LOW);
    delay_ms(1000);
    motor_stop();
  }
}
