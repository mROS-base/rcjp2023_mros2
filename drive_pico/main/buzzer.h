#ifndef __BUZZER_H_
#define __BUZZER_H_

#define BUZZER GPIO_NUM_38

#define INC_FREQ 2000
#define DEC_FREQ 1000

#define FREQ_C 523 // ド
#define FREQ_D 587 // レ
#define FREQ_E 659 // ミ

#define LEDC_FREQ 5000
#define HARF_DUTY 511 // (((2 ** 10) -1) / 2

void buzzer_init(void);
void buzzer_on(int freq);
void buzzer_mute();

#endif // __BUZZER_H_