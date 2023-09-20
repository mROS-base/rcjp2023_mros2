
#ifndef __MOTOR_H_
#define __MOTOR_H_

#define MOTOR_EN GPIO_NUM_9
#define CW_R GPIO_NUM_14
#define CW_L GPIO_NUM_21
#define PWM_R GPIO_NUM_13
#define PWM_L GPIO_NUM_45

#define PI 3.1415926535897932384626433832795
#define MIN_HZ 80
#define TIRE_DIAMETER (48.00)
#define PULSE (TIRE_DIAMETER * PI / 400.0)
#define MIN_SPEED (MIN_HZ * PULSE)

#define HIGH 1
#define LOW 0

void motor_init();
void constant_speed(int len, int tar_speed);
void accelerate(int len, int tar_speed);
void decelerate(int len, int tar_speed);

#endif // __MOTOR_H_