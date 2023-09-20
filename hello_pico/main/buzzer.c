#include "buzzer.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

void buzzer_init(void){
  gpio_config_t config = {};
  config.pin_bit_mask = (1ULL << BUZZER);
  config.mode = GPIO_MODE_OUTPUT;
  config.intr_type = GPIO_INTR_DISABLE;
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  config.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&config);

  ledc_timer_config_t timer_conf = {};
  timer_conf.duty_resolution = LEDC_TIMER_10_BIT;
  timer_conf.freq_hz = LEDC_FREQ;
  timer_conf.speed_mode = LEDC_LOW_SPEED_MODE ;
  timer_conf.timer_num = LEDC_TIMER_0;
  ledc_timer_config(&timer_conf);

  ledc_channel_config_t ledc_conf = {};
  ledc_conf.channel = LEDC_CHANNEL_0;
  ledc_conf.duty = 2;//HARF_DUTY;
  ledc_conf.gpio_num = BUZZER;
  ledc_conf.intr_type = LEDC_INTR_DISABLE;
  ledc_conf.speed_mode = LEDC_LOW_SPEED_MODE ;
  ledc_conf.timer_sel = LEDC_TIMER_0;
  ledc_channel_config(&ledc_conf);
}

void buzzer_on(int freq){
  ledc_set_freq(LEDC_LOW_SPEED_MODE , LEDC_TIMER_0, freq);
  ledc_set_duty(LEDC_LOW_SPEED_MODE , LEDC_CHANNEL_0, HARF_DUTY);
  ledc_update_duty(LEDC_LOW_SPEED_MODE , LEDC_CHANNEL_0);
}

void buzzer_mute(){
  ledc_set_duty(LEDC_LOW_SPEED_MODE , LEDC_CHANNEL_0, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE , LEDC_CHANNEL_0);
}
