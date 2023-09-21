#include "pico.h"

void pico_init()
{
  led_init();
  pushswitch_init();
  buzzer_init();
  analog_sensor_init();
  motor_init();
}
