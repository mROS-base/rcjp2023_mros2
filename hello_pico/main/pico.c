#include "pico.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void pico_init() {
    led_init();
    pushswitch_init();
    buzzer_init();
    analog_sensor_init();
}

void delay_ms(int ms)
{
  vTaskDelay(ms / portTICK_PERIOD_MS);
}