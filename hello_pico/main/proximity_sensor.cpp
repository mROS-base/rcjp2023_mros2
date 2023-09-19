#include "proximity_sensor.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"


static int _read_sensor(adc1_channel_t sensor, gpio_num_t ked);

void sensor_init(){
  gpio_config_t config = {};
  config.pin_bit_mask = (1ULL << SLED_FR) | (1ULL << SLED_R) | (1ULL << SLED_L) | (1ULL << SLED_FR);
  config.mode = GPIO_MODE_OUTPUT;
  config.intr_type = GPIO_INTR_DISABLE;
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  config.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&config);

  config.pin_bit_mask = (1ULL << AD1) | (1ULL << AD2) | (1ULL << AD3) | (1ULL << AD4);
  config.mode = GPIO_MODE_INPUT;
  gpio_config(&config);

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(AD1_CHANNEL, ADC_ATTEN_DB_11);
  adc1_get_raw(AD1_CHANNEL);
}

int read_sensor(SENSOR_POS pos){
  switch(pos){
    case FR:
      return _read_sensor(AD1_CHANNEL, SLED_FR);
    case FL:
      return _read_sensor(AD2_CHANNEL, SLED_FL);
    case R:
      return _read_sensor(AD3_CHANNEL, SLED_R);
    case L:
      return _read_sensor(AD4_CHANNEL, SLED_L);
  }
  return 0;
}

//static int _read_sensor(adc_channel_t sensor_channel, gpio_num_t sled){
static int _read_sensor(adc1_channel_t sensor, gpio_num_t sled){
  gpio_set_level(sled, 1);
  for(int i = 0; i < 300; i++){
    asm("nop \n");
  }
  int value = adc1_get_raw(sensor);
  gpio_set_level(sled, 0);
  return value;
}