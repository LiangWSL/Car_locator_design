#ifndef __GPS_H
#define __GPS_H	 
#include <board.h>
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//ATK-NEO-6M GPSģ����������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/10/26
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved							  
////////////////////////////////////////////////////////////////////////////////// 	   

//GPS NMEA-0183Э����Ҫ�����ṹ�嶨�� 
//������Ϣ
__packed typedef struct  
{
 	rt_uint8_t num;		//���Ǳ��
	rt_uint8_t eledeg;	//��������
	rt_uint16_t azideg;	//���Ƿ�λ��
	rt_uint8_t sn;		//�����		   
}nmea_slmsg;
//UTCʱ����Ϣ
__packed typedef struct  
{
 	rt_uint16_t year;	//���
	rt_uint8_t month;	//�·�
	rt_uint8_t date;	//����
	rt_uint8_t hour; 	//Сʱ
	rt_uint8_t min; 	//����
	rt_uint8_t sec; 	//����
}nmea_utc_time;
//NMEA 0183 Э����������ݴ�Žṹ��
__packed typedef struct  
{
 	rt_uint8_t svnum;					//�ɼ�������
	nmea_slmsg slmsg[12];		//���12������
	nmea_utc_time utc;			//UTCʱ��
	rt_uint32_t latitude;				//γ�� ������100000��,ʵ��Ҫ����100000
	rt_uint8_t nshemi;					//��γ/��γ,N:��γ;S:��γ				  
	rt_uint32_t longitude;			    //���� ������100000��,ʵ��Ҫ����100000
	rt_uint8_t ewhemi;					//����/����,E:����;W:����
	rt_uint8_t gpssta;					//GPS״̬:0,δ��λ;1,�ǲ�ֶ�λ;2,��ֶ�λ;6,���ڹ���.				  
 	rt_uint8_t posslnum;				//���ڶ�λ��������,0~12.
 	rt_uint8_t possl[12];				//���ڶ�λ�����Ǳ��
	rt_uint8_t fixmode;					//��λ����:1,û�ж�λ;2,2D��λ;3,3D��λ
	rt_uint16_t pdop;					//λ�þ������� 0~500,��Ӧʵ��ֵ0~50.0
	rt_uint16_t hdop;					//ˮƽ�������� 0~500,��Ӧʵ��ֵ0~50.0
	rt_uint16_t vdop;					//��ֱ�������� 0~500,��Ӧʵ��ֵ0~50.0 

	int altitude;			 	//���θ߶�,�Ŵ���10��,ʵ�ʳ���10.��λ:0.1m	 
	rt_uint16_t speed;					//��������,�Ŵ���1000��,ʵ�ʳ���10.��λ:0.001����/Сʱ	 
}nmea_msg;

static nmea_msg gps_data;

int NMEA_Str2num(rt_uint8_t *buf,rt_uint8_t*dx);
void GPS_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
void NMEA_GPGGA_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
void NMEA_GPRMC_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
void NMEA_GPVTG_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);

#endif  

 


