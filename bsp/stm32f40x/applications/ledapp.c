#include <stdio.h>
#include <rthw.h>
#include <rtdevice.h>
#include "board.h"
#include <rtthread.h>
#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */
/* led thread entry */
#define PINRX 44  // PC14-PC15 in gpio.c
#define PINTX 45
void led_thread_entry(void* parameter)
{
    rt_pin_mode(PINRX, PIN_MODE_OUTPUT);
    rt_pin_mode(PINTX, PIN_MODE_OUTPUT);
    rt_pin_write(PINRX,1);
    rt_pin_write(PINTX,0);
	while(1)
	{
        rt_thread_delay(RT_TICK_PER_SECOND/RT_TICK_PER_SECOND);
        rt_pin_write(PINRX,rt_pin_read(PINRX)?0:1);
        rt_pin_write(PINTX,rt_pin_read(PINTX)?0:1);
	}
}
