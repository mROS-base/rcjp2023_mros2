#include "pushswitch.h"
#include "driver/gpio.h"

void pushswitch_init()
{
  gpio_config_t config;
  config.pin_bit_mask = (1ULL << SW_L) | (1ULL << SW_C) | (1ULL << SW_R);
  config.mode = GPIO_MODE_INPUT;
  config.intr_type = GPIO_INTR_DISABLE;
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  config.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&config);
}
