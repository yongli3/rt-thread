/* RT-Thread config file */
#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__

//#define RT_USING_TC

/* RT_NAME_MAX*/
#define RT_NAME_MAX	   8

/* RT_ALIGN_SIZE*/
#define RT_ALIGN_SIZE	4

/* PRIORITY_MAX */
#define RT_THREAD_PRIORITY_MAX	32

/* Tick per Second */
#define RT_TICK_PER_SECOND	1000

/* SECTION: RT_DEBUG */
/* Thread Debug */
#define RT_DEBUG
//#define RT_DEBUG_MEM 				(1)
//#define RT_DEBUG_SCHEDULER 			(1)
//#define RT_DEBUG_IPC 				(1)
//#define THREAD_DEBUG
//#define IRQ_DEBUG
#define RT_USING_OVERFLOW_CHECK

//#define RT_IRQHDL_DEBUG
//#define RT_USART_DEBUG
//#define RT_LEUART_DEBUG
//#define RT_IIC_DEBUG
//#define RT_MISC_DEBUG
//#define RT_ADC_DEBUG
//#define RT_ACMP_DEBUG
//#define RT_TIMER_DEBUG
//#define RT_RTC_DEBUG
/* Using Hook */
#define RT_USING_HOOK

#define IDLE_THREAD_STACK_SIZE     1024

/* Using Software Timer */
/* #define RT_USING_TIMER_SOFT */
#define RT_TIMER_THREAD_PRIO		4
#define RT_TIMER_THREAD_STACK_SIZE	512
//#define RT_TIMER_TICK_PER_SECOND	(10)

/* SECTION: IPC */
/* Using Semaphore*/
#define RT_USING_SEMAPHORE

/* Using Mutex */
#define RT_USING_MUTEX

/* Using Event */
#define RT_USING_EVENT

/* Using MailBox */
#define RT_USING_MAILBOX

/* Using Message Queue */
#define RT_USING_MESSAGEQUEUE

/* SECTION: Memory Management */
/* Using Memory Pool Management*/
#define RT_USING_MEMPOOL

/* Using Dynamic Heap Management */
#define RT_USING_HEAP

/* Using Small MM */
#define RT_USING_SMALL_MEM

// <bool name="RT_USING_COMPONENTS_INIT" description="Using RT-Thread components initialization" default="true" />
//#define RT_USING_COMPONENTS_INIT

/* SECTION: Device System */
/* Using Device System */
#define RT_USING_DEVICE
#define RT_USING_DEVICE_IPC
/* Using serial framework */
#define RT_USING_SERIAL

#define RT_USING_UART1
#define RT_USING_UART2
//#define RT_USING_UART3

/* Using GPIO pin framework */
#define RT_USING_PIN

/* Using Hardware Timer framework */
//#define RT_USING_HWTIMER

/* SECTION: Console options */
#define RT_USING_CONSOLE
/* the buffer size of console*/
#define RT_CONSOLEBUF_SIZE	128
//#define RT_CONSOLE_DEVICE_NAME	    "uart1"

/* SECTION: finsh, a C-Express shell */
#define RT_USING_FINSH
/* Using symbol table */
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION

#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT

/* SECTION: device filesystem */
/* Using Device file system */
#define RT_USING_DFS
/* the max number of mounted filesystem */
#define DFS_FILESYSTEMS_MAX			2
/* the max number of opened files 		*/
#define DFS_FD_MAX					4

/* Using ELM FATFS */
//#define RT_USING_DFS_ELMFAT
#define RT_DFS_ELM_WORD_ACCESS
/* Reentrancy (thread safe) of the FatFs module.  */
#define RT_DFS_ELM_REENTRANT
/* Number of volumes (logical drives) to be used. */
#define RT_DFS_ELM_DRIVES			2
/* #define RT_DFS_ELM_USE_LFN			1 */
#define RT_DFS_ELM_MAX_LFN			255
/* Maximum sector size to be handled. */
#define RT_DFS_ELM_MAX_SECTOR_SIZE  512

/* Using ROM file system */
// #define RT_USING_DFS_ROMFS

/* SECTION: lwip, a lighwight TCP/IP protocol stack */

#define RT_USING_LWIP
// make sure use the new version
#define RT_USING_LWIP141
/* Trace LwIP protocol */
#define RT_LWIP_DEBUG
#define RT_USING_NETUTILS
/* LwIP uses RT-Thread Memory Management */
#define RT_LWIP_USING_RT_MEM
/* Enable ICMP protocol*/
#define RT_LWIP_ICMP
/* Enable IGMP protocol */
#define RT_LWIP_IGMP
/* Enable UDP protocol*/
#define RT_LWIP_UDP
/* Enable TCP protocol*/
#define RT_LWIP_TCP
/* Enable DNS */
#define RT_LWIP_DNS

/* the number of simulatenously active TCP connections*/
#define RT_LWIP_TCP_PCB_NUM	5

/* Using DHCP */
#define RT_LWIP_DHCP

/* ip address of target*/
#define RT_LWIP_IPADDR0	192
#define RT_LWIP_IPADDR1	168
#define RT_LWIP_IPADDR2	0
#define RT_LWIP_IPADDR3	201

/* gateway address of target*/
#define RT_LWIP_GWADDR0	192
#define RT_LWIP_GWADDR1	168
#define RT_LWIP_GWADDR2	0
#define RT_LWIP_GWADDR3	3

/* mask address of target*/
#define RT_LWIP_MSKADDR0	255
#define RT_LWIP_MSKADDR1	255
#define RT_LWIP_MSKADDR2	255
#define RT_LWIP_MSKADDR3	0

/* tcp thread options */
#define RT_LWIP_TCPTHREAD_PRIORITY		12
#define RT_LWIP_TCPTHREAD_MBOX_SIZE		4
#define RT_LWIP_TCPTHREAD_STACKSIZE		1024

/* ethernet if thread options */
#define RT_LWIP_ETHTHREAD_PRIORITY		15
#define RT_LWIP_ETHTHREAD_MBOX_SIZE		4
#define RT_LWIP_ETHTHREAD_STACKSIZE		512

/* TCP sender buffer space */
#define RT_LWIP_TCP_SND_BUF	8192
/* TCP receive window. */
#define RT_LWIP_TCP_WND		8192

/* RT_GDB_STUB */
//#define RT_USING_GDB

#endif
