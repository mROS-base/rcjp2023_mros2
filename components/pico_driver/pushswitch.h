#ifndef __PUSHSWITCH_H_
#define __PUSHSWITCH_H_

#include "driver/gpio.h"

#define SW_L GPIO_NUM_10
#define SW_C GPIO_NUM_11
#define SW_R GPIO_NUM_12

void pushswitch_init(void);
int read_switch(int switch_no);
#endif // __PUSHSWITCH_H_
