#include "lpc407x_8x_177x_8x.h"
#include "drv_timer.h"
/*init the timer*/
void Timer_Init(void)
{
    /*使能timer0时钟*/
    LPC_SC->PCONP |= (0x01 << 1);

    LPC_TIM0->CTCR &= ~0x03;
    LPC_TIM0->CTCR |= 0x00;

    LPC_TIM0->TC = 0;
    LPC_TIM0->PC = 0;
    /*分频为1khz*/
    LPC_TIM0->PR = (PeripheralClock / 1000) - 1;
    LPC_TIM0->TCR |= (1 << 1); //Reset Counter
    LPC_TIM0->TCR &= ~(1 << 1); //release reset
    // no matches or interrupts
    LPC_TIM0->MCR = 0x0000;
    // enable
    LPC_TIM0->TCR = 0x01;
}

// returns timer 0 to same state as after reset
// needed so user applications can make assumptions about
// the state of the registers
void Timer_DeInit(void)
{
    // disable
    LPC_TIM0->TCR  = 0x00;
    LPC_TIM0->CTCR = 0x00;
    LPC_TIM0->PR   = 0x00;
    LPC_TIM0->MCR  = 0x00;
    LPC_TIM0->TC   = 0x00;
    LPC_TIM0->PC   = 0x00;
}

// get the current timer 0 count
uint32_t Timer_GetCurrentTick(void)
{
    return LPC_TIM0->TC;
}

// check to see if a timestamp is in the past
// returns 1 if in the past, 0 if not
int Timer_CheckTimeout(uint32_t timeout)
{
    uint32_t current_tick = LPC_TIM0->TC;

    if (current_tick > timeout)
    {
        if ((current_tick - timeout) < 0x80000000)
            return 1;
        else
            return 0;
    }
    else
    {
        if ((timeout - current_tick) >= 0x80000000)
            return 1;
        else
            return 0;
    }
}

// pause for a specific number of milliseconds
void delay_ms(uint32_t milliseconds)
{
    uint32_t timeout = Timer_GetCurrentTick() + milliseconds;
    while (!Timer_CheckTimeout(timeout));
}
