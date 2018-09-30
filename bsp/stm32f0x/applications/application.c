/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-11-15     bright       add init thread and components initial
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <stdio.h>

#include <board.h>
#include <rtthread.h>
#include <finsh.h>

#include "led.h"

/* led thread entry */
static void led_thread_entry(void* parameter)
{
	while(1)
	{
        rt_hw_led_on();
        rt_thread_delay(RT_TICK_PER_SECOND);

        rt_hw_led_off();
        rt_thread_delay(RT_TICK_PER_SECOND);
	}
}

static void rt_init_thread_entry(void* parameter)
{
	rt_thread_t led_thread;

/* Initialization RT-Thread Components */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_init();
#endif

/* Set finsh device */
#ifdef  RT_USING_FINSH
    finsh_system_init();
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif  /* RT_USING_FINSH */

    /* Create led thread */
    led_thread = rt_thread_create("led",
    		led_thread_entry, RT_NULL,
    		256, 20, 20);
    if(led_thread != RT_NULL)
    	rt_thread_startup(led_thread);
}

int rt_application_init()
{
	rt_thread_t init_thread;

#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   512, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   512, 80, 20);
#endif
    if(init_thread != RT_NULL)
    	rt_thread_startup(init_thread);

    return 0;
}

static rt_device_t uart2_dev = RT_NULL;

static int uart2_test(int argc, char** argv)
{
    rt_err_t ret = RT_ERROR;
    
    char test[80] = "uart2>";
    rt_kprintf("Hello RT-Thread!\n");

    uart2_dev = rt_device_find("uart2");
    if (uart2_dev != RT_NULL)
    {
        ret = rt_device_open(uart2_dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
        if (ret != RT_EOK) {
            rt_kprintf("Open error!\n");
            return -1;
        }

        rt_device_write(uart2_dev, 0, test, sizeof(test));
        while(1){

        rt_size_t reclen = rt_device_read(uart2_dev, 0, test, 10);
        if(reclen > 0) 
            rt_device_write(uart2_dev, 0, test, reclen);
        rt_thread_delay(RT_TICK_PER_SECOND);
    }
}


    return 0;
}
MSH_CMD_EXPORT(uart2_test, uart2 test);

static int uart3_test()
{	
	return 0;
}
MSH_CMD_EXPORT(uart3_test, uart3 test);