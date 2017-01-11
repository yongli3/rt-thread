#include <lwip/api.h>

#define UDP_ECHO_PORT   7

static void udpecho_entry(void *parameter)
{
	struct netconn *conn;
	struct netbuf *buf;
	struct ip_addr *addr;
	unsigned short port;

	conn = netconn_new(NETCONN_UDP);
	netconn_bind(conn, IP_ADDR_ANY, UDP_ECHO_PORT);

	while(1)
	{
        /* received data to buffer */
#if LWIP_VERSION_MINOR==3U 
		buf = netconn_recv(conn);
#else
		netconn_recv(conn, &buf);
#endif
		addr = netbuf_fromaddr(buf);
		port = netbuf_fromport(buf);
#if LWIP_VERSION_MINOR==3U
        /* send the data to buffer */
		netconn_connect(conn, addr, port);
		/* reset address, and send to client */
		buf->addr = RT_NULL;
#else        
#endif
		netconn_send(conn, buf);

        /* release buffer */
		netbuf_delete(buf);
	}
}

#ifdef RT_USING_FINSH
#include <finsh.h>
static rt_thread_t echo_tid = RT_NULL;
void udpecho(rt_uint32_t startup)
{
	if (startup && echo_tid == RT_NULL)
	{
		echo_tid = rt_thread_create("uecho",
									udpecho_entry, RT_NULL,
									512, 30, 5);
		if (echo_tid != RT_NULL)
			rt_thread_startup(echo_tid);
	}
	else
	{
		if (echo_tid != RT_NULL)
			rt_thread_delete(echo_tid); /* delete thread */
		echo_tid = RT_NULL;
	}
}

int cmd_udpecho(int argc, char **argv)
{
    udpecho(1);
}

FINSH_FUNCTION_EXPORT(cmd_udpecho, startup or stop UDP echo server);
#endif
//MSH_CMD_EXPORT(udpecho, startup or stop UDP echo server);
FINSH_FUNCTION_EXPORT_ALIAS(cmd_udpecho, __cmd_udpecho, ping network host);

