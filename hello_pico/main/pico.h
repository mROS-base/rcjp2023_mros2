#ifndef __PICO_H_
#define __PICO_H_

#include "led.h"
#include "pushswitch.h"
#include "buzzer.h"
#include "analog_sensor.h"

void pico_init();
void delay_ms(int ms);

#endif // __PICO_H_
