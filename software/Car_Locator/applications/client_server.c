#include <string.h>
#include <stdarg.h>	
#include <stdio.h>
#include <sys/socket.h>
#include <at_socket.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME "ali.sample"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#include "gps.h"

static rt_thread_t recv_data_pro_thread = RT_NULL;

rt_int32_t g_socket_id = -1;
char g_post_data_src[512];
char g_get_data_src[512];

extern nmea_msg gps_data;


/**************************************************************
��������:	socket_create
��������:	socket����
�������:	��
�� �� ֵ:	socket_id
��    ע:	��
**************************************************************/
rt_int32_t socket_create(void)
{
		rt_int32_t socket_id = -1;
		unsigned char domain = AF_INET;
		unsigned char type = SOCK_STREAM;
		unsigned char protocol = IPPROTO_IP;
		
		socket_id = at_socket(domain, type, protocol);

		if(socket_id < 0)
		{
				LOG_E("Socket create failure, socket_id:%d\n", socket_id);
		}
		else
		{
				LOG_I("Scoket created successfully, socket_id:%d\n", socket_id);
		}
		
		return socket_id;
}

/**************************************************************
��������:	socket_connect
��������:	socket ����Զ�̷�����
�������:	socket_id������socketʱ���ص�id
			remote_addr:Զ�̷�������ַ
			remote_port:�˿�
�� �� ֵ:	RT_EOK�����ӳɹ���RT_ERROR������ʧ��
��    ע:	��
**************************************************************/
rt_err_t socket_connect(rt_int32_t socket_id, char *remote_addr, rt_int32_t remote_port)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(remote_port);
	addr.sin_addr.s_addr = inet_addr(remote_addr);
	
	if(0 == at_connect(socket_id, (struct sockaddr *)&addr, sizeof(addr)))
	{	
		LOG_I("Socket connected to the remote server successfully\n");
		return RT_EOK;
	}
	else
	{
		LOG_E("Socket connected to the remote server failure\n");
		return RT_ERROR;
	}
}

/**************************************************************
��������:	socket_close
��������:	�ر�socket
�������:	socket_id������socketʱ���ص�id
�� �� ֵ:	RT_EOK���رճɹ���RT_ERROR���ر�ʧ��
��    ע:	��
**************************************************************/
rt_err_t socket_close(rt_int32_t socket_id)
{
	if(0 == at_closesocket(socket_id))
	{
		LOG_I("Socket closed successfully\n");
		return RT_EOK;
	}
	else
	{
		LOG_E("Socket closed failure\n");
		return RT_ERROR;
	}
}

/**************************************************************
��������:	socket_send
��������:	socket ��������
�������:	socket_id������socketʱ���ص�id
			data_buf:���ݰ�
			data_len:���ݰ���С
�� �� ֵ:	���ͳɹ��������ݰ���С
��    ע:	��
**************************************************************/
rt_int32_t socket_send(int socket_id, char *data_buf, int data_len)
{
	rt_int32_t send_rec  = 0;

	send_rec = at_send(socket_id, data_buf, data_len, 0);

	return send_rec;
}

/**************************************************************
��������:	socket_receive
��������:	socket ���շ������·�������
�������:	socket_id������socketʱ���ص�id
			data_buf���洢���յ���������
			data_len�������մ�С
�� �� ֵ:	�ɹ�ʱ���ش���0
��    ע:	��
**************************************************************/
rt_int32_t socket_receive(rt_int32_t socket_id, char *data_buf, rt_int32_t data_len)
{
	rt_int32_t recv_result = 0;

	recv_result = at_recv(socket_id, data_buf, data_len, MSG_DONTWAIT);

	return recv_result;
}

/**************************************************************
��������:	receive_aliot_http_process_thread
��������:	����ƽ̨�·��������
�������:	parameter:�߳���ڲ���
�� �� ֵ:	��
��    ע��	��
**************************************************************/
void receive_aliot_http_process_thread(void *parameter)
{
	char data_ptr[384];
	memset(data_ptr, 0, sizeof(data_ptr));
	
	while(1)
	{
		if(socket_receive(g_socket_id, data_ptr, 384) > 0)
		{
			rt_kprintf("\r\nSocket receive data is:\r\n%s\r\n", data_ptr);
			memset(data_ptr, 0, sizeof(data_ptr));
		}
		rt_thread_mdelay(1);
		rt_thread_yield();/* ����ʣ��ʱ��Ƭ������һ���߳��л� */
	}
}

/**************************************************************
��������:	connect_onenet_http_device
��������:	����OneNETƽ̨�豸
�������:	��
�� �� ֵ:	��
��    ע:	��
**************************************************************/
void connect_aliot_http_device(void)
{
		if(g_socket_id >= 0)
		{
				socket_close(g_socket_id);
		}
		
		g_socket_id = socket_create();
		if(g_socket_id >= 0)
		{
				if(RT_EOK == socket_connect(g_socket_id, "139.196.67.150", 80))
				{
						rt_thread_delay(500);
						rt_thread_resume(recv_data_pro_thread);
				}
		}
}

/**************************************************************
��������:	disconnect_aliot_http_device
��������:	�Ͽ����Ӱ�����������ƽ̨�豸
�������:	��
�� �� ֵ:	��
��    ע:	��
**************************************************************/
void disconnect_aliot_http_device(void)
{
	if(RT_EOK == socket_close(g_socket_id))
	{
		rt_thread_delay(500);
		rt_thread_suspend(recv_data_pro_thread);
	}
}

void a9g(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (!strcmp(argv[1], "connect"))
        {
						connect_aliot_http_device();
			//rt_thread_resume(recv_data_pro_thread);
        }
        else if (!strcmp(argv[1], "post"))
        {
//            post_data_stream_to_onenet();
        }
				else if (!strcmp(argv[1], "get"))
        {
//            get_data_stream_from_onenet();
        }
				else if (!strcmp(argv[1], "close"))
				{
						disconnect_aliot_http_device();
//						socket_close(g_socket_id);
//						rt_thread_suspend(recv_data_pro_thread);
				}
        else
        {
            rt_kprintf("Unknown command. Please enter 'a9g' for help\n");
        }
    }
    else
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("a9g connect   - a9g connect to aliot http server\n");
				rt_kprintf("a9g post      - a9g post data stream to aliot http server\n");
				rt_kprintf("a9g get       - a9g get data stream from aliot http server\n");
				rt_kprintf("a9g close   	- a9g close connection\n");
    }
}
MSH_CMD_EXPORT(a9g, a9g module function);

