#include "analog_sensor.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

static int _read_sensor(adc_channel_t sensor, gpio_num_t ked);
static adc_oneshot_unit_handle_t adc_handle;

void analog_sensor_init()
{
  gpio_config_t config = {};
  config.pin_bit_mask = (1ULL << SLED_FR) | (1ULL << SLED_R) | (1ULL << SLED_L) | (1ULL << SLED_FR);
  config.mode = GPIO_MODE_OUTPUT;
  config.intr_type = GPIO_INTR_DISABLE;
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  config.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&config);

  config.pin_bit_mask = (1ULL << AD1) | (1ULL << AD2) | (1ULL << AD3) | (1ULL << AD4) | (1ULL << AD0);
  config.mode = GPIO_MODE_INPUT;
  gpio_config(&config);

  adc_oneshot_unit_init_cfg_t adc_config = {};
  adc_config.unit_id = ADC_UNIT_1;
  adc_config.ulp_mode = ADC_ULP_MODE_DISABLE;
  adc_oneshot_new_unit(&adc_config, &adc_handle);

  adc_oneshot_chan_cfg_t adc_channel_config = {};
  adc_channel_config.bitwidth = ADC_BITWIDTH_12;
  adc_channel_config.atten = ADC_ATTEN_DB_11;
  adc_oneshot_config_channel(adc_handle, AD1_CHANNEL, &adc_channel_config);
}

int read_sensor(SENSOR_POS pos)
{
  switch (pos)
  {
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

int read_battery_level()
{
  int value;
  adc_oneshot_read(adc_handle, AD0_CHANNEL, &value);
  return value;
}

static int _read_sensor(adc_channel_t sensor_channel, gpio_num_t sled)
{
  gpio_set_level(sled, 1);
  for (int i = 0; i < 300; i++)
  {
    asm("nop \n");
  }

  int value;
  adc_oneshot_read(adc_handle, sensor_channel, &value);
  gpio_set_level(sled, 0);
  return value;
}
