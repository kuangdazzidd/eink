/*******************************************************************************
  EPD DRIVER FOR STM32F2/4 w/ FSMC
  By ZephRay(zephray@outlook.com)
*******************************************************************************/
#ifndef __EPD_H__
#define __EPD_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
//Use new ED060SC4 H1/H2 screens
//if you are using ED060SC4 without any postfix, comment this



int hc595_init(void);
int hc595_uninit(void);
int hc595_write8(uint8_t val);
int hc595_write16(uint16_t val);
int hc595_write16_bit(uint8_t bit, uint8_t val);

//¿ªÆôHscan
#define USE_H_SCREEN 


#define BYTE_WHITE 0xAA
#define BYTE_BLACK 0x55

#define CLEAR_MODE_BLACK  0
#define CLEAR_MODE_WHITE  1
#define CLEAR_MODE_STREAK 2

extern const unsigned char wave_init[200];

#define PulseDelay()    {usleep_range(10, 15);}



#define __GPIO_PA_BASE  0
#define __GPIO_PB_BASE  32
#define __GPIO_PC_BASE  64
#define __GPIO_PD_BASE  96
#define __GPIO_PE_BASE  128
#define __GPIO_PF_BASE  160
#define __GPIO_PG_BASE  192
#define __GPIO_PH_BASE  224
#define __GPIO_PI_BASE  256
#define __GPIO_PL_BASE  352
#define __GPIO_PM_BASE  384
#define __GPIO_PN_BASE  416

/* port allot */
#define __GPIO_595_SHCP     (__GPIO_PB_BASE + 6)  //IO-B04
#define __GPIO_595_STCP     (__GPIO_PB_BASE + 7)  //IO-B05
#define __GPIO_595_DS       (__GPIO_PC_BASE + 1)  //IO-C01

#define __GPIO_EPD_XCL      (__GPIO_PC_BASE + 3)  //IO-C03
#define __GPIO_EPD_EN_S     (__GPIO_PC_BASE + 0)  //IO-C00
#define __GPIO_EPD_EN_D     (__GPIO_PC_BASE + 2)  //IO-C02


/**
 *****595-1******
 *  0-7     D0-D7
 *
 *****595-2******
 *  0       XLE
 *  1       XOE
 *  2       XSTL2
 *  3       MODE
 *  4       SPV
 *  5       CKV
 */
#define __EPD_XCL_H()   gpio_set_value(__GPIO_EPD_XCL, 1)
#define __EPD_XCL_L()   gpio_set_value(__GPIO_EPD_XCL, 0)

#define __EPD_EN_S_H()  gpio_set_value(__GPIO_EPD_EN_S, 1)
#define __EPD_EN_S_L()  gpio_set_value(__GPIO_EPD_EN_S, 0)

#define __EPD_EN_D_H()  gpio_set_value(__GPIO_EPD_EN_D, 1)
#define __EPD_EN_D_L()  gpio_set_value(__GPIO_EPD_EN_D, 0)

#define __EPD_XLE_H()   hc595_write16_bit(0 + 8, 1)
#define __EPD_XLE_L()   hc595_write16_bit(0 + 8, 0)

#define __EPD_XOE_H()   hc595_write16_bit(1 + 8, 1)
#define __EPD_XOE_L()   hc595_write16_bit(1 + 8, 0)

#define __EPD_XSTL2_H() hc595_write16_bit(2 + 8, 1)
#define __EPD_XSTL2_L() hc595_write16_bit(2 + 8, 0)

#define __EPD_MODE_H()  hc595_write16_bit(3 + 8, 1)
#define __EPD_MODE_L()  hc595_write16_bit(3 + 8, 0)

#define __EPD_SPV_H()   hc595_write16_bit(4 + 8, 1)
#define __EPD_SPV_L()   hc595_write16_bit(4 + 8, 0)

#define __EPD_CKV_H()   hc595_write16_bit(5 + 8, 1)
#define __EPD_CKV_L()   hc595_write16_bit(5 + 8, 0)

#define __EPD_DATA(val) hc595_write8(val)

//V
#define EPD_EN_N_L()        {__EPD_EN_S_L(); PulseDelay();}
#define EPD_EN_N_H()        {__EPD_EN_S_H(); PulseDelay();}
#define EPD_EN_P_L()        {__EPD_EN_D_L(); PulseDelay();}
#define EPD_EN_P_H()        {__EPD_EN_D_H(); PulseDelay();}

//SOURCE DRIVER
#define EPD_CL_L()        {__EPD_XCL_L(); PulseDelay();}
#define EPD_CL_H()        {__EPD_XCL_H(); PulseDelay();}
#define EPD_LE_L()        {__EPD_XLE_L(); PulseDelay();}
#define EPD_LE_H()        {__EPD_XLE_H(); PulseDelay();}
#define EPD_OE_L()        {__EPD_XOE_L(); PulseDelay();}
#define EPD_OE_H()        {__EPD_XOE_H(); PulseDelay();}
#define EPD_SPH_L()       {__EPD_XSTL2_L(); PulseDelay();}
#define EPD_SPH_H()       {__EPD_XSTL2_H(); PulseDelay();}
#define EPD_SHR_L()       {PulseDelay();}
#define EPD_SHR_H()       {PulseDelay();}

//GATE DRIVER
#define EPD_GMODE1_L()    {__EPD_MODE_L(); PulseDelay();}
#define EPD_GMODE1_H()    {__EPD_MODE_H(); PulseDelay();}
#define EPD_GMODE2_L()    {__EPD_MODE_L(); PulseDelay();}
#define EPD_GMODE2_H()    {__EPD_MODE_H(); PulseDelay();}
#define EPD_XRL_L()       {PulseDelay();}
#define EPD_XRL_H()       {PulseDelay();}
#define EPD_SPV_L()       {__EPD_SPV_L(); PulseDelay();}
#define EPD_SPV_H()       {__EPD_SPV_H(); PulseDelay();}
#define EPD_CKV_L()       {__EPD_CKV_L(); PulseDelay();}
#define EPD_CKV_H()       {__EPD_CKV_H(); PulseDelay();}

void EPD_Init(void);

void EPD_Power_On_Vvdd(void);
void EPD_Power_Off_Vpos(void);
void EPD_Power_On_Vpos(void);
void EPD_Power_Off_Vneg(void);
void EPD_Power_OnVneg(void);


void EPD_Part_Refresh(const uint8_t *array);

void EPD_Clear(uint8_t mode);
void EPD_Show_Array(const uint8_t *array);
void EPD_DispPicture(void);
void EPD_PrepareWaveform(void);
void EPD_DispPic(void);
void EPD_DispScr(unsigned int startLine, unsigned int lineCount);
void EPD_ClearFB(unsigned char c);
void EPD_SetPixel(unsigned short x,unsigned short y,unsigned char color);
void EPD_Line(unsigned short x0,unsigned short y0,unsigned short x1,unsigned short y1,unsigned short color);
void EPD_PutChar_16(unsigned short x, unsigned short y, unsigned short chr, unsigned char color);
void EPD_PutChar_Legacy(unsigned short x, unsigned short y, unsigned short chr, unsigned char color);
void EPD_String_16(unsigned short x,unsigned short y,unsigned char *s,unsigned char color);
void EPD_String_24(unsigned short x,unsigned short y,unsigned char *s,unsigned char color);
void EPD_FillRect(unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2,unsigned char color);

#endif
