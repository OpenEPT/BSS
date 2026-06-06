/*
 * profiler.c
 *
 *  Created on: Oct 6, 2025
 *      Author: elektronika
 */

#ifdef STM32_BUILD
#include "main.h"
#endif

void PROFILER_Reset()
{
#ifdef STM32_BUILD
    CoreDebug->DEMCR |= 0x01000000;
    DWT->CYCCNT = 0; // reset the counter
    DWT->CTRL = 0;
#endif
}

void PROFILER_Start()
{
#ifdef STM32_BUILD
    DWT->CTRL |= 0x00000001 ; // enable the counter
#endif
}

void PROFILER_Stop()
{
#ifdef STM32_BUILD
     DWT->CTRL &= 0xFFFFFFFE ; // disable the counter
#endif
}

unsigned int PROFILER_GetValue()
{
#ifdef STM32_BUILD
    return DWT->CYCCNT ;
#else
    return 0; // Dummy value for PC build
#endif
}
