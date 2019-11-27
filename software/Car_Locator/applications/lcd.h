#ifndef __LCD_H
#define __LCD_H		

#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "stdlib.h" 


//LCD��Ҫ������
typedef struct
{
	rt_uint16_t width;			//LCD ���
	rt_uint16_t height;			//LCD �߶�
	rt_uint16_t id;				//LCD ID
	rt_uint8_t  dir;			//���������������ƣ�0��������1��������	
	rt_uint16_t	wramcmd;		//��ʼдgramָ��
	rt_uint16_t  setxcmd;		//����x����ָ��
	rt_uint16_t  setycmd;		//����y����ָ�� 
}_lcd_dev; 	  

//LCD����
extern _lcd_dev lcddev;	//����LCD��Ҫ����
//LCD�Ļ�����ɫ�ͱ���ɫ	   
extern rt_uint16_t  POINT_COLOR;//Ĭ�Ϻ�ɫ    
extern rt_uint16_t  BACK_COLOR; //������ɫ.Ĭ��Ϊ��ɫ


//////////////////////////////////////////////////////////////////////////////////	 
//-----------------LCD�˿ڶ���---------------- 
#define LCD_LED GET_PIN(B, 15)  		//LCD����    		 PB15


//LCD��ַ�ṹ��
typedef struct
{
	rt_uint16_t LCD_REG;
	rt_uint16_t LCD_RAM;
} LCD_TypeDef;
//ʹ��NOR/SRAM�� Bank1.sector4,��ַλHADDR[27,26]=11 A6��Ϊ�������������� 
//ע������ʱSTM32�ڲ�������һλ����! 111 1110=0X7E			    
#define LCD_BASE        ((rt_uint32_t)(0x6C000000 | 0x0000007E))
#define LCD             ((LCD_TypeDef *) LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////
	 
//ɨ�跽����
#define L2R_U2D  0 //������,���ϵ���
#define L2R_D2U  1 //������,���µ���
#define R2L_U2D  2 //���ҵ���,���ϵ���
#define R2L_D2U  3 //���ҵ���,���µ���

#define U2D_L2R  4 //���ϵ���,������
#define U2D_R2L  5 //���ϵ���,���ҵ���
#define D2U_L2R  6 //���µ���,������
#define D2U_R2L  7 //���µ���,���ҵ���	 

#define DFT_SCAN_DIR  U2D_R2L  //Ĭ�ϵ�ɨ�跽��

//������ɫ
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	   0x001F  
#define BRED             0XF81F
#define GRED 			       0XFFE0
#define GBLUE			       0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			     0XBC40 //��ɫ
#define BRRED 			     0XFC07 //�غ�ɫ
#define GRAY  			     0X8430 //��ɫ
//GUI��ɫ

#define DARKBLUE      	 0X01CF	//����ɫ
#define LIGHTBLUE      	 0X7D7C	//ǳ��ɫ  
#define GRAYBLUE       	 0X5458 //����ɫ
//������ɫΪPANEL����ɫ 
 
#define LIGHTGREEN     	 0X841F //ǳ��ɫ
//#define LIGHTGRAY        0XEF5B //ǳ��ɫ(PANNEL)
#define LGRAY 			     0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ

#define LGRAYBLUE        0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE           0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)
	    															  
void LCD_Init(void);													   	//��ʼ��
void LCD_DisplayOn(void);													//����ʾ
void LCD_DisplayOff(void);													//����ʾ
void LCD_Clear(rt_uint16_t Color);	 												//����
void LCD_SetCursor(rt_uint16_t Xpos, rt_uint16_t Ypos);										//���ù��
void LCD_DrawPoint(rt_uint16_t x,rt_uint16_t y);											//����
void LCD_Fast_DrawPoint(rt_uint16_t x,rt_uint16_t y,rt_uint16_t color);								//���ٻ���
rt_uint16_t  LCD_ReadPoint(rt_uint16_t x,rt_uint16_t y); 											//���� 
void LCD_Draw_Circle(rt_uint16_t x0,rt_uint16_t y0,rt_uint8_t r);						 			//��Բ
void LCD_DrawLine(rt_uint16_t x1, rt_uint16_t y1, rt_uint16_t x2, rt_uint16_t y2);							//����
void LCD_DrawRectangle(rt_uint16_t x1, rt_uint16_t y1, rt_uint16_t x2, rt_uint16_t y2);		   				//������
void LCD_Fill(rt_uint16_t sx,rt_uint16_t sy,rt_uint16_t ex,rt_uint16_t ey,rt_uint16_t color);		   				//��䵥ɫ
void LCD_Color_Fill(rt_uint16_t sx,rt_uint16_t sy,rt_uint16_t ex,rt_uint16_t ey,rt_uint16_t *color);				//���ָ����ɫ
void LCD_ShowChar(rt_uint16_t x,rt_uint16_t y,rt_uint8_t num,rt_uint8_t size,rt_uint8_t mode);						//��ʾһ���ַ�
void LCD_ShowNum(rt_uint16_t x,rt_uint16_t y,rt_uint32_t num,rt_uint8_t len,rt_uint8_t size);  						//��ʾһ������
void LCD_ShowxNum(rt_uint16_t x,rt_uint16_t y,rt_uint32_t num,rt_uint8_t len,rt_uint8_t size,rt_uint8_t mode);				//��ʾ ����
void LCD_ShowString(rt_uint16_t x,rt_uint16_t y,rt_uint16_t width,rt_uint16_t height,rt_uint8_t size,rt_uint8_t *p);		//��ʾһ���ַ���,12/16����

void LCD_WriteReg(rt_uint16_t LCD_Reg, rt_uint16_t LCD_RegValue);
rt_uint16_t LCD_ReadReg(rt_uint16_t LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(rt_uint16_t RGB_Code);
void LCD_SSD_BackLightSet(rt_uint8_t pwm);							//SSD1963 �������
void LCD_Scan_Dir(rt_uint8_t dir);									//������ɨ�跽��
void LCD_Display_Dir(rt_uint8_t dir);								//������Ļ��ʾ����
void LCD_Set_Window(rt_uint16_t sx,rt_uint16_t sy,rt_uint16_t width,rt_uint16_t height);	//���ô���					   						   																			 
//LCD�ֱ�������
#define SSD_HOR_RESOLUTION		800		//LCDˮƽ�ֱ���
#define SSD_VER_RESOLUTION		480		//LCD��ֱ�ֱ���
//LCD������������
#define SSD_HOR_PULSE_WIDTH		1		//ˮƽ����
#define SSD_HOR_BACK_PORCH		46		//ˮƽǰ��
#define SSD_HOR_FRONT_PORCH		210		//ˮƽ����

#define SSD_VER_PULSE_WIDTH		1		//��ֱ����
#define SSD_VER_BACK_PORCH		23		//��ֱǰ��
#define SSD_VER_FRONT_PORCH		22		//��ֱǰ��
//���¼����������Զ�����
#define SSD_HT	(SSD_HOR_RESOLUTION+SSD_HOR_BACK_PORCH+SSD_HOR_FRONT_PORCH)
#define SSD_HPS	(SSD_HOR_BACK_PORCH)
#define SSD_VT 	(SSD_VER_RESOLUTION+SSD_VER_BACK_PORCH+SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

#endif  
	 
	 



