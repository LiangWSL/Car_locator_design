/*
 * File      : at_socket_a9g.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-12     malongwei    first version
 * 2019-05-13     chenyong     multi AT socket client support
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <at_device_a9g.h>

#if !defined(AT_SW_VERSION_NUM) || AT_SW_VERSION_NUM < 0x10300
#error "This AT Client version is older, please check and update latest AT Client!"
#endif

#define LOG_TAG                        "at.dev"
#include <at_log.h>
    
#ifdef AT_DEVICE_USING_A9G

#define A9G_WAIT_CONNECT_TIME      5000
#define A9G_THREAD_STACK_SIZE      1024
#define A9G_THREAD_PRIORITY        (RT_THREAD_PRIORITY_MAX/2)

/* AT+CSTT command default*/
static char *CSTT_CHINA_MOBILE  = "AT+CSTT=\"CMNET\"";
static char *CSTT_CHINA_UNICOM  = "AT+CSTT=\"UNINET\"";
static char *CSTT_CHINA_TELECOM = "AT+CSTT=\"CTNET\"";

static void a9g_power_on(struct at_device *device)
{
    struct at_device_a9g *a9g = RT_NULL;

    a9g = (struct at_device_a9g *) device->user_data;

    /* not nead to set pin configuration for m26 device power on */
    if (a9g->power_pin == -1 || a9g->power_status_pin == -1)
    {
        return;
    }

    if (rt_pin_read(a9g->power_status_pin) == PIN_HIGH)
    {
        return;
    }
    rt_pin_write(a9g->power_pin, PIN_HIGH);

    while (rt_pin_read(a9g->power_status_pin) == PIN_LOW)
    {
        rt_thread_mdelay(10);
    }
    rt_pin_write(a9g->power_pin, PIN_LOW);
}

static void a9g_power_off(struct at_device *device)
{
    struct at_device_a9g *a9g = RT_NULL;

    a9g = (struct at_device_a9g *) device->user_data;

    /* not nead to set pin configuration for m26 device power on */
    if (a9g->power_pin == -1 || a9g->power_status_pin == -1)
    {
        return;
    }

    if (rt_pin_read(a9g->power_status_pin) == PIN_LOW)
    {
        return;
    }
    rt_pin_write(a9g->power_pin, PIN_HIGH);

    while (rt_pin_read(a9g->power_status_pin) == PIN_HIGH)
    {
        rt_thread_mdelay(10);
    }
    rt_pin_write(a9g->power_pin, PIN_LOW);
}

/* =============================  a9g network interface operations ============================= */

/* set a9g network interface device status and address information */
static int a9g_netdev_set_info(struct netdev *netdev)
{
#define a9g_IEMI_RESP_SIZE      32
#define a9g_IPADDR_RESP_SIZE    32
#define a9g_DNS_RESP_SIZE       96
#define a9g_INFO_RESP_TIMO      rt_tick_from_millisecond(300)

    int result = RT_EOK;
    ip_addr_t addr;
    at_response_t resp = RT_NULL;
    struct at_device *device = RT_NULL;

    if (netdev == RT_NULL)
    {
        LOG_E("input network interface device is NULL.");
        return -RT_ERROR;
    }

    device = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV, netdev->name);
    if (device == RT_NULL)
    {
        LOG_E("get a9g device by netdev name(%s) failed.", netdev->name);
        return -RT_ERROR;
    }

    /* set network interface device status */
    netdev_low_level_set_status(netdev, RT_TRUE);
    netdev_low_level_set_link_status(netdev, RT_TRUE);
    netdev_low_level_set_dhcp_status(netdev, RT_TRUE);

    resp = at_create_resp(a9g_IEMI_RESP_SIZE, 0, a9g_INFO_RESP_TIMO);
    if (resp == RT_NULL)
    {
        LOG_E("a9g device(%s) set IP address failed, no memory for response object.", device->name);
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* set network interface device hardware address(IEMI) */
    {
        #define a9g_NETDEV_HWADDR_LEN   8
        #define a9g_IEMI_LEN            15

        char iemi[a9g_IEMI_LEN] = {0};
        int i = 0, j = 0;

        /* send "AT+GSN" commond to get device IEMI */
        if (at_obj_exec_cmd(device->client, resp, "AT+GSN") < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        if (at_resp_parse_line_args(resp, 2, "%s", iemi) <= 0)
        {
            LOG_E("a9g device(%s) prase \"AT+GSN\" commands resposne data error.", device->name);
            result = -RT_ERROR;
            goto __exit;
        }

        LOG_D("a9g device(%s) IEMI number: %s", device->name, iemi);

        netdev->hwaddr_len = a9g_NETDEV_HWADDR_LEN;
        /* get hardware address by IEMI */
        for (i = 0, j = 0; i < a9g_NETDEV_HWADDR_LEN && j < a9g_IEMI_LEN; i++, j += 2)
        {
            if (j != a9g_IEMI_LEN - 1)
            {
                netdev->hwaddr[i] = (iemi[j] - '0') * 10 + (iemi[j + 1] - '0');
            }
            else
            {
                netdev->hwaddr[i] = (iemi[j] - '0');
            }
        }
    }

    /* set network interface device IP address */
    {
        #define IP_ADDR_SIZE_MAX    16
        char ipaddr[IP_ADDR_SIZE_MAX] = {0};

        at_resp_set_info(resp, a9g_IPADDR_RESP_SIZE, 2, a9g_INFO_RESP_TIMO);

        /* send "AT+CIFSR" commond to get IP address */
        if (at_obj_exec_cmd(device->client, resp, "AT+CIFSR") < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        if (at_resp_parse_line_args_by_kw(resp, ".", "%s", ipaddr) <= 0)
        {
            LOG_E("a9g device(%s) prase \"AT+CIFSR\" commands resposne data error!", device->name);
            result = -RT_ERROR;
            goto __exit;
        }

        LOG_D("a9g device(%s) IP address: %s", device->name, ipaddr);

        /* set network interface address information */
        inet_aton(ipaddr, &addr);
        netdev_low_level_set_ipaddr(netdev, &addr);
    }

    /* set network interface device dns server */
    {
        #define DNS_ADDR_SIZE_MAX   16
        char dns_server1[DNS_ADDR_SIZE_MAX] = {0}, dns_server2[DNS_ADDR_SIZE_MAX] = {0};

        at_resp_set_info(resp, a9g_DNS_RESP_SIZE, 0, a9g_INFO_RESP_TIMO);

        /* send "AT+CDNSCFG?" commond to get DNS servers address */
        if (at_obj_exec_cmd(device->client, resp, "AT+CDNSCFG?") < 0)
        {
            result = -RT_ERROR;
            goto __exit;
        }

        if (at_resp_parse_line_args_by_kw(resp, "PrimaryDns:", "PrimaryDns:%s", dns_server1) <= 0 ||
            at_resp_parse_line_args_by_kw(resp, "SecondaryDns:", "SecondaryDns:%s", dns_server2) <= 0)
        {
            LOG_E("Prase \"AT+CDNSCFG?\" commands resposne data error!");
            result = -RT_ERROR;
            goto __exit;
        }

        LOG_D("a9g device(%s) primary DNS server address: %s", device->name, dns_server1);
        LOG_D("a9g device(%s) secondary DNS server address: %s", device->name, dns_server2);

        inet_aton(dns_server1, &addr);
        netdev_low_level_set_dns_server(netdev, 0, &addr);

        inet_aton(dns_server2, &addr);
        netdev_low_level_set_dns_server(netdev, 1, &addr);
    }

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }
    
    return result;
}

static void check_link_status_entry(void *parameter)
{
#define a9g_LINK_STATUS_OK   1
#define a9g_LINK_RESP_SIZE   64
#define a9g_LINK_RESP_TIMO   (3 * RT_TICK_PER_SECOND)
#define a9g_LINK_DELAY_TIME  (30 * RT_TICK_PER_SECOND)

    at_response_t resp = RT_NULL;
    int result_code, link_status;
    struct at_device *device = RT_NULL;
    struct netdev *netdev = (struct netdev *)parameter;

    device = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV, netdev->name);
    if (device == RT_NULL)
    {
        LOG_E("get a9g device by netdev name(%s) failed.", netdev->name);
        return;
    }

    resp = at_create_resp(a9g_LINK_RESP_SIZE, 0, a9g_LINK_RESP_TIMO);
    if (resp == RT_NULL)
    {
        LOG_E("a9g device(%s) set check link status failed, no memory for response object.", device->name);
        return;
    }

    while (1)
    {
        extern rt_sem_t dynamic_sem;//<-- add by moneng
        rt_err_t result = RT_NULL;//<-- add by moneng
        result = rt_sem_trytake(dynamic_sem);//<-- add by moneng
        if(result == RT_EOK)//<-- add by moneng
        {
            #if USED_SIM800C
            /* send "AT+CGREG?" commond  to check netweork interface device link status */
            if (at_obj_exec_cmd(device->client, resp, "AT+CGREG?") < 0)
            {
                rt_thread_mdelay(a9g_LINK_DELAY_TIME);
                continue;
            }
            link_status = -1;
            at_resp_parse_line_args_by_kw(resp, "+CGREG:", "+CGREG: %d,%d", &result_code, &link_status);
            #endif
            #if USED_A9_A9G
            /* send "AT+CREG?" commond  to check netweork interface device link status */
            if (at_obj_exec_cmd(device->client, resp, "AT+CREG?") < 0)
            {
                rt_thread_mdelay(a9g_LINK_DELAY_TIME);
                continue;
            }
            link_status = -1;
            at_resp_parse_line_args_by_kw(resp, "+CREG:", "+CREG: %d,%d", &result_code, &link_status);
            #endif

            /* check the network interface device link status  */
            if ((a9g_LINK_STATUS_OK == link_status) != netdev_is_link_up(netdev))
            {
                netdev_low_level_set_link_status(netdev, (a9g_LINK_STATUS_OK == link_status));
            }
            rt_sem_release(dynamic_sem);//<-- add by moneng
        }
        rt_thread_mdelay(a9g_LINK_DELAY_TIME);
    }
}

static int a9g_netdev_check_link_status(struct netdev *netdev)
{
#define a9g_LINK_THREAD_TICK           20
#define a9g_LINK_THREAD_STACK_SIZE     512
#define a9g_LINK_THREAD_PRIORITY       (RT_THREAD_PRIORITY_MAX - 2)

    rt_thread_t tid;
    char tname[RT_NAME_MAX] = {0};

    if (netdev == RT_NULL)
    {
        LOG_E("input network interface device is NULL.\n");
        return -RT_ERROR;
    }

    rt_snprintf(tname, RT_NAME_MAX, "%s_link", netdev->name);

    tid = rt_thread_create(tname, check_link_status_entry, (void *) netdev, 
            a9g_LINK_THREAD_STACK_SIZE, a9g_LINK_THREAD_PRIORITY, a9g_LINK_THREAD_TICK);
    if (tid)
    {
        rt_thread_startup(tid);
    }

    return RT_EOK;
}

static int a9g_net_init(struct at_device *device);

static int a9g_netdev_set_up(struct netdev *netdev)
{
    struct at_device *device = RT_NULL;

    device = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV, netdev->name);
    if (device == RT_NULL)
    {
        LOG_E("get a9g device by netdev name(%s) failed.", netdev->name);
        return -RT_ERROR;
    }

    if (device->is_init == RT_FALSE)
    {
        a9g_net_init(device);
        //device->is_init = RT_TRUE;//<-- 感觉这里应该由 a9g_net_init 决定是否修改 device->is_init
        
        //netdev_low_level_set_status(netdev, RT_TRUE);//<-- 感觉这里应该由 a9g_net_init 决定是否修改 netdev->flags ?= NETDEV_FLAG_UP
        LOG_D("the network interface device(%s) set up status.", netdev->name);
    }

    return RT_EOK;
}

static int a9g_netdev_set_down(struct netdev *netdev)
{
    struct at_device *device = RT_NULL;

    device = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV, netdev->name);
    if (device == RT_NULL)
    {
        LOG_E("get a9g device by netdev name(%s) failed.", netdev->name);
        return -RT_ERROR;
    }

    if (device->is_init == RT_TRUE)
    {
        a9g_power_off(device);
        device->is_init = RT_FALSE;

        netdev_low_level_set_status(netdev, RT_FALSE);
        LOG_D("the network interface device(%s) set down status.", netdev->name);
    }

    return RT_EOK;
}

static int a9g_netdev_set_dns_server(struct netdev *netdev, uint8_t dns_num, ip_addr_t *dns_server)
{
#define a9g_DNS_RESP_LEN     8
#define a9g_DNS_RESP_TIMEO   rt_tick_from_millisecond(300)

    int result = RT_EOK;
    at_response_t resp = RT_NULL;
    struct at_device *device = RT_NULL;

    RT_ASSERT(netdev);
    RT_ASSERT(dns_server);

    device = at_device_get_by_name(AT_DEVICE_NAMETYPE_NETDEV, netdev->name);
    if (device == RT_NULL)
    {
        LOG_E("get a9g device by netdev name(%s) failed.", netdev->name);
        return -RT_ERROR;
    }

    resp = at_create_resp(a9g_DNS_RESP_LEN, 0, a9g_DNS_RESP_TIMEO);
    if (resp == RT_NULL)
    {
        LOG_D("a9g set dns server failed, no memory for response object.");
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* send "AT+CDNSCFG=<pri_dns>[,<sec_dns>]" commond to set dns servers */
    if (at_obj_exec_cmd(device->client, resp, "AT+CDNSCFG=\"%s\"", inet_ntoa(*dns_server)) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }

    netdev_low_level_set_dns_server(netdev, dns_num, dns_server);

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;
}

static int a9g_ping_domain_resolve(struct at_device *device, const char *name, char ip[16])
{
    int result = RT_EOK;
    char recv_ip[16] = { 0 };
    at_response_t resp = RT_NULL;

    /* The maximum response time is 14 seconds, affected by network status */
    #if BSP_USING_A9G
    resp = at_create_resp(128, 3, 14 * RT_TICK_PER_SECOND);
    #endif
    if (resp == RT_NULL)
    {
        LOG_E("no memory for a9g device(%s) response structure.", device->name);
        return -RT_ENOMEM;
    }

    #if BSP_USING_A9G
    if (at_obj_exec_cmd(device->client, resp, "AT+CGACT=1,1") < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }
    #endif
    if (at_obj_exec_cmd(device->client, resp, "AT+CDNSGIP=\"%s\"", name) < 0)
    {
        result = -RT_ERROR;
        goto __exit;
    }

    /* parse the third line of response data, get the IP address */
    if (at_resp_parse_line_args_by_kw(resp, "+CDNSGIP:", "%*[^,],%*[^,],\"%[^\"]", recv_ip) < 0)
    {
        rt_thread_mdelay(100);
        /* resolve failed, maybe receive an URC CRLF */
    }

    if (rt_strlen(recv_ip) < 8)
    {
        rt_thread_mdelay(100);
        /* resolve failed, maybe receive an URC CRLF */
    }
    else
    {
        rt_strncpy(ip, recv_ip, 15);
        ip[15] = '\0';
    }

__exit:
    if (resp)
    {
        at_delete_resp(resp);
    }

    return result;
}

#ifdef NETDEV_USING_PING
#if BSP_USING_A9G
static int a9g_netdev_ping(struct netdev *netdev, const char *host, 
        size_t data_len, uint32_t timeout, struct netdev_ping_resp *ping_resp)
{
    rt_kprintf("I don't have PING function!\r\n");
    return RT_EOK;
}
#endif
#endif /* NETDEV_USING_PING */

#ifdef NETDEV_USING_NETSTAT
void a9g_netdev_netstat(struct netdev *netdev)
{ 
    // TODO netstat support
}
#endif /* NETDEV_USING_NETSTAT */

const struct netdev_ops a9g_netdev_ops =
{
    a9g_netdev_set_up,
    a9g_netdev_set_down,

    RT_NULL, /* not support set ip, netmask, gatway address */
    a9g_netdev_set_dns_server,
    RT_NULL, /* not support set DHCP status */

#ifdef NETDEV_USING_PING
    a9g_netdev_ping,
#endif
#ifdef NETDEV_USING_NETSTAT
    a9g_netdev_netstat,
#endif
};

static struct netdev *a9g_netdev_add(const char *netdev_name)
{
#define a9g_NETDEV_MTU       1500
    struct netdev *netdev = RT_NULL;

    RT_ASSERT(netdev_name);

    netdev = (struct netdev *) rt_calloc(1, sizeof(struct netdev));
    if (netdev == RT_NULL)
    {
        LOG_E("no memory for a9g device(%s) netdev structure.", netdev_name);
        return RT_NULL;
    }

    netdev->mtu = a9g_NETDEV_MTU;
    netdev->ops = &a9g_netdev_ops;

#ifdef SAL_USING_AT
    extern int sal_at_netdev_set_pf_info(struct netdev *netdev);
    /* set the network interface socket/netdb operations */
    sal_at_netdev_set_pf_info(netdev);
#endif

    netdev_register(netdev, netdev_name, RT_NULL);

    return netdev;
}

/* =============================  a9g device operations ============================= */

#define AT_SEND_CMD(client, resp, resp_line, timeout, cmd)                                         \
    do {                                                                                           \
        (resp) = at_resp_set_info((resp), 128, (resp_line), rt_tick_from_millisecond(timeout));    \
        if (at_obj_exec_cmd((client), (resp), (cmd)) < 0)                                          \
        {                                                                                          \
            result = -RT_ERROR;                                                                    \
            goto __exit;                                                                           \
        }                                                                                          \
    } while(0)                                                                                     \

/* init for a9g */
/* AT_SEND_CMD 是个宏定义函数 执行失败后会直接跳到 __exit 所以不能循环调用 */
static void a9g_init_thread_entry(void *parameter)
{
#define INIT_RETRY                     5
#define CPIN_RETRY                     10
#define CSQ_RETRY                      10
#define CREG_RETRY                     10
#define CGREG_RETRY                    20

    int i, qimux, retry_num = INIT_RETRY;
    char parsed_data[10] = {0};
    rt_err_t result = RT_EOK;
    at_response_t resp = RT_NULL;
    struct at_device *device = (struct at_device *)parameter;
    struct at_client *client = device->client;

    resp = at_create_resp(128, 0, rt_tick_from_millisecond(300));
    if (resp == RT_NULL)
    {
        LOG_E("no memory for a9g device(%s) response structure.", device->name);
        return;
    }

    LOG_D("start initializing the a9g device(%s)", device->name);

    while (retry_num--)
    {
        rt_memset(parsed_data, 0, sizeof(parsed_data));
        rt_thread_mdelay(500);
        a9g_power_on(device);
        rt_thread_mdelay(1000);

        /* wait a9g startup finish */
        if (at_client_obj_wait_connect(client, a9g_WAIT_CONNECT_TIME))
        {
            result = -RT_ETIMEOUT;
            goto __exit;
        }

        /* disable echo */
        AT_SEND_CMD(client, resp, 0, 300, "ATE0");
        /* get module version */
        #if BSP_USING_A9G
        AT_SEND_CMD(client, resp, 5, 300, "ATI");
        #endif
        /* show module version */
        for (i = 0; i < (int)resp->line_counts - 1; i++)
        {
            LOG_D("%s", at_resp_get_line(resp, i + 1));
        }
        /* check SIM card */
        for (i = 0; i < CPIN_RETRY; i++)
        {
            #if BSP_USING_A9G
            AT_SEND_CMD(client, resp, 2, 10 * 1000, "AT+CCID");
            if (at_resp_get_line_by_kw(resp, "+CCID:"))
            #endif
            {
                LOG_D("a9g device(%s) SIM card detection success.", device->name);
                break;
            }
            rt_thread_mdelay(1000);
        }
        if (i == CPIN_RETRY)
        {
            LOG_E("a9g device(%s) SIM card detection failed.", device->name);
            result = -RT_ERROR;
            goto __exit;
        }
        /* waiting for dirty data to be digested */
        rt_thread_mdelay(10);

        /* check the GSM network is registered */
        for (i = 0; i < CREG_RETRY; i++)
        {
            AT_SEND_CMD(client, resp, 0, 300, "AT+CREG?");
            at_resp_parse_line_args_by_kw(resp, "+CREG:", "+CREG: %s", &parsed_data);
            #if BSP_USING_A9G
            if (!strncmp(parsed_data, "1,1", sizeof(parsed_data)) ||
                !strncmp(parsed_data, "1,5", sizeof(parsed_data)))
            #endif
            {
                LOG_D("a9g device(%s) GSM network is registered(%s),", device->name, parsed_data);
                break;
            }
            rt_thread_mdelay(1000 + 500 * (i+1));
        }
        if (i == CREG_RETRY)
        {
            LOG_E("a9g device(%s) GSM network is register failed(%s).", device->name, parsed_data);
            result = -RT_ERROR;
            goto __exit;
        }

        #if BSP_USING_A9G
        /* the device default response timeout is 40 seconds, but it set to 15 seconds is convenient to use. */
        for (uint8_t ii = 0; ii < INIT_RETRY; ii++)
        {
            //AT_SEND_CMD(client, resp, 0, 5 * 1000, "AT+CGATT=0");
            resp = at_resp_set_info(resp, 128, 0, rt_tick_from_millisecond(10 * 1000));
            if (at_obj_exec_cmd(client, resp, "AT+CGATT=0") == RT_EOK)
            {
                break;
            }
            rt_thread_mdelay(1000);
        }
        #endif
        
        /* check the GPRS network is registered */
        for (i = 0; i < CGREG_RETRY; i++)
        {
            #if BSP_USING_A9G
            //CGATT ~ CGDCONT ~ CGACT
            for (uint8_t ii = 0; ii < INIT_RETRY; ii++)
            {
                //AT_SEND_CMD(client, resp, 0, 5 * 1000, "AT+CGATT=1");
                resp = at_resp_set_info(resp, 128, 0, rt_tick_from_millisecond(10 * 1000));
                if (at_obj_exec_cmd(client, resp, "AT+CGATT=1") == RT_EOK)
                {
                    break;
                }
                rt_thread_mdelay(1000);
            }
            AT_SEND_CMD(client, resp, 0, 1000, "AT+CGDCONT=1,\"IP\",\"CMNET\"");
            AT_SEND_CMD(client, resp, 0, 5 * 1000, "AT+CGACT=1,1");
            at_resp_parse_line_args_by_kw(resp, "OK", "%s", &parsed_data);
            if (!strncmp(parsed_data, "OK", sizeof(parsed_data)))
            {
                LOG_D("a9g device(%s) GPRS network is registered(%s).", device->name, parsed_data);
                break;
            }
            #endif
            rt_thread_mdelay(1000);
        }
        if (i == CGREG_RETRY)
        {
            LOG_E("a9g device(%s) GPRS network is register failed(%s).", device->name, parsed_data);
            result = -RT_ERROR;
            goto __exit;
        }

        /* check signal strength */
        for (i = 0; i < CSQ_RETRY; i++)
        {
            AT_SEND_CMD(client, resp, 0, 300, "AT+CSQ");
            at_resp_parse_line_args_by_kw(resp, "+CSQ:", "+CSQ: %s", &parsed_data);
            if (strncmp(parsed_data, "99,99", sizeof(parsed_data)))
            {
                LOG_D("a9g device(%s) signal strength: %s", device->name, parsed_data);
                break;
            }
            rt_thread_mdelay(1000);
        }
        if (i == CSQ_RETRY)
        {
            LOG_E("a9g device(%s) signal strength check failed (%s)", device->name, parsed_data);
            result = -RT_ERROR;
            goto __exit;
        }

        #if USED_SIM800C
        /* the device default response timeout is 40 seconds, but it set to 15 seconds is convenient to use. */
        AT_SEND_CMD(client, resp, 2, 20 * 1000, "AT+CIPSHUT");
        #endif
        #if BSP_USING_A9G
        //
        #endif

        /* Set to multiple connections */
        AT_SEND_CMD(client, resp, 0, 300, "AT+CIPMUX?");

        #if BSP_USING_A9G
        at_resp_parse_line_args_by_kw(resp, "+CIPMUX:", "+CIPMUX:%d", &qimux);
        #endif
        if (qimux == 0)
        {
            //跑程序的时候这个地方总会超时一次
            //猜测可能是因为没有替代"AT+CIPSHUT"的操作导致的
            AT_SEND_CMD(client, resp, 0, 1 * 1000, "AT+CIPMUX=1");
        }

        AT_SEND_CMD(client, resp, 0, 300, "AT+COPS?");
        at_resp_parse_line_args_by_kw(resp, "+COPS:", "+COPS: %*[^\"]\"%[^\"]", &parsed_data);
        if (rt_strcmp(parsed_data, "CHINA MOBILE") == 0)
        {
            /* "CMCC" */
            LOG_I("a9g device(%s) network operator: %s", device->name, parsed_data);
            AT_SEND_CMD(client, resp, 0, 300, CSTT_CHINA_MOBILE);
        }
        else if (rt_strcmp(parsed_data, "CHN-UNICOM") == 0)
        {
            /* "UNICOM" */
            LOG_I("a9g device(%s) network operator: %s", device->name, parsed_data);
            AT_SEND_CMD(client, resp, 0, 300, CSTT_CHINA_UNICOM);
        }
        else if (rt_strcmp(parsed_data, "CHN-CT") == 0)
        {
            AT_SEND_CMD(client, resp, 0, 300, CSTT_CHINA_TELECOM);
            /* "CT" */
            LOG_I("a9g device(%s) network operator: %s", device->name, parsed_data);
        }

        #if USED_SIM800C
        /* the device default response timeout is 150 seconds, but it set to 20 seconds is convenient to use. */
        AT_SEND_CMD(client, resp, 0, 20 * 1000, "AT+CIICR");
        #endif
        #if BSP_USING_A9G
        //
        #endif

        AT_SEND_CMD(client, resp, 2, 300, "AT+CIFSR");
        if (at_resp_get_line_by_kw(resp, "ERROR") != RT_NULL)
        {
            LOG_E("a9g device(%s) get the local address failed.", device->name);
            result = -RT_ERROR;
            goto __exit;
        }
        result = RT_EOK;

    __exit:
        if (result == RT_EOK)
        {
            break;
        }
        else
        {
            /* power off the a9g device */
            a9g_power_off(device);
            rt_thread_mdelay(1000);

            LOG_I("a9g device(%s) initialize retry...", device->name);
        }
    }

    if (resp)
    {
        at_delete_resp(resp);
    }

    if (result == RT_EOK)
    {
        device->is_init = RT_TRUE;//<-- add by moneng
        
        /* set network interface device status and address information */
        a9g_netdev_set_info(device->netdev);
        /*  */
        a9g_netdev_check_link_status(device->netdev);
        /*  */
        LOG_I("a9g device(%s) network initialize success!", device->name);

    }
    else
    {
        device->is_init = RT_FALSE;//<-- add by moneng
        
        netdev_low_level_set_status(device->netdev, RT_FALSE);//<-- add by moneng
        LOG_E("a9g device(%s) network initialize failed(%d)!", device->name, result);
    }
}

static int a9g_net_init(struct at_device *device)
{
#ifdef AT_DEVICE_a9g_INIT_ASYN
    rt_thread_t tid;

    tid = rt_thread_create("a9g_net_init", a9g_init_thread_entry, (void *)device,
                a9g_THREAD_STACK_SIZE, a9g_THREAD_PRIORITY, 20);
    if (tid)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("create a9g device(%s) initialization thread failed.", device->name);
        return -RT_ERROR;
    }
#else
    a9g_init_thread_entry(device);
#endif /* AT_DEVICE_a9g_INIT_ASYN */

    return RT_EOK;
}

static void urc_func(struct at_client *client, const char *data, rt_size_t size)
{
    RT_ASSERT(data);

    LOG_I("URC data : %.*s", size, data);
}

/* a9g device URC table for the device control */
static const struct at_urc urc_table[] = 
{
        {"RDY",         "\r\n",                 urc_func},
};

static int a9g_init(struct at_device *device)
{
    struct at_device_a9g *a9g = (struct at_device_a9g *) device->user_data;

    /* initialize AT client */
    at_client_init(a9g->client_name, a9g->recv_line_num);

    device->client = at_client_get(a9g->client_name);
    if (device->client == RT_NULL)
    {
        LOG_E("a9g device(%s) initialize failed, get AT client(%s) failed.", a9g->device_name, a9g->client_name);
        return -RT_ERROR;
    }

    /* register URC data execution function  */
    at_obj_set_urc_table(device->client, urc_table, sizeof(urc_table) / sizeof(urc_table[0]));

#ifdef AT_USING_SOCKET
    a9g_socket_init(device);
#endif

    /* add a9g device to the netdev list */
    device->netdev = a9g_netdev_add(a9g->device_name);
    if (device->netdev == RT_NULL)
    {
        LOG_E("a9g device(%s) initialize failed, get network interface device failed.", a9g->device_name);
        return -RT_ERROR;
    }

    /* initialize a9g pin configuration */
    if (a9g->power_pin != -1 && a9g->power_status_pin != -1)
    {
        rt_pin_mode(a9g->power_pin, PIN_MODE_OUTPUT);
        rt_pin_mode(a9g->power_status_pin, PIN_MODE_INPUT);
    }

    /* initialize a9g device network */
    return a9g_netdev_set_up(device->netdev);
}

static int a9g_deinit(struct at_device *device)
{
    return a9g_netdev_set_down(device->netdev);
}

static int a9g_control(struct at_device *device, int cmd, void *arg)
{
    int result = -RT_ERROR;

    RT_ASSERT(device);

    switch (cmd)
    {
    case AT_DEVICE_CTRL_POWER_ON:
    case AT_DEVICE_CTRL_POWER_OFF:
    case AT_DEVICE_CTRL_RESET:
    case AT_DEVICE_CTRL_LOW_POWER:
    case AT_DEVICE_CTRL_SLEEP:
    case AT_DEVICE_CTRL_WAKEUP:
    case AT_DEVICE_CTRL_NET_CONN:
    case AT_DEVICE_CTRL_NET_DISCONN:
    case AT_DEVICE_CTRL_SET_WIFI_INFO:
    case AT_DEVICE_CTRL_GET_SIGNAL:
    case AT_DEVICE_CTRL_GET_GPS:
    case AT_DEVICE_CTRL_GET_VER:
        LOG_W("a9g not support the control command(%d).", cmd);
        break;
    default:
        LOG_E("input error control command(%d).", cmd);
        break;
    }

    return result;
}

const struct at_device_ops a9g_device_ops = 
{
    a9g_init,
    a9g_deinit,
    a9g_control,
};

static int a9g_device_class_register(void)
{
    struct at_device_class *class = RT_NULL;

    class = (struct at_device_class *) rt_calloc(1, sizeof(struct at_device_class));
    if (class == RT_NULL)
    {
        LOG_E("no memory for a9g device class create.");
        return -RT_ENOMEM;
    }

    /* fill a9g device class object */
#ifdef AT_USING_SOCKET
    a9g_socket_class_register(class);
#endif
    class->device_ops = &a9g_device_ops;

    return at_device_class_register(class, AT_DEVICE_CLASS_a9g);
}
INIT_DEVICE_EXPORT(a9g_device_class_register);

#endif /* AT_DEVICE_USING_a9g */
