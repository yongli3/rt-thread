#include <rtthread.h>
#include <lwip/sockets.h> /* 使用BSD Socket接口必须包含sockets.h这个头文件 */

static const char send_data[] = "This is TCP Server from RT-Thread."; /* 发送用到的数据 */

#define BUFF_SIZE  (2048)

unsigned char uart2_rx_buffer[BUFF_SIZE];

struct rt_semaphore uart2_sem;

struct paramet {
    rt_device_t uart2dev;
    int connected;
};

static rt_err_t uart2_rx_ind(rt_device_t dev, rt_size_t size)
{
    rt_kprintf("rx %d\n", size);
    return rt_sem_release(&uart2_sem);
}

// monitor the uart2 RX
static void uart2in_thread_entry(void* parameter)
{
    int connected;
    rt_size_t read_len;

    struct paramet *par = parameter;
    
    rt_kprintf("+%s\n", __func__);

    rt_sem_take(&uart2_sem, RT_WAITING_FOREVER);

    read_len = rt_device_read(par->uart2dev, 0, uart2_rx_buffer, sizeof(uart2_rx_buffer));
    rt_kprintf("uart2 read=%d\n", read_len);

    // send to network
    connected = par->connected;
    send(connected, uart2_rx_buffer, read_len, 0);
}

void tcpserv(void* parameter)
{
   char *recv_data; /* 用于接收的指针，后面会做一次动态分配以请求可用内存 */
   rt_uint32_t sin_size;
   int sock, connected, bytes_received;
   unsigned int total_received = 0;
   struct sockaddr_in server_addr, client_addr;
   rt_bool_t stop = RT_FALSE; /* 停止标志 */
   rt_device_t uart2dev;
   rt_err_t res;
   rt_thread_t thread;
   struct paramet par;
   int int_lvl;
   rt_uint16_t odev_flag;

   recv_data = rt_malloc(BUFF_SIZE); /* 分配接收用的数据缓冲 */
   if (recv_data == RT_NULL)
   {
       rt_kprintf("No memory\n");
       return;
   }

    // open uart2 for data write
    uart2dev = rt_device_find("uart2");
    if (!uart2dev)
    {
        rt_kprintf("could not find odev\n");
    }

    rt_sem_init(&uart2_sem, "uart2_sem", 0, RT_IPC_FLAG_FIFO);
    
    int_lvl = rt_hw_interrupt_disable();
    rt_device_set_rx_indicate(uart2dev, uart2_rx_ind);
    odev_flag = uart2dev->flag;
    uart2dev->flag &= ~RT_DEVICE_FLAG_STREAM;
    rt_hw_interrupt_enable(int_lvl);
    
    res = rt_device_open(uart2dev, uart2dev->flag);
    if (res != RT_EOK)
    {
        rt_kprintf("open output device error: 0x%x", -res);
        return;
    }

    rt_device_write(uart2dev, 0, send_data, strlen(send_data));

#if 0
while (1) 
{
    rt_kprintf("uart2 ... ");
    rt_sem_take(&uart2_sem, RT_WAITING_FOREVER);
    bytes_received = rt_device_read(uart2dev, 0, uart2_rx_buffer, sizeof(uart2_rx_buffer));
    rt_kprintf("recv=%d\n", bytes_received);
    //rt_thread_delay(RT_TICK_PER_SECOND/10);
}
#endif

   /* 一个socket在使用前，需要预先创建出来，指定SOCK_STREAM为TCP的socket */
   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
       /* 创建失败的错误处理 */
       rt_kprintf("Socket error\n");

       /* 释放已分配的接收缓冲 */
       rt_free(recv_data);
       return;
   }

   /* 初始化服务端地址 */
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(5000); /* 服务端工作的端口 */
   server_addr.sin_addr.s_addr = INADDR_ANY;
   rt_memset(&(server_addr.sin_zero),8, sizeof(server_addr.sin_zero));

   /* 绑定socket到服务端地址 */
   if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
   {
       /* 绑定失败 */
       rt_kprintf("Unable to bind\n");

       /* 释放已分配的接收缓冲 */
       rt_free(recv_data);
       return;
   }

   /* 在socket上进行监听 */
   if (listen(sock, 5) == -1)
   {
       rt_kprintf("Listen error\n");

       /* release recv buffer */
       rt_free(recv_data);
       return;
   }

   rt_kprintf("\nTCPServer Waiting for client on port 5000...\n");
   while(stop != RT_TRUE)
   {
       sin_size = sizeof(struct sockaddr_in);

       /* 接受一个客户端连接socket的请求，这个函数调用是阻塞式的 */
       connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
       /* 返回的是连接成功的socket */

       /* 接受返回的client_addr指向了客户端的地址信息 */
       rt_kprintf("I got a connection from (%s , %d)\n",
                  inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

#if 1
        // start the uart2 input monitor thread
        par.connected = connected;
        par.uart2dev =uart2dev;
        thread = rt_thread_create("uart2in", uart2in_thread_entry, &par, 512, 20, 20);
        if(thread != RT_NULL) {
            rt_thread_startup(thread);
        } else {
            rt_kprintf("thread create fail!\n");
        }
#endif
       /* 客户端连接的处理 */
        total_received = 0;
       while (1)
       {
           /* 发送数据到connected socket */
           //send(connected, send_data, strlen(send_data), 0);

           /* 从connected socket中接收数据，接收buffer是BUFF_SIZE大小，但并不一定能够收到BUFF_SIZE大小的数据 */
           
           bytes_received = recv(connected, recv_data, BUFF_SIZE, 0);
           total_received += bytes_received;
           rt_kprintf("%d rcv=%d total=%d\n", rt_tick_get(), bytes_received, total_received);
           
           if (bytes_received <= 0)
           {
               /* 接收失败，关闭这个connected socket */
               lwip_close(connected);
               break;
           }

           #if 0
           /* 有接收到数据，把末端清零 */
           recv_data[bytes_received] = '\0';
           if (strcmp(recv_data , "q") == 0 || strcmp(recv_data , "Q") == 0)
           {
               /* 如果是首字母是q或Q，关闭这个连接 */
               lwip_close(connected);
               break;
           }
           else if (strcmp(recv_data, "exit") == 0)
           {
               /* 如果接收的是exit，则关闭整个服务端 */
               lwip_close(connected);
               stop = RT_TRUE;
               break;
           }
           else
           #endif 
           {
               /* 在控制终端显示收到的数据 */
               //rt_kprintf("RECEIVED DATA = %s \n" , recv_data);
               rt_device_write(uart2dev, 0, recv_data, bytes_received);
           }
       }
   }

   /* 退出服务 */
   lwip_close(sock);

   /* 释放接收缓冲 */
   rt_free(recv_data);

    rt_device_close(uart2dev);
   return ;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
/* 输出tcpserv函数到finsh shell中 */
FINSH_FUNCTION_EXPORT(tcpserv, startup tcp server);
#endif
MSH_CMD_EXPORT(tcpserv, startup tcp server);
