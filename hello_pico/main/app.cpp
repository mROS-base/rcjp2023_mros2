#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led.h"
#include "pushswitch.h"

void delay_ms(int ms)
{
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

extern "C" void app_main(void)
{
  led_init();
  pushswitch_init();

  int pushswitch_r = 0;
  int pushswitch_c = 0;
  int pushswitch_l = 0;

  while (1)
  {
    while (gpio_get_level(SW_L) && gpio_get_level(SW_C) && gpio_get_level(SW_R))
    {
      continue;
    }
    if (gpio_get_level(SW_R) == 0)
    {
      pushswitch_r = !pushswitch_r;
      gpio_set_level(LED3, (pushswitch_r & 0x01));
    }
    if (gpio_get_level(SW_C) == 0)
    {
      pushswitch_c = !pushswitch_c;
      gpio_set_level(LED2, (pushswitch_c & 0x01));
      gpio_set_level(LED1, (pushswitch_c & 0x01));
    }
    if (gpio_get_level(SW_L) == 0)
    {
      pushswitch_l = !pushswitch_l;      
      gpio_set_level(LED0, (pushswitch_l & 0x01));
    }

    delay_ms(30);
    while (!(gpio_get_level(SW_L) && gpio_get_level(SW_C) && gpio_get_level(SW_R)))
    {
      continue;
    }
    delay_ms(30);
  }
}
