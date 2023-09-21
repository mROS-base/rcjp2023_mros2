#ifndef __LED_H_
#define __LED_H_
#define LED0 GPIO_NUM_1
#define LED1 GPIO_NUM_2
#define LED2 GPIO_NUM_42
#define LED3 GPIO_NUM_41

void led_init();
void set_led(char);

#endif // __LED_H_
