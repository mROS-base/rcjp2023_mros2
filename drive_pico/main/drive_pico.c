#include <stdio.h>
#include "pico.h"

void app_main(void)
{
  pico_init();
  while (1)
  {
    while (read_switch(SW_C))
    {
      delay_ms(10);
      continue;
    }
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
