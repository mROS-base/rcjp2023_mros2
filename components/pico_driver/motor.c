#include <stdio.h>
#include "motor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/gptimer.h"
#include "soc/ledc_reg.h"
#include "delay_ms.h"
#include "esp_log.h"

portMUX_TYPE timer_mux = portMUX_INITIALIZER_UNLOCKED;

const unsigned int R_STEP_HZ = MIN_HZ;
const unsigned int L_STEP_HZ = MIN_HZ;

unsigned int step_hz_r = MIN_HZ;
unsigned int step_hz_l = MIN_HZ;

volatile unsigned int step_r;
volatile unsigned int step_l;
double max_speed = MIN_SPEED;
double min_speed = MIN_SPEED;
double accel = 0.0;
double speed = 0.0;
bool speed_adjusting = false;

static double limit_speed(double speed);
static bool isr_speed_adjust(gptimer_handle_t timer, const gptimer_alarm_event_data_t *alarm_data, void *param);
static void isr_pwm_count_r(void *);
static void isr_pwm_count_l(void *);

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

  // speed ajuster isr conf
  gptimer_handle_t gptimer = NULL;
  gptimer_config_t timer_config = {};
  timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
  timer_config.direction = GPTIMER_COUNT_UP;
  timer_config.resolution_hz = 1000000; // 1MHz
  gptimer_new_timer(&timer_config, &gptimer);
  gptimer_event_callbacks_t timer_callback = {
      .on_alarm = isr_speed_adjust,
  };
  gptimer_register_event_callbacks(gptimer, &timer_callback, NULL);
  gptimer_alarm_config_t alarm_config = {
      .reload_count = 0,
      .alarm_count = 1000, // 1kHz
      .flags.auto_reload_on_alarm = 1,
  };
  gptimer_set_alarm_action(gptimer, &alarm_config);
  gptimer_enable(gptimer);
  gptimer_start(gptimer);

  // right motor pwm conf
  ledc_timer_config_t ledc_timer_conf_r = {};
  ledc_timer_conf_r.duty_resolution = LEDC_TIMER_10_BIT;
  ledc_timer_conf_r.freq_hz = 1000;
  ledc_timer_conf_r.speed_mode = LEDC_LOW_SPEED_MODE;
  ledc_timer_conf_r.timer_num = LEDC_TIMER_1;
  ledc_timer_conf_r.clk_cfg = LEDC_SLOW_CLK_RC_FAST,
  ledc_timer_config(&ledc_timer_conf_r);

  ledc_channel_config_t ledc_channel_conf_r = {};
  ledc_channel_conf_r.channel = LEDC_CHANNEL_1;
  ledc_channel_conf_r.duty = DUTY;
  ledc_channel_conf_r.gpio_num = PWM_R;
  ledc_channel_conf_r.intr_type = LEDC_INTR_FADE_END;
  ledc_channel_conf_r.speed_mode = LEDC_LOW_SPEED_MODE;
  ledc_channel_conf_r.timer_sel = LEDC_TIMER_1;
  ledc_channel_config(&ledc_channel_conf_r);

  // left motor pwm conf
  ledc_timer_config_t ledc_timer_conf_l = {};
  ledc_timer_conf_l.duty_resolution = LEDC_TIMER_10_BIT;
  ledc_timer_conf_l.freq_hz = 1000;
  ledc_timer_conf_l.speed_mode = LEDC_LOW_SPEED_MODE;
  ledc_timer_conf_l.timer_num = LEDC_TIMER_2;
  ledc_timer_conf_l.clk_cfg = LEDC_SLOW_CLK_RC_FAST,
  ledc_timer_config(&ledc_timer_conf_l);

  ledc_channel_config_t ledc_channel_conf_l = {};
  ledc_channel_conf_l.channel = LEDC_CHANNEL_2;
  ledc_channel_conf_l.duty = DUTY;
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

void drive_by_cmd_vel(double linear_x, double angular_z)
{
  speed = linear_x * 1000;
  double speed_r = speed + angular_z * TREAD_WIDTH / 2.0;
  double speed_l = speed - angular_z * TREAD_WIDTH / 2.0;

  if (speed_r > 0)
  {
    gpio_set_level(CW_R, LOW);
  }
  else
  {
    gpio_set_level(CW_R, HIGH);
    speed_r *= -1;
  }
  speed_r = limit_speed(speed_r);

  if (speed_l > 0)
  {
    gpio_set_level(CW_L, LOW);
  }
  else
  {
    gpio_set_level(CW_L, HIGH);
    speed_l *= -1;
  }
  speed_l = limit_speed(speed_l);

  if (speed_r == 0.0)
  {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
  }
  else
  {
    step_hz_r = (unsigned short)(speed_r / PULSE);
    ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_1, step_hz_r);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, DUTY);
  }

  if (speed_l == 0.0)
  {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 0);
  }
  else
  {
    step_hz_l = (unsigned short)(speed_l / PULSE);
    ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_2, step_hz_l);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, DUTY);
  }

  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
}

void constant_speed(int len, int tar_speed)
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

  speed_adjusting = true;
  while ((step_r + step_l) < obj_step)
  {
    delay_ms(1); // ビジーループはFreeRTOSのWDTに引っかかるのでDelayを入れている．以下同様
    continue;
  }
  speed_adjusting = false;
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

  speed_adjusting = true;
  while ((step_r + step_l) < obj_step)
  {
    delay_ms(1);
    continue;
  }
  speed_adjusting = false;
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

  speed_adjusting = true;
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
  speed_adjusting = false;
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 0);

  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
}

void motor_start()
{
  gpio_set_level(MOTOR_EN, HIGH);
}

void motor_stop()
{
  gpio_set_level(MOTOR_EN, LOW);
  speed_adjusting = false;
}

static double limit_speed(double speed)
{
  double limited_speed = speed;
  if (speed > 500)
  {
    limited_speed = 500;
  }
  else if (speed < 0.001)
  {
    limited_speed = 0.0;
  }
  else if (speed < MIN_SPEED)
  {
    limited_speed = MIN_SPEED;
  }
  return limited_speed;
}

// 速度更新用タイマー割り込み(1kHz)
static bool IRAM_ATTR isr_speed_adjust(gptimer_handle_t timer, const gptimer_alarm_event_data_t *alarm_data, void *param)
{
  if (!speed_adjusting)
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

  step_hz_l = step_hz_r = (unsigned int)(speed / PULSE);
  ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_1, step_hz_r);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, DUTY);

  ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_2, step_hz_l);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, DUTY);

  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
  portEXIT_CRITICAL_ISR(&timer_mux);
  return 0;
}

// 右モーターのパルスカウント
static IRAM_ATTR void isr_pwm_count_r(void *user_arg)
{
  if (!speed_adjusting)
  {
    WRITE_PERI_REG(LEDC_INT_CLR_REG, (1U << 1));
    return;
  }
  portENTER_CRITICAL_ISR(&timer_mux);
  WRITE_PERI_REG(LEDC_INT_CLR_REG, (1U << 1));
  step_r++;
  portEXIT_CRITICAL_ISR(&timer_mux);
}

// 右モーターのパルスカウント
static IRAM_ATTR void isr_pwm_count_l(void *user_arg)
{
  if (!speed_adjusting)
  {
    WRITE_PERI_REG(LEDC_INT_CLR_REG, (1U << 2));
    return;
  }
  portENTER_CRITICAL_ISR(&timer_mux);
  WRITE_PERI_REG(LEDC_INT_CLR_REG, (1U << 2));
  step_l++;
  portEXIT_CRITICAL_ISR(&timer_mux);
}
