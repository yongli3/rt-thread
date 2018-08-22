/*
 * File      : main.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-07-29     Arda.Fu      first implementation
 */
#include <rtdevice.h>
#include "board.h"
#include <rtthread.h>
#include "app_uart.h"

//#include <drivers/pin.h>

#define UART4_TX   21 // PB10
#define KEY0   22 // PB11

#define LED_PIN 2 //PC13

// for 9600 bps
#define DELAY_US (104)
// 8N1 115200 7us=1 pluse
// 8N1 9600 bps = 1200 bytes = 12000 bits/second; 83us = one pluse
// TICK_PER_SECOND = 100000 1 tick = 10us
static int uart4_tx(int argc, char** argv)
{
    int i = 0;
    unsigned char ch = 0xaa;

    rt_kprintf("TX %x\n", ch);

    // connected to PA3 for uart2 to receive

    // start to send one 8-bit
    rt_pin_write(UART4_TX, 0);
    rt_hw_us_delay(DELAY_US);

    for (i = 0; i < 8; i++)
    {
        if (ch & 0x1)
            rt_pin_write(UART4_TX, 1);
        else
            rt_pin_write(UART4_TX, 0);

        rt_hw_us_delay(DELAY_US);
        ch >>= 1;
    }

    rt_pin_write(UART4_TX, 1);
    rt_hw_us_delay(DELAY_US);
    return 0;
}
MSH_CMD_EXPORT(uart4_tx, uart4 TX);

static void led_thread_entry(void* parameter)
{
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    while (1)
    {
        /* 点亮 led */
        //rt_kprintf("led on, count : %d\r\n", count);
        rt_pin_write(LED_PIN, 0);
        rt_thread_delay(RT_TICK_PER_SECOND / 2); 

        /* 关灭 led */
        //rt_kprintf("led off\r\n");
        rt_pin_write(LED_PIN, 1);
        rt_thread_delay(RT_TICK_PER_SECOND / 2);
    }
}

// PA2=usart_TX PA3=usart2_RX
static void uart2_thread_entry(void* parameter)
{    
    rt_uint8_t uart_rx_data;

    rt_kprintf("+%s\n", __func__);

    /* 打开串口 */
    if (uart_open("uart2") != RT_EOK)
    {
        rt_kprintf("uart open error.\n");
         while (1)
         {
            rt_thread_delay(10);
         }
    }

    /* 单个字符写 */
    uart_putchar('2');
    uart_putchar('0');

    uart_putchar('1');
    uart_putchar('8');
    uart_putchar('\n');
    /* 写字符串 */
    uart_putstring("Hello RT-Thread!\r\n");

    while (1)
    {   
        /* read from uart2 */
        uart_rx_data = uart_getchar();
        // output to uart1
        rt_kprintf("%c", uart_rx_data);
        /* 错位 */
        //uart_rx_data = uart_rx_data + 1;
        /* 输出 */
        //uart_putchar(uart_rx_data);

    }            
}

void hdr_callback(void *args)
{
    char *a = args;
    //rt_kprintf("key0 IRQ %x\n", rt_pin_read(KEY0));
    return;
}

static void extirq_thread_entry(void* parameter)
{
    rt_kprintf("+%s\n", __func__);

    rt_pin_mode(KEY0, PIN_MODE_INPUT_PULLDOWN);
                              
    rt_pin_attach_irq(KEY0, PIN_IRQ_MODE_RISING, hdr_callback, (void*)"callback args");
    rt_pin_irq_enable(KEY0, PIN_IRQ_ENABLE);

    return;
}


// https://www.rt-thread.org/document/site/rtthread-application-note/driver/gpio/an0002-rtthread-driver-gpio/
int main(void)
{
    int i = 0;

    rt_kprintf("Built on ...%s %s %u\n", __DATE__, __TIME__, RT_TICK_PER_SECOND);

 
    rt_pin_mode(UART4_TX, PIN_MODE_OUTPUT);
    rt_pin_write(UART4_TX, 1);
#if 0
// gpio pin IRQ test
    rt_pin_mode(PINRX, PIN_IRQ_MODE_RISING_FALLING);
    //rt_pin_attach_irq();
    rt_pin_irq_enable(PINRX, RT_TRUE);


    //rt_pin_mode(PINTX, GPIO_MODE_OUTPUT_PP);
        rt_pin_write(PINRX,1);
        //rt_pin_write(PINTX,0);
        while(1)
        {
            rt_thread_delay(RT_TICK_PER_SECOND);
            rt_pin_write(PINRX,rt_pin_read(PINRX)?0:1);
            //rt_pin_write(PINTX,rt_pin_read(PINTX)?0:1);
        }
#endif


     rt_thread_t tid_irq;
    tid_irq = rt_thread_create("extirq",
                    extirq_thread_entry,
                    RT_NULL,
                    1024, 
                    4, 
                    10);
    if (tid_irq != RT_NULL)
        rt_thread_startup(tid_irq);
    else
        rt_kprintf("thread_create error!\n");


#if 0
    // create thread
     rt_thread_t tid;
    tid = rt_thread_create("uart2",
                    uart2_thread_entry,
                    RT_NULL,
                    1024, 
                    2, 
                    10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    else
        rt_kprintf("thread_create error!\n");
#endif

    // create thread
     rt_thread_t tid_led;
    tid_led = rt_thread_create("led",
                    led_thread_entry,
                    RT_NULL,
                    1024, 
                    2, 
                    10);
    if (tid_led != RT_NULL)
        rt_thread_startup(tid_led);
    else
        rt_kprintf("thread_create error!\n");



    /* user app entry */
    while (1) {

    //rt_kprintf("[RTC Test]RTC Test Start...%d\n", i++);
    rt_thread_delay(RT_TICK_PER_SECOND);
    
    }
    return 0;
}


int hello_func(int argc, char** argv)
{
    rt_kprintf("Hello RT-Thread!\n");
    return 0;
}
MSH_CMD_EXPORT(hello_func, say hello);

int gpio(int argc, char** argv)
{

    rt_kprintf("Hello RT-Thread! %d\n", argc);
    rt_pin_mode(KEY0, PIN_MODE_INPUT_PULLUP);
    
    rt_kprintf("%d %d\n", KEY0, rt_pin_read(KEY0));    
    return 0;
}
MSH_CMD_EXPORT(gpio, gpio tool);




