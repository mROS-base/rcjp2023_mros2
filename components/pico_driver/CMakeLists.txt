set(SRCS "analog_sensor.c"
         "buzzer.c"
         "delay_ms.c"
         "led.c"
         "motor.c"
         "pico.c"
         "pushswitch.c"
)

idf_component_register(SRCS ${SRCS}
                    REQUIRES driver esp_adc
                    INCLUDE_DIRS ".")
