#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"





void TIM2_Init(u16 arr,u16 psc);
void TIM3_Init(u16 arr,u16 psc);
void TIM4_Init(u16 arr,u16 psc);
void IWDG_Init(uint16_t prer,uint16_t rlr);
void IWDG_Feed(void);
#endif
