#include "delay_ms.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void delay_ms(int ms)
{
  vTaskDelay(ms / portTICK_PERIOD_MS);
}
