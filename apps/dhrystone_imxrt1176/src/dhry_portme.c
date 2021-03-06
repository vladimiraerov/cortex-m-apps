/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "dhry_portme.h"
#include "clock_config.h"
#include "fsl_gpt.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/

double secs;

/*******************************************************************************
 * Code
 ******************************************************************************/

volatile uint32_t s_timerHighCounter = 0;

#if __CORTEX_M == 7
#define CLOCKS_PER_SEC (100000)

void SysTick_Handler(void)
{
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
    {
        SysTick->VAL = 0;
    }
    s_timerHighCounter++;
}

void timer_systick_init(void)
{
    SysTick->LOAD      = (uint32_t)(0xFFFFFF);                       /* set reload register */
    NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
    SysTick->VAL  = 0UL;                                             /* Load the SysTick Counter Value */
    SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void start_time(void)
{
    timer_systick_init();
    s_timerHighCounter = 0;
}

void end_time(void)
{
    uint64_t retVal = 0;
    uint32_t high;
    uint32_t low;
    do
    {
        high = s_timerHighCounter;
        low = (~(SysTick->VAL)) & 0xFFFFFF;
    } while (high != s_timerHighCounter);
    retVal = ((uint64_t)high << 24U) + low;

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    secs = retVal / (CLOCKS_PER_SEC * 1.0);
}
#elif __CORTEX_M == 4
#define CLOCKS_PER_SEC (32000)

void timer_gpt_init(void)
{
    gpt_config_t gpt;
    GPT_GetDefaultConfig(&gpt);
    GPT_Init(GPT2, &gpt);
    /* Using 32K osc as timer clock source */
    GPT_SetClockSource(GPT2, kGPT_ClockSource_LowFreq);
    GPT_StartTimer(GPT2);
}

void start_time(void)
{
    timer_gpt_init();
    s_timerHighCounter = 0;
}

void end_time(void)
{
    uint64_t retVal = GPT_GetCurrentTimerCount(GPT2);
    GPT_StopTimer(GPT2);

    secs = retVal / (CLOCKS_PER_SEC * 1.0);
}
#endif


