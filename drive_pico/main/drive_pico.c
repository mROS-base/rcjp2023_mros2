#include <stdio.h>
#include "pico.h"
#include "esp_log.h"

void app_main(void)
{
  pico_init();
  while (1)
  {
    while (read_switch(SW_L) & read_switch(SW_C) & read_switch(SW_R))
    {
      delay_ms(10);
      continue;
    }
    if (read_switch(SW_R) == 0)
    {
      motor_start();
    }
    else if (read_switch(SW_C) == 0)
    {
      motor_start();
      drive_by_cmd_vel(0.2, 0.0);
      delay_ms(3000);
      drive_by_cmd_vel(0.0, 0.0);
      delay_ms(500);
      drive_by_cmd_vel(-0.2, 0.0);
      delay_ms(3000);
      drive_by_cmd_vel(0.1, 2.0);
      delay_ms(3000);
      drive_by_cmd_vel(0.1, -2.0);
      delay_ms(3000);
      motor_stop();
    }
    else if (read_switch(SW_L) == 0)
    {
      motor_stop();
    }
  }
}
