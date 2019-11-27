/******************************************************************************
* @ File name --> rc522.c
* @ Author    --> By@ ³��
* @ Version   --> V1.0
* @ Date      --> 07 - 17 - 2019
* @ Brief     --> RFIDģ����������
*
* @ Copyright (C) 2019 -
* @ All rights reserved	www.cqutlab.club
*******************************************************************************
*
*                                  File Update
* @ Version   --> V1.0
* @ Author    --> By@³��
* @ Date      --> 07 - 17 - 2019
* @ Revise    --> �½��ļ�
*
******************************************************************************/
#include "rc522.h"
#include "stm32f4xx_hal_spi.h"
#include "string.h"
#include "lcd.h"

/***********************
*����˵����
*1--SS  <----->PA4
*2--SCK <----->PA5
*3--MOSI<----->PA7
*4--MISO<----->PA6
*5--����
*6--GND <----->GND
*7--RST <----->PC4
*8--VCC <----->VCC
***********************/


unsigned char CT[2];			//������
unsigned char SN[4]; 			//����
unsigned char ST[4];
char Card_flag=0;			//�ڶ�������⵽����־  0 ���ޣ�1����


void delay_ns(rt_uint32_t ns)
{
    rt_uint32_t i;
    for(i=0; i<ns; i++)
    {
        __nop();
        __nop();
        __nop();
    }
}

rt_uint8_t SPIWriteByte(rt_uint8_t Byte)
{
    while((SPI1->SR&0X02)==0);		//�ȴ���������
    SPI1->DR=Byte;	 	            //����һ��byte
    while((SPI1->SR&0X01)==0);      //�ȴ�������һ��byte
    return SPI1->DR;          	    //�����յ�������
}

SPI_HandleTypeDef hspi1;
/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

void SPI1_Init(void)
{
//    GPIO_InitTypeDef  GPIO_InitStructure;
//    SPI_InitTypeDef   SPI_InitStructure;

//    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOAʱ��
//    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//ʹ��GPIOCʱ��
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);//ʹ��SPI1ʱ��

//    //NSS
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
//    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
//    GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��
//    GPIO_SetBits(GPIOA,GPIO_Pin_4);

//    //RST
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
//    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
//    GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��
//		GPIO_SetBits(GPIOC,GPIO_Pin_4);

//    //MISO\MOSI  CLK
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
//    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
//    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
//    GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��
		
//    GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
//    GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1);
//    GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1);
//    GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);

//    //����ֻ���SPI�ڳ�ʼ��
//    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);//��λSPI1
//    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);//ֹͣ��λSPI1
		
//    RC522_CS = 1;
		rt_pin_mode(RC522_CS_PIN, PIN_MODE_OUTPUT);
		rt_pin_write(RC522_CS_PIN, PIN_HIGH);
//    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
//    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
//    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
//    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�͵�ƽ
//    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//����ͬ��ʱ�ӵĵ�һ�������أ��������½������ݱ�����
//    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
//    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ32
//    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
//    SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
//    SPI_Init(SPI1, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

//    SPI_Cmd(SPI1, ENABLE); //ʹ��SPI����
		MX_SPI1_Init();
		SPIWriteByte(0xff);//��������

}

void InitRc522(void)
{
    SPI1_Init();
    PcdReset();
    PcdAntennaOff();
    rt_thread_delay(2);
    PcdAntennaOn();
    M500PcdConfigISOType( 'A' );
}
//INIT_DEVICE_EXPORT(InitRc522);

void Reset_RC522(void)
{
    PcdReset();
    PcdAntennaOff();
    rt_thread_delay(2);
    PcdAntennaOn();
}
/////////////////////////////////////////////////////////////////////
//��    �ܣ�Ѱ��
//����˵��: req_code[IN]:Ѱ����ʽ
//                0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
//                0x26 = Ѱδ��������״̬�Ŀ�
//          pTagType[OUT]����Ƭ���ʹ���
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRequest(rt_uint8_t   req_code,rt_uint8_t *pTagType)
{
    char   status;
    rt_uint8_t   unLen;
    rt_uint8_t   ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x07);
    SetBitMask(TxControlReg,0x03);

    ucComMF522Buf[0] = req_code;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

    if ((status == MI_OK) && (unLen == 0x10))
    {
        *pTagType     = ucComMF522Buf[0];
        *(pTagType+1) = ucComMF522Buf[1];
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�����ײ
//����˵��: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
char PcdAnticoll(rt_uint8_t *pSnr)
{
    char   status;
    rt_uint8_t   i,snr_check=0;
    rt_uint8_t   unLen;
    rt_uint8_t   ucComMF522Buf[MAXRLEN];


    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
        for (i=0; i<4; i++) {
            *(pSnr+i)  = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i]) {
            status = MI_ERR;
        }
    }

    SetBitMask(CollReg,0x80);
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ѡ����Ƭ
//����˵��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
char PcdSelect(rt_uint8_t *pSnr)
{
    char   status;
    rt_uint8_t   i;
    rt_uint8_t   unLen;
    rt_uint8_t   ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
        ucComMF522Buf[i+2] = *(pSnr+i);
        ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);

    ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);

    if ((status == MI_OK) && (unLen == 0x18))
    {
        status = MI_OK;
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���֤��Ƭ����
//����˵��: auth_mode[IN]: ������֤ģʽ
//                 0x60 = ��֤A��Կ
//                 0x61 = ��֤B��Կ
//          addr[IN]�����ַ
//          pKey[IN]������
//          pSnr[IN]����Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
char PcdAuthState(rt_uint8_t   auth_mode,rt_uint8_t   addr,rt_uint8_t *pKey,rt_uint8_t *pSnr)
{
    char   status;
    rt_uint8_t   unLen;
    rt_uint8_t   ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    memcpy(&ucComMF522Buf[2], pKey, 6);
    memcpy(&ucComMF522Buf[8], pSnr, 4);

    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���ȡM1��һ������
//����˵��: addr[IN]�����ַ
//          p [OUT]�����������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRead(rt_uint8_t   addr,rt_uint8_t *p )
{
    char   status;
    rt_uint8_t   unLen;
    rt_uint8_t   i,ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
//   if ((status == MI_OK) && (unLen == 0x90))
//   {   memcpy(p , ucComMF522Buf, 16);   }
    {
        for (i=0; i<16; i++)
        {
            *(p +i) = ucComMF522Buf[i];
        }
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�д���ݵ�M1��һ��
//����˵��: addr[IN]�����ַ
//          p [IN]��д������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
char PcdWrite(rt_uint8_t   addr,rt_uint8_t *p )
{
    char   status;
    rt_uint8_t   unLen;
    rt_uint8_t   i,ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, p , 16);
        for (i=0; i<16; i++)
        {
            ucComMF522Buf[i] = *(p +i);
        }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ����Ƭ��������״̬
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
char PcdHalt(void)
{
    uint8_t   status;
    uint8_t   unLen;
    uint8_t   ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return status;
}

/////////////////////////////////////////////////////////////////////
//��MF522����CRC16����
/////////////////////////////////////////////////////////////////////
void CalulateCRC(rt_uint8_t *pIn ,rt_uint8_t   len,rt_uint8_t *pOut )
{
    rt_uint8_t   i,n;
    ClearBitMask(DivIrqReg,0x04);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {
        WriteRawRC(FIFODataReg, *(pIn +i));
    }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOut [0] = ReadRawRC(CRCResultRegL);
    pOut [1] = ReadRawRC(CRCResultRegM);
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���λRC522
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
char PcdReset(void)
{
//    RC522_CS=1;
		rt_pin_write(RC522_CS_PIN, PIN_HIGH);
    delay_ns(10);
		rt_pin_write(RC522_CS_PIN, PIN_LOW);
//    RC522_CS=0;
    delay_ns(10);
//    RC522_CS=1;
		rt_pin_write(RC522_CS_PIN, PIN_HIGH);
    delay_ns(10);
    WriteRawRC(CommandReg,PCD_RESETPHASE);
    WriteRawRC(CommandReg,PCD_RESETPHASE);
    delay_ns(10);

    WriteRawRC(ModeReg,0x3D);            //��Mifare��ͨѶ��CRC��ʼֵ0x6363
    WriteRawRC(TReloadRegL,30);
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);

    WriteRawRC(TxAutoReg,0x40);       //

    return MI_OK;
}
//////////////////////////////////////////////////////////////////////
//����RC632�Ĺ�����ʽ
//////////////////////////////////////////////////////////////////////
char M500PcdConfigISOType(rt_uint8_t   type)
{
    if (type == 'A')                     //ISO14443_A
    {
        ClearBitMask(Status2Reg,0x08);
        WriteRawRC(ModeReg,0x3D);//3F
        WriteRawRC(RxSelReg,0x86);//84
        WriteRawRC(RFCfgReg,0x7F);   //4F
        WriteRawRC(TReloadRegL,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec)
        WriteRawRC(TReloadRegH,0);
        WriteRawRC(TModeReg,0x8D);
        WriteRawRC(TPrescalerReg,0x3E);
        delay_ns(1000);
        PcdAntennaOn();
    }
    else {
        return 1;
    }

    return MI_OK;
}
/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//��    �أ�������ֵ
/////////////////////////////////////////////////////////////////////
rt_uint8_t ReadRawRC(rt_uint8_t   Address)
{
    rt_uint8_t   ucAddr;
    rt_uint8_t   ucResult=0;
//    RC522_CS=0;
		rt_pin_write(RC522_CS_PIN, PIN_LOW);
    ucAddr = ((Address<<1)&0x7E)|0x80;

    SPIWriteByte(ucAddr);
    ucResult=SPIReadByte();
//    RC522_CS=1;
		rt_pin_write(RC522_CS_PIN, PIN_HIGH);
    return ucResult;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�дRC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//          value[IN]:д���ֵ
/////////////////////////////////////////////////////////////////////
void WriteRawRC(rt_uint8_t   Address, rt_uint8_t   value)
{
    rt_uint8_t   ucAddr;

//    RC522_CS=0;
		rt_pin_write(RC522_CS_PIN, PIN_LOW);
    ucAddr = ((Address<<1)&0x7E);

    SPIWriteByte(ucAddr);
    SPIWriteByte(value);
//    RC522_CS=1;
		rt_pin_write(RC522_CS_PIN, PIN_HIGH);
}
/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
void SetBitMask(rt_uint8_t   reg,rt_uint8_t   mask)
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
void ClearBitMask(rt_uint8_t   reg,rt_uint8_t   mask)
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ͨ��RC522��ISO14443��ͨѶ
//����˵����Command[IN]:RC522������
//          pIn [IN]:ͨ��RC522���͵���Ƭ������
//          InLenByte[IN]:�������ݵ��ֽڳ���
//          pOut [OUT]:���յ��Ŀ�Ƭ��������
//          *pOutLenBit[OUT]:�������ݵ�λ����
/////////////////////////////////////////////////////////////////////
 char PcdComMF522(rt_uint8_t   Command,
                 rt_uint8_t *pIn ,
                 rt_uint8_t   InLenByte,
                 rt_uint8_t *pOut ,
                 rt_uint8_t *pOutLenBit)
{
    char   status = MI_ERR;
    rt_uint8_t   irqEn   = 0x00;
    rt_uint8_t   waitFor = 0x00;
    rt_uint8_t   lastBits;
    rt_uint8_t   n;
    long   i;
    switch (Command) {
    case PCD_AUTHENT:
        irqEn   = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE:
        irqEn   = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    WriteRawRC(ComIEnReg,irqEn|0x80);
    ClearBitMask(ComIrqReg,0x80);	//�������ж�λ
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);	 	//��FIFO����

    for (i=0; i<InLenByte; i++) {
        WriteRawRC(FIFODataReg, pIn [i]);
    }
    WriteRawRC(CommandReg, Command);

    if (Command == PCD_TRANSCEIVE) {
        SetBitMask(BitFramingReg,0x80);
    }	 //��ʼ����

    //i = 600;//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
    i = 100000;
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    } while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);

    if (i!=0) {
        if(!(ReadRawRC(ErrorReg)&0x1B)) {
            status = MI_OK;
            if (n & irqEn & 0x01) {
                status = MI_NOTAGERR;
            }
            if (Command == PCD_TRANSCEIVE) {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits) {
                    *pOutLenBit = (n-1)*8 + lastBits;
                } else {
                    *pOutLenBit = n*8;
                }
                if (n == 0) {
                    n = 1;
                }
                if (n > MAXRLEN) {
                    n = MAXRLEN;
                }
                for (i=0; i<n; i++) {
                    pOut [i] = ReadRawRC(FIFODataReg);
                }
            }
        } else {
            status = MI_ERR;
        }

    }
    SetBitMask(ControlReg,0x80);           // stop timer now
    WriteRawRC(CommandReg,PCD_IDLE);
    return status;
}

/////////////////////////////////////////////////////////////////////
//��������
//ÿ��������ر����߷���֮��Ӧ������1ms�ļ��
/////////////////////////////////////////////////////////////////////
void PcdAntennaOn(void)
{
    rt_uint8_t   i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03)) {
        SetBitMask(TxControlReg, 0x03);
    }
}


/////////////////////////////////////////////////////////////////////
//�ر�����
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff(void)
{
    ClearBitMask(TxControlReg, 0x03);
}
char num[9]= {0};
/*************************************
*�������ܣ�ת�����Ŀ��ţ���ʮ���������
*��������
***************************************/
void TurnID(rt_uint8_t *p)	 //ת�����Ŀ��ţ���ʮ��������ʾ
{
    unsigned char i;
    for(i=0; i<4; i++)
    {
        num[i*2]=p[i]>>4;
        num[i*2]>9?(num[i*2]+='7'):(num[i*2]+='0');
        num[i*2+1]=p[i]&0x0f;
        num[i*2+1]>9?(num[i*2+1]+='7'):(num[i*2+1]+='0');
        rt_kprintf("%c%c",num[i*2],num[i*2+1]);
    }
    rt_kprintf("\r\n");
    num[8]=0;
}
/*************************************
*�������ܣ���ȡ��ƬID
*��������
***************************************/
rt_uint8_t KEY[6]= {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
unsigned char ReadID(void)								//��SPI1��Ӧ�Ŀ�
{
		rt_uint8_t i;
		unsigned char status;
		status = PcdRequest(PICC_REQALL,CT);/*����*/
		PcdReset();
		PcdAntennaOff(); 
		PcdAntennaOn();
		status = PcdRequest(PICC_REQALL,CT);
		if(status == MI_OK)
		{
				status = MI_ERR;
				status = PcdAnticoll(SN);
		}
		if(status == MI_OK)
		{
				Card_flag = 1;
				status = MI_ERR;
				for(i=0; i<4; i++)
				{
						rt_kprintf("%x",SN[i]);
				}
				rt_kprintf("\r\n");
				TurnID(SN);	 //ת�����Ŀ��ţ���������num[]��
				for(i=0; i<4; i++)
				{
						SN[i] = 0;
				}
		}
		return status;
}


