#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "motor.h"
#include "common.h"

void app_main(void)
{
  motor_init();
  delay_ms(300);
  constant_speed(100, 50);
}
