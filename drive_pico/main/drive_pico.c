#include <stdio.h>
#include "pico.h"

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
    delay_ms(1000);

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
}
