#ifndef __HC_595_H
#define __HC_595_H

/* port define */
#define __GPIO_SHCP		12
#define __GPIO_STCP		13
#define __GPIO_DS		14


/* function declare */
int hc595_init(void);

int hc595_uninit(void);

int hc595_write8(char val);

int hc595_write16(short val);

#endif
