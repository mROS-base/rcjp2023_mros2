#ifndef _PROXIMITY_SENSOR_H
#define _PROXIMITY_SENSOR_H

#define SLED_FR GPIO_NUM_16
#define SLED_FL GPIO_NUM_15
#define SLED_R GPIO_NUM_18
#define SLED_L GPIO_NUM_17

#define AD1 GPIO_NUM_4
#define AD2 GPIO_NUM_5
#define AD3 GPIO_NUM_6
#define AD4 GPIO_NUM_7

#define ADC_UNIT ADC_UNIT_1
#define AD1_CHANNEL ADC1_CHANNEL_3
#define AD2_CHANNEL ADC1_CHANNEL_4
#define AD3_CHANNEL ADC1_CHANNEL_5
#define AD4_CHANNEL ADC1_CHANNEL_6

typedef enum {
  L, FL, FR, R
} SENSOR_POS;

void proximity_sensor_init();
int read_sensor(SENSOR_POS);

#endif // _PROXIMITY_SENSOR_H