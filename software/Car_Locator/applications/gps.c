#include "gps.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lcd.h"

#ifdef AT_USING_A9G_GPS

#define LOG_TAG                        "gps"
#include <at_log.h>

nmea_msg gps_data;

//��buf����õ���cx���������ڵ�λ��
//����ֵ:0~0XFE,����������λ�õ�ƫ��.
//       0XFF,�������ڵ�cx������
rt_uint8_t NMEA_Comma_Pos(rt_uint8_t *buf,rt_uint8_t cx)
{
    rt_uint8_t *p=buf;
    while(cx)
    {
        if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//����'*'���߷Ƿ��ַ�,�򲻴��ڵ�cx������
        if(*buf==',')cx--;
        buf++;
    }
    return buf-p;
}
//m^n����
//����ֵ:m^n�η�.
rt_uint32_t NMEA_Pow(rt_uint8_t m,rt_uint8_t n)
{
    rt_uint32_t result=1;
    while(n--)result*=m;
    return result;
}
//strת��Ϊ����,��','����'*'����
//buf:���ִ洢��
//dx:С����λ��,���ظ����ú���
//����ֵ:ת�������ֵ
int NMEA_Str2num(rt_uint8_t *buf,rt_uint8_t*dx)
{
    rt_uint8_t *p=buf;
    rt_uint32_t ires=0,fres=0;
    rt_uint8_t ilen=0,flen=0,i;
    rt_uint8_t mask=0;
    int res;
    while(1) //�õ�������С���ĳ���
    {
        if(*p=='-') {
            mask|=0X02;    //�Ǹ���
            p++;
        }
        if(*p==','||(*p=='*'))break;//����������
        if(*p=='.') {
            mask|=0X01;    //����С������
            p++;
        }
        else if(*p>'9'||(*p<'0'))	//�зǷ��ַ�
        {
            ilen=0;
            flen=0;
            break;
        }
        if(mask&0X01)flen++;
        else ilen++;
        p++;
    }
    if(mask&0X02)buf++;	//ȥ������
    for(i=0; i<ilen; i++)	//�õ�������������
    {
        ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
    }
    if(flen>5)flen=5;	//���ȡ5λС��
    *dx=flen;	 		//С����λ��
    for(i=0; i<flen; i++)	//�õ�С����������
    {
        fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
    }
    res=ires*NMEA_Pow(10,flen)+fres;
    if(mask&0X02)res=-res;
    return res;
}
//����GPGSV��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,rt_uint8_t *buf)
{
    rt_uint8_t *p,*p1,dx;
    rt_uint8_t len,i,j,slx=0;
    rt_uint8_t posx;
    p=buf;
    p1=(rt_uint8_t*)strstr((const char *)p,"$GPGSV");
    len=p1[7]-'0';								//�õ�GPGSV������
    posx=NMEA_Comma_Pos(p1,3); 					//�õ��ɼ���������
    if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
    for(i=0; i<len; i++)
    {
        p1=(rt_uint8_t*)strstr((const char *)p,"$GPGSV");
        for(j=0; j<4; j++)
        {
            posx=NMEA_Comma_Pos(p1,4+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//�õ����Ǳ��
            else break;
            posx=NMEA_Comma_Pos(p1,5+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//�õ���������
            else break;
            posx=NMEA_Comma_Pos(p1,6+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//�õ����Ƿ�λ��
            else break;
            posx=NMEA_Comma_Pos(p1,7+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//�õ����������
            else break;
            slx++;
        }
        p=p1+1;//�л�����һ��GPGSV��Ϣ
    }
}
//����GNGGA��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GNGGA_Analysis(nmea_msg *gpsx,rt_uint8_t *buf)
{
		char data[50] = { 0 };
		rt_memset(data, 0, sizeof(data));
    rt_uint8_t *p1,dx;
    rt_uint8_t posx;
    p1=(rt_uint8_t*)strstr((const char *)buf,"$GNGGA");
		
    posx=NMEA_Comma_Pos(p1,6);								//�õ�GPS״̬
    if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,7);								//�õ����ڶ�λ��������
    if(posx!=0XFF)gpsx->posslnum=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,9);								//�õ����θ߶�
    if(posx!=0XFF)gpsx->altitude=NMEA_Str2num(p1+posx,&dx);
}
//����GPGSA��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,rt_uint8_t *buf)
{
    rt_uint8_t *p1,dx;
    rt_uint8_t posx;
    rt_uint8_t i;
    p1=(rt_uint8_t*)strstr((const char *)buf,"$GPGSA");
    posx=NMEA_Comma_Pos(p1,2);								//�õ���λ����
    if(posx!=0XFF)gpsx->fixmode=NMEA_Str2num(p1+posx,&dx);
    for(i=0; i<12; i++)												//�õ���λ���Ǳ��
    {
        posx=NMEA_Comma_Pos(p1,3+i);
        if(posx!=0XFF)gpsx->possl[i]=NMEA_Str2num(p1+posx,&dx);
        else break;
    }
    posx=NMEA_Comma_Pos(p1,15);								//�õ�PDOPλ�þ�������
    if(posx!=0XFF)gpsx->pdop=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,16);								//�õ�HDOPλ�þ�������
    if(posx!=0XFF)gpsx->hdop=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,17);								//�õ�VDOPλ�þ�������
    if(posx!=0XFF)gpsx->vdop=NMEA_Str2num(p1+posx,&dx);
}
//����GNRMC��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
static uint16_t count = 0;
void NMEA_GNRMC_Analysis(nmea_msg *gpsx,rt_uint8_t *buf)
{
		char data[50] = { 0 };
    rt_uint8_t *p1,dx;
    rt_uint8_t posx;
    rt_uint32_t temp;
    float rs;
		int gps_longitude_int;
		int gps_longitude_fen;
		int gps_longitude_miao;
		int gps_latitude_int;
		int gps_latitude_fen;
		int gps_latitude_miao;
		count++;
    p1=(rt_uint8_t*)strstr((const char *)buf,"GNRMC");//"$GNRMC",������&��GNRMC�ֿ������,��ֻ�ж�GNRMC.
    posx=NMEA_Comma_Pos(p1,1);								//�õ�UTCʱ��
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//�õ�UTCʱ��,ȥ��ms
        gpsx->utc.hour=temp/10000;
        gpsx->utc.min=(temp/100)%100;
        gpsx->utc.sec=temp%100;
    }
		posx=NMEA_Comma_Pos(p1,4);								//��γ���Ǳ�γ
    if(posx!=0XFF)gpsx->nshemi=*(p1+posx);
    posx=NMEA_Comma_Pos(p1,3);								//�õ�γ��
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx);
        gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//�õ���
        rs=temp%NMEA_Pow(10,dx+2);				//�õ�'
        gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ��
				rt_memset(data, 0, sizeof(data));
				gps_latitude_int = gpsx->latitude / 100000;
				gps_latitude_fen = (int)(((float)(gpsx->latitude / 100000.0)-gps_latitude_int) * 60);
				gps_latitude_miao = (((float)(gpsx->latitude / 100000.0)-gps_latitude_int) * 60 - gps_latitude_fen)*60;
				sprintf(data, "%2d:%2d`%2d\"  %1c\n", gps_latitude_int, gps_latitude_fen, gps_latitude_miao, gpsx->nshemi);
				LCD_ShowString(20+88, 120, 100, 16, 16, (rt_uint8_t *)data);
    }
    
		posx=NMEA_Comma_Pos(p1,6);								//������������
    if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);
    posx=NMEA_Comma_Pos(p1,5);								//�õ�����
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx);
        gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//�õ���
        rs=temp%NMEA_Pow(10,dx+2);				//�õ�'
        gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ��
				rt_memset(data, 0, sizeof(data));
				gps_longitude_int = gpsx->longitude / 100000;
				gps_longitude_fen = (int)(((float)(gpsx->longitude / 100000.0)-gps_longitude_int) * 60);
				gps_longitude_miao = (((float)(gpsx->longitude / 100000.0)-gps_longitude_int) * 60 - gps_longitude_fen)*60;
				sprintf(data, "%2d:%2d`%2d\"  %1c\n", gps_longitude_int, gps_longitude_fen, gps_longitude_miao, gpsx->ewhemi);
				LCD_ShowString(20+88, 80, 100, 16, 16, (rt_uint8_t *)data);
    }
    
    posx=NMEA_Comma_Pos(p1,9);								//�õ�UTC����
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx);		 				//�õ�UTC����
        gpsx->utc.date=temp/10000;
        gpsx->utc.month=(temp/100)%100;
        gpsx->utc.year=2000+temp%100;
    }
		if(count >= 200)
		{
				count = 0;
				rt_memset(data, 0, sizeof(data));
				sprintf(data, "%2d:%2d`%2d\"  %1c\n", gps_longitude_int, gps_longitude_fen, gps_longitude_miao, gpsx->ewhemi);
				LOG_I(data);
				rt_memset(data, 0, sizeof(data));
				sprintf(data, "%2d:%2d`%2d\"  %1c\n", gps_latitude_int, gps_latitude_fen, gps_latitude_miao, gpsx->nshemi);
				LOG_I(data);
				rt_memset(data, 0, sizeof(data));
				sprintf(data, "%.2f  km/h\n", (float)gpsx->speed / 1000);
				LOG_I(data);
		}
		
}
//����GNVTG��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GNVTG_Analysis(nmea_msg *gpsx,rt_uint8_t *buf)
{
		char data[50] = { 0 };
    rt_uint8_t *p1,dx;
    rt_uint8_t posx;
    p1=(rt_uint8_t*)strstr((const char *)buf,"$GNVTG");
    posx=NMEA_Comma_Pos(p1,7);								//�õ���������
    if(posx!=0XFF)
    {
        gpsx->speed=NMEA_Str2num(p1+posx,&dx);
        if(dx<3)gpsx->speed*=NMEA_Pow(10,3-dx);	 	 		//ȷ������1000��
				rt_memset(data, 0, sizeof(data));
				sprintf(data, "%.2f  km/h\n", (float)gpsx->speed / 1000);
//				rt_kprintf(data);
				LCD_ShowString(20+8*10, 160, 100, 16, 16, (rt_uint8_t *)data);
    }
}
//��ȡNMEA-0183��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void GPS_Analysis(nmea_msg *gpsx,rt_uint8_t *buf)
{
		if (strstr((const char *)buf, "$GPGSV"))
		{
				NMEA_GPGSV_Analysis(gpsx,buf);	//GPGSV����
		}
		if (strstr((const char *)buf, "$GNGGA"))
		{
				NMEA_GNGGA_Analysis(gpsx,buf);	//GNGGA����
		}
		if (strstr((const char *)buf, "$GPGSA"))
		{
				NMEA_GPGSA_Analysis(gpsx,buf);	//GPGSA����
		}
		if (strstr((const char *)buf, "GNRMC"))
		{
				NMEA_GNRMC_Analysis(gpsx,buf);	//GNRMC����
		}
    if (strstr((const char *)buf, "$GNVTG"))
		{
				NMEA_GNVTG_Analysis(gpsx,buf);	//GNVTG����
		}
}

/* ���ڽ����¼���־ */
#define UART_RX_EVENT (1 << 0)

/* �¼����ƿ� */
static struct rt_event event;
/* �����豸��� */
static rt_device_t uart_device = RT_NULL;

/* �ص����� */
static rt_err_t uart_intput(rt_device_t dev, rt_size_t size)
{
    /* �����¼� */
    rt_event_send(&event, UART_RX_EVENT);
    
    return RT_EOK;
}

/* uart device get char */
static rt_uint8_t uart_getchar(void)
{
    rt_uint32_t e;
    rt_uint8_t ch;
    
    /* ��ȡ1�ֽ����� */
    while (rt_device_read(uart_device, 0, &ch, 1) != 1)
    {
				/* �����¼� */
				rt_event_recv(&event, UART_RX_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &e);
    }

    return ch;
}


static void uart_putchar(const rt_uint8_t c)
{
    rt_size_t len = 0;
    rt_uint32_t timeout = 0;
    do
    {
        len = rt_device_write(uart_device, 0, &c, 1);
        timeout++;
    }
    while (len != 1 && timeout < 500);
}

static void uart_putstring(const rt_uint8_t *s)
{
    while(*s)
    {
        uart_putchar(*s++);
    }
}

/* open the uart device */
static rt_err_t uart_open(rt_device_t device)
{
    rt_err_t res;

    if (device != RT_NULL)
    {
        res = rt_device_set_rx_indicate(device, uart_intput);
        /* ��鷵��ֵ */
        if (res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n",device->parent.name,res);
            return -RT_ERROR;
        }

        /* ���豸���Կɶ�д���жϷ�ʽ */
        res = rt_device_open(device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );       
        /* ��鷵��ֵ */
        if (res != RT_EOK)
        {
            rt_kprintf("open %s device error.%d\n",device->parent.name,res);
            return -RT_ERROR;
        }
        
        /* ��ʼ���¼����� */
        rt_event_init(&event, "event", RT_IPC_FLAG_FIFO); 
        
        return RT_EOK;
    }
    else
    {
        rt_kprintf("can't open %s device.\n",device->parent.name);
        return -RT_ERROR;
    }
}

/* read device data */
static void data_read(uint8_t* buff, int len)
{
    rt_uint8_t uart_rx_byte;
    rt_uint8_t count = 0;
    
    do
		{
        uart_rx_byte = uart_getchar();
        buff[count] = uart_rx_byte;
        count ++;       
    }while((uart_rx_byte != '\n') && (count <= len));
    
    buff[count] = 0;
}

extern nmea_msg gps_data;
void gps_thread_entry(void *parameter)
{
    uint8_t line[GPS_DEVICE_RECV_BUFF_LEN];
		
    while(1)
    {
        data_read(line, GPS_DEVICE_RECV_BUFF_LEN);
//				rt_kprintf("%s", line);
				
				GPS_Analysis(&gps_data, line);
				rt_thread_mdelay(10);
    }
}

/* gps port initial */
rt_err_t gps_init(void)
{
    rt_thread_t tid;
    
    /* ����ϵͳ�еĴ����豸 */
    uart_device = rt_device_find(GPS_UART_NAME);
    
    if (RT_NULL == uart_device)
    {
        rt_kprintf(" find device %s failed!\n",GPS_UART_NAME);  
        return RT_EINVAL;
    }
    struct serial_configure gps_use_config = 
    {
        BAUD_RATE_9600,   /* 9600 bits/s */
        DATA_BITS_8,      /* 8 databits */
        STOP_BITS_1,      /* 1 stopbit */
        PARITY_NONE,      /* No parity  */ 
        BIT_ORDER_LSB,    /* LSB first sent */
        NRZ_NORMAL,       /* Normal mode */
        1024,             /* Buffer size */
        0   
    };

    if (RT_EOK != rt_device_control(uart_device, RT_DEVICE_CTRL_CONFIG,(void *)&gps_use_config))
    {
        rt_kprintf("uart config failed.\n");
        return RT_ERROR;
    }

    if (uart_open(uart_device) != RT_EOK)
    {
        rt_kprintf("uart open error.\n");
        return RT_ERROR;
    }
    if (RT_EOK != rt_device_control(uart_device, RT_DEVICE_CTRL_CONFIG,(void *)&gps_use_config))
    {
        rt_kprintf("uart config failed.\n");
        return RT_ERROR;
    }

    tid = rt_thread_create("gps_recive", 
                            gps_thread_entry,
                            RT_NULL,
                            2048,
                            8,
                            200);
    if (tid == RT_NULL)
    {
        return RT_ERROR;
    }
		
    rt_thread_startup(tid);

    return RT_EOK;
}
INIT_DEVICE_EXPORT(gps_init);
#endif /* AT_USING_A9G_GPS */

