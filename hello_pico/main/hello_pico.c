#include "esp_log.h"

#include "pico.h"

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

void app_main(void)
{
  pico_init();

  char mode = 1;
  set_led(mode);
  int light_sensor_value_r;
  int light_sensor_value_fr;
  int light_sensor_value_fl;
  int light_sensor_value_l;
  int battery_level;

  while (1)
  {
    while (read_switch(SW_L) && read_switch(SW_C) && read_switch(SW_R))
    {
      light_sensor_value_r = read_light_sensor(R);
      light_sensor_value_fr = read_light_sensor(FR);
      light_sensor_value_fl = read_light_sensor(FL);
      light_sensor_value_l = read_light_sensor(L);
      battery_level = read_battery_level();
      ESP_LOGI("app_main", "right sensor:       %d\n\r", light_sensor_value_r);
      ESP_LOGI("app_main", "front-right_sensor: %d\n\r", light_sensor_value_fr);
      ESP_LOGI("app_main", "front-left_sensor:  %d\n\r", light_sensor_value_fl);
      ESP_LOGI("app_main", "left_sensor:        %d\n\r", light_sensor_value_l);
      ESP_LOGI("app_main", "battery_level:      %d\n\r", battery_level);

      delay_ms(100);
      continue;
    }

    if (read_switch(SW_R) == 0)
    {
      mode++;
      if (mode > 15)
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

    if (read_switch(SW_L) == 0)
    {
      mode--;
      if (mode < 1)
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

    if (read_switch(SW_C) == 0)
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

    while (!(read_switch(SW_L) && read_switch(SW_C) && read_switch(SW_R)))
    {
      delay_ms(10);
      continue;
    }
    delay_ms(30);
  }
}
