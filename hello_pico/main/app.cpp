#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led.h"
#include "pushswitch.h"
#include "buzzer.h"

void delay_ms(int ms)
{
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

void execByMode(char mode)
{
  switch (mode)
  {
  case 1:
    buzzer_on(FREQ_C);
    break;
  case 2:
    buzzer_on(FREQ_D);
    break;
  case 3:
    buzzer_on(FREQ_E);
    break;
  }
  delay_ms(1000);
  buzzer_mute();
}

extern "C" void app_main(void)
{
  led_init();
  pushswitch_init();
  buzzer_init();

  char mode = 1;
  set_led(mode);

  while (1)
  {
    while (gpio_get_level(SW_L) && gpio_get_level(SW_C) && gpio_get_level(SW_R))
    {
      delay_ms(10);
      continue;
    }

    if (gpio_get_level(SW_R) == 0)
    {
      mode++;
      if(mode > 15)
      {
        mode = 15;
      }
      else
      {
        buzzer_on(INC_FREQ);
        delay_ms(30);
        buzzer_mute();
      }
      set_led(mode);
    }

    if (gpio_get_level(SW_L) == 0)
    {
      mode--;
      if(mode < 1)
      {
        mode = 1;
      }
      else
      {
        buzzer_on(DEC_FREQ);
        delay_ms(30);
        buzzer_mute();
      }
      set_led(mode);
    }

    if (gpio_get_level(SW_C) == 0)
    {
        buzzer_on(INC_FREQ);
        delay_ms(30);
        buzzer_mute();
        buzzer_on(DEC_FREQ);
        delay_ms(30);
        buzzer_mute();
        execByMode(mode);

    }
    delay_ms(30);
    while (!(gpio_get_level(SW_L) && gpio_get_level(SW_C) && gpio_get_level(SW_R)))
    {
      delay_ms(10);
      continue;
    }
    delay_ms(30);
  }
}
