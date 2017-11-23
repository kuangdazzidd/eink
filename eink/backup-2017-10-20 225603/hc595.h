#ifndef __HC_595_H
#define __HC_595_H

#include <linux/gpio.h>

/* port define */
#define __GPIO_PA_BASE	0
#define __GPIO_PB_BASE	32
#define __GPIO_PC_BASE	64
#define __GPIO_PD_BASE	96
#define __GPIO_PE_BASE	128
#define __GPIO_PF_BASE	160
#define __GPIO_PG_BASE	192
#define __GPIO_PH_BASE	224
#define __GPIO_PI_BASE	256
#define __GPIO_PL_BASE	352
#define __GPIO_PM_BASE	384
#define __GPIO_PN_BASE	416

/* port allot */
#define __GPIO_595_SHCP     (__GPIO_PB_BASE + 6)  //IO-B04
#define __GPIO_595_STCP     (__GPIO_PB_BASE + 7)  //IO-B05
#define __GPIO_595_DS       (__GPIO_PC_BASE + 1)  //IO-C01


/* function declare */
int hc595_init(void);
int hc595_uninit(void);
int hc595_write8(uint8_t val);
int hc595_write16(uint16_t val);
int hc595_write16_bit(uint8_t bit, uint8_t val);

#endif
