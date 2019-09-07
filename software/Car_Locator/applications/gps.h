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
//////////////////////////////////////////////////////////////////////////////////////////////////// 	
//UBLOX NEO-6M ����(���,����,���ص�)�ṹ��
__packed typedef struct  
{										    
 	rt_uint16_t header;					//cfg header,�̶�Ϊ0X62B5(С��ģʽ)
	rt_uint16_t id;						//CFG CFG ID:0X0906 (С��ģʽ)
	rt_uint16_t dlength;				//���ݳ��� 12/13
	rt_uint32_t clearmask;				//�������������(1��Ч)
	rt_uint32_t savemask;				//�����򱣴�����
	rt_uint32_t loadmask;				//�������������
	rt_uint8_t  devicemask; 		  	//Ŀ������ѡ������	b0:BK RAM;b1:FLASH;b2,EEPROM;b4,SPI FLASH
	rt_uint8_t  cka;		 			//У��CK_A 							 	 
	rt_uint8_t  ckb;			 		//У��CK_B							 	 
}_ublox_cfg_cfg; 

//UBLOX NEO-6M ��Ϣ���ýṹ��
__packed typedef struct  
{										    
 	rt_uint16_t header;					//cfg header,�̶�Ϊ0X62B5(С��ģʽ)
	rt_uint16_t id;						//CFG MSG ID:0X0106 (С��ģʽ)
	rt_uint16_t dlength;				//���ݳ��� 8
	rt_uint8_t  msgclass;				//��Ϣ����(F0 ����NMEA��Ϣ��ʽ)
	rt_uint8_t  msgid;					//��Ϣ ID 
														//00,GPGGA;01,GPGLL;02,GPGSA;
														//03,GPGSV;04,GPRMC;05,GPVTG;
														//06,GPGRS;07,GPGST;08,GPZDA;
														//09,GPGBS;0A,GPDTM;0D,GPGNS;
	rt_uint8_t  iicset;					//IIC���������    0,�ر�;1,ʹ��.
	rt_uint8_t  uart1set;				//UART1�������	   0,�ر�;1,ʹ��.
	rt_uint8_t  uart2set;				//UART2�������	   0,�ر�;1,ʹ��.
	rt_uint8_t  usbset;					//USB�������	   0,�ر�;1,ʹ��.
	rt_uint8_t  spiset;					//SPI�������	   0,�ر�;1,ʹ��.
	rt_uint8_t  ncset;					//δ֪�������	   Ĭ��Ϊ1����.
 	rt_uint8_t  cka;			 		//У��CK_A 							 	 
	rt_uint8_t  ckb;			    	//У��CK_B							 	 
}_ublox_cfg_msg; 

//UBLOX NEO-6M UART�˿����ýṹ��
__packed typedef struct  
{										    
 	rt_uint16_t header;				//cfg header,�̶�Ϊ0X62B5(С��ģʽ)
	rt_uint16_t id;						//CFG PRT ID:0X0006 (С��ģʽ)
	rt_uint16_t dlength;			//���ݳ��� 20
	rt_uint8_t  portid;				//�˿ں�,0=IIC;1=UART1;2=UART2;3=USB;4=SPI;
	rt_uint8_t  reserved;			//����,����Ϊ0
	rt_uint16_t txready;			//TX Ready��������,Ĭ��Ϊ0
	rt_uint32_t mode;					//���ڹ���ģʽ����,��żУ��,ֹͣλ,�ֽڳ��ȵȵ�����.
 	rt_uint32_t baudrate;			//����������
 	rt_uint16_t inprotomask;	//����Э�鼤������λ  Ĭ������Ϊ0X07 0X00����.
 	rt_uint16_t outprotomask;	//���Э�鼤������λ  Ĭ������Ϊ0X07 0X00����.
 	rt_uint16_t reserved4; 		//����,����Ϊ0
 	rt_uint16_t reserved5; 		//����,����Ϊ0 
 	rt_uint8_t  cka;			 		//У��CK_A 							 	 
	rt_uint8_t  ckb;			    //У��CK_B							 	 
}_ublox_cfg_prt; 

//UBLOX NEO-6M ʱ���������ýṹ��
__packed typedef struct  
{										    
 	rt_uint16_t header;				//cfg header,�̶�Ϊ0X62B5(С��ģʽ)
	rt_uint16_t id;						//CFG TP ID:0X0706 (С��ģʽ)
	rt_uint16_t dlength;			//���ݳ���
	rt_uint32_t interval;			//ʱ��������,��λΪus
	rt_uint32_t length;				//������,��λΪus
	signed char status;				//ʱ����������:1,�ߵ�ƽ��Ч;0,�ر�;-1,�͵�ƽ��Ч.			  
	rt_uint8_t timeref;			  //�ο�ʱ��:0,UTCʱ��;1,GPSʱ��;2,����ʱ��.
	rt_uint8_t flags;					//ʱ���������ñ�־
	rt_uint8_t reserved;			//����			  
 	signed short antdelay;	 	//������ʱ
 	signed short rfdelay;			//RF��ʱ
	signed int userdelay; 	 	//�û���ʱ	
	rt_uint8_t cka;						//У��CK_A 							 	 
	rt_uint8_t ckb;						//У��CK_B							 	 
}_ublox_cfg_tp; 

//UBLOX NEO-6M ˢ���������ýṹ��
__packed typedef struct  
{
 	rt_uint16_t header;				//cfg header,�̶�Ϊ0X62B5(С��ģʽ)
	rt_uint16_t id;						//CFG RATE ID:0X0806 (С��ģʽ)
	rt_uint16_t dlength;			//���ݳ���
	rt_uint16_t measrate;			//����ʱ��������λΪms�����ٲ���С��200ms��5Hz��
	rt_uint16_t navrate;			//�������ʣ����ڣ����̶�Ϊ1
	rt_uint16_t timeref;			//�ο�ʱ�䣺0=UTC Time��1=GPS Time��
 	rt_uint8_t  cka;					//У��CK_A 							 	 
	rt_uint8_t  ckb;					//У��CK_B							 	 
}_ublox_cfg_rate;
				 
int NMEA_Str2num(rt_uint8_t *buf,rt_uint8_t*dx);
void GPS_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
void NMEA_GPGGA_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
void NMEA_GPRMC_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
void NMEA_GPVTG_Analysis(nmea_msg *gpsx,rt_uint8_t *buf);
rt_uint8_t Ublox_Cfg_Cfg_Save(void);
rt_uint8_t Ublox_Cfg_Msg(rt_uint8_t msgid,rt_uint8_t uart1set);
rt_uint8_t Ublox_Cfg_Prt(rt_uint32_t baudrate);
rt_uint8_t Ublox_Cfg_Tp(rt_uint32_t interval,rt_uint32_t length,signed char status);
rt_uint8_t Ublox_Cfg_Rate(rt_uint16_t measrate,rt_uint8_t reftime);
void Ublox_Send_Date(rt_uint8_t* dbuf,rt_uint16_t len);

#endif  

 


