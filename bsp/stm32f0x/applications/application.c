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
#include <shell.h>

#include "led.h"

// AP address
#define AP_ADDRX    0xff
#define AP_ADDRY    0xff

#define CMD_SETADDR 0
#define CMD_ACK     1
#define CMD_REPORT  2
#define CMD_BYPASS  3
#define CMD_REPORT_ACK     4


#define UART_PKT_HEADER 0xbeef

#define STATE_WAIT_SET_ADDR 1
#define STATE_WAIT_REPORT_ACK 2
#define STATE_SEND_ACK 3
#define STATE_SEND_REPORT 4
#define STATE_SET_NEXT_ADDR 5
#define STATE_WAIT_NEXT_ADDR_ACK 6
#define STATE_WAIT_PACKET 7


/*
AP:
    OUT SET_ADDR first addr 0,0
    IN wait for ACK
    IN WAIT for report data
    OUT send out ACK
    
MCU:
    IN wait for SET_ADDR from upstream_UART
    OUT send out ACK to upstream_UART

    OUT report DATA to AP to upstream_UART
    IN wait for ACK from upstream_UART

   OUT set addr for next MCU to downstream_UART
   IN wait for ACK from downstream_UART

   IN wait for by-pass packet from downstream_UART or upstream_UART
   OUT by-pass packet
*/

typedef __packed struct
{
    uint16_t header;
    uint8_t command;
    uint8_t srcx;
    uint8_t srcy;
    uint8_t dstx;
    uint8_t dsty;
    uint32_t data;
} UART_PKT;

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
static rt_device_t uart3_dev = RT_NULL;

static int uart2_test(int argc, char** argv)
{
    uint16_t *pbuf = NULL;
    int i = 0;
    rt_tick_t current_tick = 0;
    rt_size_t reclen = 0;
    rt_err_t ret = RT_ERROR;
    char rxbuf[sizeof(UART_PKT) * 2] = "";
    char txbuf[sizeof(UART_PKT) * 2] = "";
    UART_PKT rx_packet, tx_packet;
    UART_PKT *prx_pkt = NULL;
    uint8_t local_addrx = 0xff;
    uint8_t local_addry = 0xff;

    rt_kprintf("Hello RT-Thread! %d\n", sizeof(tx_packet));

    uart2_dev = rt_device_find("uart2");
    if (uart2_dev != RT_NULL)
    {
        ret = rt_device_open(uart2_dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
        if (ret != RT_EOK) {
            rt_kprintf("Open error!\n");
            return -1;
        }

        //rt_device_write(uart2_dev, 0, test, sizeof(test));
        // start uart TX/RX
        while(1){
            // wait for SET_ADDR command
            rt_kprintf("%u read.\n", rt_tick_get());
            memset(&rxbuf, 0, sizeof(rxbuf));
            reclen = rt_device_read(uart2_dev, 0, rxbuf, sizeof(rxbuf));
            rt_kprintf(">>read %d %x\n", reclen, rxbuf[0]);
            prx_pkt = NULL;
            if(reclen >= sizeof(UART_PKT)) {
                // search for header 0xbeef;
                pbuf = (uint16_t *)rxbuf;
                for (i = 0; i < reclen - 1; i++) {
                    pbuf = (uint16_t *)&rxbuf[i];
                    if (*pbuf == UART_PKT_HEADER) {
                        if (i + sizeof(UART_PKT) <= reclen) {
                            rt_kprintf("FIND@%d\n", i);
                            prx_pkt = (UART_PKT *)pbuf;
                            break;
                        }
                    }
                    rt_kprintf("%d=%x\n", i, rxbuf[i]);
                }
                //rt_kprintf("%u read %x-%x-%x [%s]\n", rt_tick_get(), rxbuf[0], rxbuf[1], rxbuf[2], rxbuf);
                //rt_kprintf("%u read [%s]\n", rt_tick_get(), rxbuf);
                //sprintf(txbuf, "%u", rt_tick_get());
                //memset(&tx_packet, 0, sizeof(tx_packet));
                //tx_packet.header = UART_PKT_HEADER;
                //tx_packet.command = CMD_ACK;
                //rt_device_write(uart2_dev, 0, &tx_packet, sizeof(tx_packet));
            }else {
                rt_kprintf("receive small packet!\n");
            }

            if (prx_pkt != NULL) {
                memset(&rx_packet, 0, sizeof(UART_PKT));
                memcpy(&rx_packet, prx_pkt, sizeof(UART_PKT));
                rt_kprintf("> %x cmd=%d src(%d %d) dst(%d %d) %d\n", 
                rx_packet.header, rx_packet.command, rx_packet.srcx, rx_packet.srcy,
                rx_packet.dstx, rx_packet.dsty, rx_packet.data);
                local_addrx = rx_packet.dstx;
                local_addry = rx_packet.dsty;
                switch (rx_packet.command) {
                    case CMD_SETADDR:
                        rt_kprintf("<<send out ACK\n");
                        memset(&tx_packet, 0, sizeof(UART_PKT));
                        tx_packet.header = UART_PKT_HEADER;
                        tx_packet.command = CMD_ACK;
                        tx_packet.srcx = AP_ADDRX;
                        tx_packet.srcy = AP_ADDRY;
                        tx_packet.dstx = rx_packet.srcx;
                        tx_packet.dsty = rx_packet.srcy;
                        tx_packet.data = rt_tick_get();
                        rt_device_write(uart2_dev, 0, &tx_packet, sizeof(tx_packet));

                        // send out REPORT
                        rt_kprintf("<<send out REPORT\n");
                        memset(&tx_packet, 0, sizeof(UART_PKT));
                        tx_packet.header = UART_PKT_HEADER;
                        tx_packet.command = CMD_REPORT;
                        tx_packet.srcx = local_addrx;
                        tx_packet.srcy = local_addry;
                        tx_packet.dstx = AP_ADDRX;
                        tx_packet.dsty = AP_ADDRY;
                        tx_packet.data = rt_tick_get();
                        rt_device_write(uart2_dev, 0, &tx_packet, sizeof(tx_packet));

                        // wait for ACK
                        
                        
                    
                        break;
                    default:
                        break;
                }
                // send out ACK
            }

            
            
           
           
            rt_thread_delay(RT_TICK_PER_SECOND);
        }
    }
    rt_device_close(uart2_dev);
    return 0;
}
MSH_CMD_EXPORT(uart2_test, uart2 test);

static int uart22_test(int argc, char** argv)
{
    uint16_t *pbuf = NULL;
    int i = 0;
    rt_tick_t current_tick = 0;
    rt_size_t reclen = 0;
    rt_err_t ret = RT_ERROR;
    char rxbuf[sizeof(UART_PKT) * 2] = "";
    char txbuf[sizeof(UART_PKT) * 2] = "";
    UART_PKT rx_packet, tx_packet;
    UART_PKT *prx_pkt = NULL;
    uint8_t local_addrx = 0xff;
    uint8_t local_addry = 0xff;
    uint8_t current_state = STATE_WAIT_SET_ADDR;
    rt_kprintf("Hello RT-Thread! %d\n", sizeof(tx_packet));

    uart2_dev = rt_device_find("uart2");
    if (uart2_dev != RT_NULL)
    {
        ret = rt_device_open(uart2_dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
        if (ret != RT_EOK) {
            rt_kprintf("Open uart2 error!\n");
            return -1;
        }
    } else {
        rt_kprintf("uart2 missing!\n");
        return -1;
    }

    uart3_dev = rt_device_find("uart3");
    if (uart3_dev != RT_NULL)
    {
        ret = rt_device_open(uart3_dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
        if (ret != RT_EOK) {
            rt_kprintf("Open uart3 error!\n");
            return -1;
        }
    } else {
        rt_kprintf("uart3 missing!\n");
        return -1;
    }

        //rt_device_write(uart2_dev, 0, test, sizeof(test));
        // start uart TX/RX

    while(1) {
        rt_kprintf("%u STATE=%d ", rt_tick_get(), current_state);
        switch (current_state) {
            case STATE_WAIT_SET_ADDR:
                //rt_kprintf("%u STATE wait_set_addr.\n", rt_tick_get());
                memset(&rxbuf, 0, sizeof(rxbuf));
                reclen = rt_device_read(uart2_dev, 0, rxbuf, sizeof(rxbuf));
                rt_kprintf(">>read %d %x\n", reclen, rxbuf[0]);
                prx_pkt = NULL;
                if(reclen >= sizeof(UART_PKT)) {
                    // search for header 0xbeef;
                    pbuf = (uint16_t *)rxbuf;
                    for (i = 0; i < reclen - 1; i++) {
                        pbuf = (uint16_t *)&rxbuf[i];
                        if (*pbuf == UART_PKT_HEADER) {
                            if (i + sizeof(UART_PKT) <= reclen) {
                                rt_kprintf("FIND@%d\n", i);
                                prx_pkt = (UART_PKT *)pbuf;
                                break;
                            }
                        }
                        rt_kprintf("%d=%x\n", i, rxbuf[i]);
                    }
                }else {
                    rt_kprintf("receive small packet!\n");
                }
                
                if (prx_pkt != NULL) {
                    memset(&rx_packet, 0, sizeof(UART_PKT));
                    memcpy(&rx_packet, prx_pkt, sizeof(UART_PKT));
                    rt_kprintf("> %x cmd=%d src(%d %d) dst(%d %d) %d\n", 
                    rx_packet.header, rx_packet.command, rx_packet.srcx, rx_packet.srcy,
                    rx_packet.dstx, rx_packet.dsty, rx_packet.data);
                    local_addrx = rx_packet.dstx;
                    local_addry = rx_packet.dsty;

                    if (CMD_SETADDR == rx_packet.command) {
                        rt_kprintf("GET ADDR sending ACK..\n");
                        current_state = STATE_SEND_ACK;
                    }
                }
                break;
            case STATE_WAIT_REPORT_ACK:
                // 2
                rt_kprintf("%u wait report ack\n", rt_tick_get());
                memset(&rxbuf, 0, sizeof(rxbuf));
                reclen = rt_device_read(uart2_dev, 0, rxbuf, sizeof(rxbuf));
                rt_kprintf(">>reclen=%d %x\n", reclen, rxbuf[0]);
                prx_pkt = NULL;
                if(reclen >= sizeof(UART_PKT)) {
                    // search for header 0xbeef;
                    pbuf = (uint16_t *)rxbuf;
                    for (i = 0; i < reclen - 1; i++) {
                        pbuf = (uint16_t *)&rxbuf[i];
                        if (*pbuf == UART_PKT_HEADER) {
                            if (i + sizeof(UART_PKT) <= reclen) {
                                rt_kprintf("FIND@%d\n", i);
                                prx_pkt = (UART_PKT *)pbuf;
                                break;
                            }
                        }
                        rt_kprintf("%d=%x\n", i, rxbuf[i]);
                    }
                }else {
                    rt_kprintf("receive small packet!\n");
                    current_state = STATE_SEND_REPORT;
                }
                
                if (prx_pkt != NULL) {
                    memset(&rx_packet, 0, sizeof(UART_PKT));
                    memcpy(&rx_packet, prx_pkt, sizeof(UART_PKT));
                    rt_kprintf("> %x cmd=%d src(%d %d) dst(%d %d) %d\n", 
                    rx_packet.header, rx_packet.command, rx_packet.srcx, rx_packet.srcy,
                    rx_packet.dstx, rx_packet.dsty, rx_packet.data);
                    local_addrx = rx_packet.dstx;
                    local_addry = rx_packet.dsty;

                    switch (rx_packet.command) {
                        case CMD_SETADDR:
                            rt_kprintf("Duplicated GET ADDR!\n");
                            current_state = STATE_SEND_ACK;
                            break;
                        case CMD_ACK:
                            break;
                        case CMD_REPORT_ACK:
                            rt_kprintf("GET REPORT ACK finsih!\n");
                            current_state = STATE_SET_NEXT_ADDR;
                            break;
                        default:
                            break;
                    }
                } else {
                    current_state = STATE_SEND_REPORT;
                }
                break;
            case STATE_SEND_REPORT:
                rt_kprintf("<<send out REPORT\n");
                memset(&tx_packet, 0, sizeof(UART_PKT));
                tx_packet.header = UART_PKT_HEADER;
                tx_packet.command = CMD_REPORT;
                tx_packet.srcx = local_addrx;
                tx_packet.srcy = local_addry;
                tx_packet.dstx = AP_ADDRX;
                tx_packet.dsty = AP_ADDRY;
                tx_packet.data = rt_tick_get();
                rt_device_write(uart2_dev, 0, &tx_packet, sizeof(tx_packet));
                current_state = STATE_WAIT_REPORT_ACK;
                break;
            case STATE_SEND_ACK:
                //while (1) 
                {
                rt_kprintf("<<send out ACK\n");
                memset(&tx_packet, 0, sizeof(UART_PKT));
                tx_packet.header = UART_PKT_HEADER;
                tx_packet.command = CMD_ACK;
                tx_packet.srcx = AP_ADDRX;
                tx_packet.srcy = AP_ADDRY;
                tx_packet.dstx = rx_packet.srcx;
                tx_packet.dsty = rx_packet.srcy;
                tx_packet.data = rt_tick_get();
                rt_device_write(uart2_dev, 0, &tx_packet, sizeof(tx_packet));
                current_state = STATE_SEND_REPORT;
                }
                break;
            case STATE_SET_NEXT_ADDR:
                // write to uart2/3/4, then wait for by-pass packet, and send to uart1(upstream)
                rt_kprintf("<<send out SET_NEXT_ADDR\n");
                memset(&tx_packet, 0, sizeof(UART_PKT));
                tx_packet.header = UART_PKT_HEADER;
                tx_packet.command = CMD_SETADDR;
                tx_packet.srcx = local_addrx;
                tx_packet.srcy = local_addry;
                tx_packet.dstx = local_addrx + 1;
                tx_packet.dsty = local_addry;
                tx_packet.data = rt_tick_get();
                rt_device_write(uart3_dev, 0, &tx_packet, sizeof(tx_packet));
                current_state = STATE_WAIT_NEXT_ADDR_ACK;
                break;
            case STATE_WAIT_NEXT_ADDR_ACK:
                // 6 read uart2/3/4
                rt_kprintf("%u wait for NEXT_ADDR ack\n", rt_tick_get());
                memset(&rxbuf, 0, sizeof(rxbuf));
                reclen = rt_device_read(uart3_dev, 0, rxbuf, sizeof(rxbuf));
                rt_kprintf(">>reclen=%d %x\n", reclen, rxbuf[0]);
                prx_pkt = NULL;
                if(reclen >= sizeof(UART_PKT)) {
                    // search for header 0xbeef;
                    pbuf = (uint16_t *)rxbuf;
                    for (i = 0; i < reclen - 1; i++) {
                        pbuf = (uint16_t *)&rxbuf[i];
                        if (*pbuf == UART_PKT_HEADER) {
                            if (i + sizeof(UART_PKT) <= reclen) {
                                rt_kprintf("FIND@%d\n", i);
                                prx_pkt = (UART_PKT *)pbuf;
                                break;
                            }
                        }
                        rt_kprintf("%d=%x\n", i, rxbuf[i]);
                    }
                }else {
                    rt_kprintf("receive small packet!\n");
                    current_state = STATE_SET_NEXT_ADDR;
                }
                
                if (prx_pkt != NULL) {
                    memset(&rx_packet, 0, sizeof(UART_PKT));
                    memcpy(&rx_packet, prx_pkt, sizeof(UART_PKT));
                    rt_kprintf("> %x cmd=%d src(%d %d) dst(%d %d) %d\n", 
                    rx_packet.header, rx_packet.command, rx_packet.srcx, rx_packet.srcy,
                    rx_packet.dstx, rx_packet.dsty, rx_packet.data);
                    local_addrx = rx_packet.dstx;
                    local_addry = rx_packet.dsty;

                    switch (rx_packet.command) {
                        case CMD_SETADDR:
                            rt_kprintf("Dup GET ADDR!\n");
                            //
                            break;
                        case CMD_ACK:
                            // nex set addr okay
                            current_state = STATE_WAIT_PACKET;
                            break;
                        case CMD_REPORT_ACK:
                            break;
                        default:
                            break;
                    }
                } else {
                    current_state = STATE_SET_NEXT_ADDR;
                }
                break;
            case STATE_WAIT_PACKET:
                // 7 read pass-through report packet from uart3/4; and read upstream ACK
                rt_kprintf("%u wait_packet\n", rt_tick_get());
                memset(&rxbuf, 0, sizeof(rxbuf));
                reclen = rt_device_read(uart3_dev, 0, rxbuf, sizeof(rxbuf));
                rt_kprintf(">>reclen=%d %x\n", reclen, rxbuf[0]);
                prx_pkt = NULL;
                if(reclen >= sizeof(UART_PKT)) {
                    // search for header 0xbeef;
                    pbuf = (uint16_t *)rxbuf;
                    for (i = 0; i < reclen - 1; i++) {
                        pbuf = (uint16_t *)&rxbuf[i];
                        if (*pbuf == UART_PKT_HEADER) {
                            if (i + sizeof(UART_PKT) <= reclen) {
                                rt_kprintf("FIND@%d\n", i);
                                prx_pkt = (UART_PKT *)pbuf;
                                break;
                            }
                        }
                        rt_kprintf("%d=%x\n", i, rxbuf[i]);
                    }
                }else {
                    rt_kprintf("receive small packet!\n");
                    //current_state = STATE_SET_NEXT_ADDR;
                }
                
                if (prx_pkt != NULL) {
                    memset(&rx_packet, 0, sizeof(UART_PKT));
                    memcpy(&rx_packet, prx_pkt, sizeof(UART_PKT));
                    rt_kprintf("> %x cmd=%d src(%d %d) dst(%d %d) %d\n", 
                    rx_packet.header, rx_packet.command, rx_packet.srcx, rx_packet.srcy,
                    rx_packet.dstx, rx_packet.dsty, rx_packet.data);
                    local_addrx = rx_packet.dstx;
                    local_addry = rx_packet.dsty;

                    switch (rx_packet.command) {
                        case CMD_REPORT:
                            // send to upstream
                            rt_kprintf("<<send PASSTHROUGH REPORT\n");
                            memset(&tx_packet, 0, sizeof(UART_PKT));
                            tx_packet.header = UART_PKT_HEADER;
                            tx_packet.command = CMD_REPORT;
                            tx_packet.srcx = rx_packet.srcx;
                            tx_packet.srcy = rx_packet.srcy;
                            tx_packet.dstx = AP_ADDRX;
                            tx_packet.dsty = AP_ADDRY;
                            tx_packet.data = rt_tick_get();
                            rt_device_write(uart2_dev, 0, &tx_packet, sizeof(tx_packet));
                            break;
                        case CMD_SETADDR:
                            break;
                        case CMD_ACK:
                            break;
                        case CMD_REPORT_ACK:
                            break;
                        default:
                            break;
                    }
                } else {
                    //current_state = STATE_SET_NEXT_ADDR;
                }

                ///
                memset(&rxbuf, 0, sizeof(rxbuf));
                reclen = rt_device_read(uart2_dev, 0, rxbuf, sizeof(rxbuf));
                rt_kprintf(">>reclen=%d %x\n", reclen, rxbuf[0]);
                prx_pkt = NULL;
                if(reclen >= sizeof(UART_PKT)) {
                    // search for header 0xbeef;
                    pbuf = (uint16_t *)rxbuf;
                    for (i = 0; i < reclen - 1; i++) {
                        pbuf = (uint16_t *)&rxbuf[i];
                        if (*pbuf == UART_PKT_HEADER) {
                            if (i + sizeof(UART_PKT) <= reclen) {
                                rt_kprintf("FIND@%d\n", i);
                                prx_pkt = (UART_PKT *)pbuf;
                                break;
                            }
                        }
                        rt_kprintf("%d=%x\n", i, rxbuf[i]);
                    }
                }else {
                    rt_kprintf("receive small packet!\n");
                    //current_state = STATE_SET_NEXT_ADDR;
                }
                
                if (prx_pkt != NULL) {
                    memset(&rx_packet, 0, sizeof(UART_PKT));
                    memcpy(&rx_packet, prx_pkt, sizeof(UART_PKT));
                    rt_kprintf("> %x cmd=%d src(%d %d) dst(%d %d) %d\n", 
                    rx_packet.header, rx_packet.command, rx_packet.srcx, rx_packet.srcy,
                    rx_packet.dstx, rx_packet.dsty, rx_packet.data);
                    local_addrx = rx_packet.dstx;
                    local_addry = rx_packet.dsty;

                    switch (rx_packet.command) {
                        case CMD_REPORT:
                            break;
                        case CMD_SETADDR:
                            break;
                        case CMD_ACK:
                            break;
                        case CMD_REPORT_ACK:
                            // report ACK
                            memset(&tx_packet, 0, sizeof(UART_PKT));
                            tx_packet.header = UART_PKT_HEADER;
                            tx_packet.command = CMD_REPORT_ACK;
                            tx_packet.srcx = rx_packet.srcx;
                            tx_packet.srcy = rx_packet.srcy;
                            tx_packet.dstx = rx_packet.dstx;
                            tx_packet.dsty = rx_packet.dstx;
                            tx_packet.data = rt_tick_get();
                            rt_device_write(uart3_dev, 0, &tx_packet, sizeof(tx_packet));
                            break;
                        default:
                            break;
                    }
                } else {
                    //current_state = STATE_SET_NEXT_ADDR;
                }

                break;
            default:
                rt_kprintf("State %d\n", current_state);
                break;
        }
        rt_thread_delay(RT_TICK_PER_SECOND/20);
    }
    rt_device_close(uart2_dev);
    return 0;
}
MSH_CMD_EXPORT(uart22_test, uart2 test);


static int uart3_test(int argc, char** argv)
{
        rt_err_t ret = RT_ERROR;
        
        char test[80] = "uart3>";
        rt_kprintf("Hello RT-Thread!\n");
    
        uart3_dev = rt_device_find("uart3");
        if (uart3_dev != RT_NULL)
        {
            ret = rt_device_open(uart3_dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
            if (ret != RT_EOK) {
                rt_kprintf("Open error!\n");
                return -1;
            }
    
            rt_device_write(uart3_dev, 0, test, sizeof(test));
            while(1){
    
            rt_size_t reclen = rt_device_read(uart3_dev, 0, test, 10);
            if(reclen > 0) 
                rt_device_write(uart3_dev, 0, test, reclen);
            rt_thread_delay(RT_TICK_PER_SECOND);
        }
    }

    return 0;
}
MSH_CMD_EXPORT(uart3_test, uart3 test);

static rt_device_t uart4_dev = RT_NULL;
static int uart4_test(int argc, char** argv)
{
        rt_err_t ret = RT_ERROR;
        
        char test[80] = "uart4>";
        rt_kprintf("Hello RT-Thread!\n");
    
        uart4_dev = rt_device_find("uart4");
        if (uart4_dev != RT_NULL)
        {
            ret = rt_device_open(uart4_dev, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
            if (ret != RT_EOK) {
                rt_kprintf("Open error!\n");
                return -1;
            }
    
            rt_device_write(uart4_dev, 0, test, sizeof(test));
            while(1){
            rt_size_t reclen = rt_device_read(uart4_dev, 0, test, 10);
            if(reclen > 0) 
                rt_device_write(uart4_dev, 0, test, reclen);
            rt_thread_delay(RT_TICK_PER_SECOND);
        }
    }

    return 0;
}
MSH_CMD_EXPORT(uart4_test, uart4 test);

