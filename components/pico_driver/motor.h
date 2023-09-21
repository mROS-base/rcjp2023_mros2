
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
#define TREAD_WIDTH (65.00)
#define PULSE (TIRE_DIAMETER * PI / 400.0)
#define MIN_SPEED (MIN_HZ * PULSE)
#define DUTY 511 // 10bit / 2 - 1

#define HIGH 1
#define LOW 0

void motor_init();
void drive_by_cmd_vel(double linear_x, double angular_z);
void constant_speed(int len, int tar_speed);
void accelerate(int len, int tar_speed);
void decelerate(int len, int tar_speed);
void motor_start();
void motor_stop();

#endif // __MOTOR_H_
