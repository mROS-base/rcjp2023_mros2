#include "led.h"
#include "driver/gpio.h"

void led_init()
{
  gpio_config_t config = {};
  config.pin_bit_mask = (1ULL << LED0) | (1ULL << LED1) | (1ULL << LED2) | (1ULL << LED3);
  config.mode = GPIO_MODE_OUTPUT;
  config.intr_type = GPIO_INTR_DISABLE;
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  config.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&config);
}

void set_led(char data)
{
  if (data & 0x01)
  {
    gpio_set_level(LED0, 1);
  }
  else
  {
    gpio_set_level(LED0, 0);
  }

  if (data & 0x02)
  {
    gpio_set_level(LED1, 1);
  }
  else
  {
    gpio_set_level(LED1, 0);
  }

  if (data & 0x04)
  {
    gpio_set_level(LED2, 1);
  }
  else
  {
    gpio_set_level(LED2, 0);
  }

  if (data & 0x08)
  {
    gpio_set_level(LED3, 1);
  }
  else
  {
    gpio_set_level(LED3, 0);
  }
}
