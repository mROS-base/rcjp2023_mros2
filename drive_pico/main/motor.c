#include <stdio.h>
#include "motor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/timer.h"
#include "soc/ledc_reg.h"
#include "common.h"

#include "esp_log.h"

portMUX_TYPE timer_mux = portMUX_INITIALIZER_UNLOCKED;

const unsigned short R_STEP_HZ = MIN_HZ;
const unsigned short L_STEP_HZ = MIN_HZ;

unsigned short step_hz_r = MIN_HZ;
unsigned short step_hz_l = MIN_HZ;

volatile unsigned int step_r;
volatile unsigned int step_l;
double max_speed = MIN_SPEED;
double min_speed = MIN_SPEED;
double accel = 0.0;
double speed = 0.0;

static bool isr_speed_adjust(void *param);
static void isr_pwm_count_r(void *);
static void isr_pwm_count_l(void *);
static void create_timer(timer_group_t timer_group, timer_idx_t timer_no, int divider, bool (*callback_func)(void *));

void motor_init()
{
  gpio_config_t config = {};
  config.pin_bit_mask = (1ULL << MOTOR_EN) | (1ULL << CW_R) | (1ULL << CW_R) | (1ULL << PWM_R) | (1ULL << PWM_L);
  config.mode = GPIO_MODE_OUTPUT;
  config.intr_type = GPIO_INTR_DISABLE;
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  config.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&config);
  gpio_set_level(MOTOR_EN, LOW);
  gpio_set_level(CW_R, LOW);
  gpio_set_level(CW_L, LOW);
  gpio_set_level(PWM_R, LOW);
  gpio_set_level(PWM_L, LOW);

  // right motor pwm conf
  ledc_timer_config_t ledc_timer_conf_r = {};
  ledc_timer_conf_r.duty_resolution = LEDC_TIMER_8_BIT;
  ledc_timer_conf_r.freq_hz = 100;
  ledc_timer_conf_r.speed_mode = LEDC_LOW_SPEED_MODE;
  ledc_timer_conf_r.timer_num = LEDC_TIMER_1;
  ledc_timer_config(&ledc_timer_conf_r);

  ledc_channel_config_t ledc_channel_conf_r = {};
  ledc_channel_conf_r.channel = LEDC_CHANNEL_1;
  ledc_channel_conf_r.duty = 127;
  ledc_channel_conf_r.gpio_num = PWM_R;
  ledc_channel_conf_r.intr_type = LEDC_INTR_FADE_END;
  ledc_channel_conf_r.speed_mode = LEDC_LOW_SPEED_MODE;
  ledc_channel_conf_r.timer_sel = LEDC_TIMER_1;
  ledc_channel_config(&ledc_channel_conf_r);

  // left motor pwm conf
  ledc_timer_config_t ledc_timer_conf_l = {};
  ledc_timer_conf_l.duty_resolution = LEDC_TIMER_8_BIT;
  ledc_timer_conf_l.freq_hz = 100;
  ledc_timer_conf_l.speed_mode = LEDC_LOW_SPEED_MODE;
  ledc_timer_conf_l.timer_num = LEDC_TIMER_2;
  ledc_timer_config(&ledc_timer_conf_l);

  ledc_channel_config_t ledc_channel_conf_l = {};
  ledc_channel_conf_l.channel = LEDC_CHANNEL_2;
  ledc_channel_conf_l.duty = 127;
  ledc_channel_conf_l.gpio_num = PWM_L;
  ledc_channel_conf_l.intr_type = LEDC_INTR_FADE_END;
  ledc_channel_conf_l.speed_mode = LEDC_LOW_SPEED_MODE;
  ledc_channel_conf_l.timer_sel = LEDC_TIMER_2;
  ledc_channel_config(&ledc_channel_conf_l);

  ledc_fade_func_install(ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED);
  ledc_isr_register(isr_pwm_count_r, NULL, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED, NULL);
  ledc_isr_register(isr_pwm_count_l, NULL, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED, NULL);
  WRITE_PERI_REG(LEDC_INT_ENA_REG, 0x06); /* LEDCのタイマーオバーフロー割り込み許可 ≒ パルス毎割り込み．0x06はTimer1とTimer2のビットを両方立てたもの */
}

void constant_speed(int len, int tar_speed)
{
  int obj_step;
  max_speed = tar_speed;
  accel = 0.0;
  step_r = step_l = 0;
  speed = min_speed = tar_speed;
  step_hz_r = step_hz_l = (unsigned short)(speed / PULSE);
  ESP_LOGI("motor.c", "step_hz: %d", (step_hz_r));

  obj_step = (int)((float)len * 2.0 / PULSE);
  gpio_set_level(CW_R, LOW);
  gpio_set_level(CW_L, LOW);

  while ((step_r + step_l) < obj_step)
  {
    delay_ms(1);
    continue;
  }
  gpio_set_level(MOTOR_EN, LOW);
}

void accelerate(int len, int tar_speed)
{
  int obj_step;
  max_speed = tar_speed;
  accel = 1.5;
  step_r = step_l = 0;
  speed = min_speed = MIN_SPEED;
  step_hz_r = step_hz_l = (unsigned short)(speed / PULSE);

  obj_step = (int)((float)len * 2.0 / PULSE);
  gpio_set_level(CW_R, LOW);
  gpio_set_level(CW_L, LOW);

  gpio_set_level(MOTOR_EN, HIGH);
  while ((step_r + step_l) < obj_step)
  {
    delay_ms(1);
    continue;
  }
  gpio_set_level(MOTOR_EN, HIGH);
}

void decelerate(int len, int tar_speed)
{
  int obj_step;
  max_speed = tar_speed;
  accel = 0.0;
  step_r = step_l = 0;
  speed = min_speed = tar_speed;
  step_hz_r = step_hz_l = (unsigned short)(speed / PULSE);
  obj_step = (int)((float)len * 2.0 / PULSE);
  gpio_set_level(CW_R, LOW);
  gpio_set_level(CW_L, LOW);

  gpio_set_level(MOTOR_EN, HIGH);
  while ((len - (step_r + step_l) / 2.0 * PULSE) >
         (((speed * speed) - (MIN_SPEED * MIN_SPEED)) / (2.0 * 1000.0 * 1.5)))
  {
    delay_ms(1);
    continue;
  }
  accel = -1.5;
  min_speed = MIN_SPEED;

  while ((step_r + step_l) < obj_step)
  {
    delay_ms(1);
    continue;
  }
  gpio_set_level(MOTOR_EN, HIGH);
}

// 速度更新用タイマー割り込み
static bool IRAM_ATTR isr_speed_adjust(void *param)
{
  // 速度更新はモーター回転時のみ
  if (gpio_get_level(MOTOR_EN) != HIGH)
  {
    return 0;
  }

  portENTER_CRITICAL_ISR(&timer_mux);
  speed += accel;
  if (speed > max_speed)
  {
    speed = max_speed;
  }
  if (speed < min_speed)
  {
    speed = min_speed;
  }
  step_hz_l = step_hz_r = (unsigned short)(speed / PULSE);
  ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_1, step_hz_r);
  ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_2, step_hz_l);
  portEXIT_CRITICAL_ISR(&timer_mux);
  return 0;
}

static IRAM_ATTR void isr_pwm_count_r(void *user_arg)
{
  portENTER_CRITICAL_ISR(&timer_mux);
  WRITE_PERI_REG(LEDC_INT_CLR_REG, (1U << 1));
  step_r++;
  portEXIT_CRITICAL_ISR(&timer_mux);
}

static IRAM_ATTR void isr_pwm_count_l(void *user_arg)
{
  portENTER_CRITICAL_ISR(&timer_mux);
  WRITE_PERI_REG(LEDC_INT_CLR_REG, (1U << 2));
  step_l++;
  portEXIT_CRITICAL_ISR(&timer_mux);
}

static void create_timer(timer_group_t timer_group, timer_idx_t timer_no, int divider, bool (*callback_func)(void *))
{
  timer_config_t timer_conf = {
      .alarm_en = TIMER_ALARM_DIS,
      .counter_en = TIMER_PAUSE,
      .intr_type = TIMER_INTR_LEVEL,
      .counter_dir = TIMER_COUNT_UP,
      .auto_reload = TIMER_AUTORELOAD_EN,
      .divider = divider};
  timer_init(timer_group, timer_no, &timer_conf);
  timer_set_counter_value(timer_group, timer_no, 0);
  timer_set_alarm_value(timer_group, timer_no, 1000);
  timer_enable_intr(timer_group, timer_no);
  timer_isr_callback_add(timer_group, timer_no, callback_func, NULL, 0);
}