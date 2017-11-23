#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>



/*************************************************************************/
int hc595_init(void);
int hc595_uninit(void);
int hc595_write8(uint8_t val);
int hc595_write16(uint16_t val);
int hc595_write16_bit(uint8_t bit, uint8_t val);

int epd_port_init(void);
int epd_power_on(void);
int epd_power_off(void);
int epd_init(void);
int epd_uninit(void);


/*************************************************************************/
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

#define __GPIO_EPD_XCL		(__GPIO_PC_BASE + 3)  //IO-C03
#define __GPIO_EPD_EN_S		(__GPIO_PC_BASE + 0)  //IO-C00
#define __GPIO_EPD_EN_D		(__GPIO_PC_BASE + 2)  //IO-C02


/**
 *****595-1******
 *	0-7		D0-D7	
 *
 *****595-2******
 *	0		XLE		
 *	1		XOE
 *  2		XSTL2
 *	3		MODE
 *	4		SPV
 *	5		CKV
 */
#define __EPD_XCL_H()	gpio_set_value(__GPIO_EPD_XCL, 1)
#define __EPD_XCL_L()	gpio_set_value(__GPIO_EPD_XCL, 0)

#define __EPD_EN_S_H()	gpio_set_value(__GPIO_EPD_EN_S, 1)
#define __EPD_EN_S_L()	gpio_set_value(__GPIO_EPD_EN_S, 0)

#define __EPD_EN_D_H()	gpio_set_value(__GPIO_EPD_EN_D, 1)
#define __EPD_EN_D_L()	gpio_set_value(__GPIO_EPD_EN_D, 0)

#define __EPD_XLE_H()	hc595_write16_bit(0 + 8, 1)
#define __EPD_XLE_L()	hc595_write16_bit(0 + 8, 0)

#define __EPD_XOE_H()	hc595_write16_bit(1 + 8, 1)
#define __EPD_XOE_L()	hc595_write16_bit(1 + 8, 0)

#define __EPD_XSTL2_H()	hc595_write16_bit(2 + 8, 1)
#define __EPD_XSTL2_L()	hc595_write16_bit(2 + 8, 0)

#define __EPD_MODE_H()	hc595_write16_bit(3 + 8, 1)
#define __EPD_MODE_L()	hc595_write16_bit(3 + 8, 0)

#define __EPD_SPV_H()	hc595_write16_bit(4 + 8, 1)
#define __EPD_SPV_L()	hc595_write16_bit(4 + 8, 0)

#define __EPD_CKV_H()	hc595_write16_bit(5 + 8, 1)
#define __EPD_CKV_L()	hc595_write16_bit(5 + 8, 0)

#define __EPD_DATA(val)	hc595_write8(val)

/*************************************************************************/
/* global port state */
static uint16_t __g_val8  = 0;
static uint16_t __g_val16 = 0;


int hc595_init(void)
{
    int  err = 0;
    bool val = false;
    val  = gpio_is_valid(__GPIO_595_SHCP);
    val &= gpio_is_valid(__GPIO_595_STCP);
    val &= gpio_is_valid(__GPIO_595_DS);
    if (!val) {
        printk("hc595 port not valid.\n");
        return -1;
    }
    err  = gpio_request(__GPIO_595_SHCP, "shcp");
    err |= gpio_request(__GPIO_595_STCP, "stcp");
    err |= gpio_request(__GPIO_595_DS,   "ds");
    if (err != 0) {
        printk("hc595 port requst failed. %d\n", err);
        return -1;
    }

    gpio_direction_output(__GPIO_595_SHCP, 0);
    gpio_direction_output(__GPIO_595_STCP, 0);
    gpio_direction_output(__GPIO_595_DS, 0);

    printk("hc595 port init successful.\n");

    return 0;
}


int hc595_uninit(void)
{
    gpio_free(__GPIO_595_SHCP);
    gpio_free(__GPIO_595_STCP);
    gpio_free(__GPIO_595_DS);

    printk("hc595 port uninit.\n");

    return 0;
}


int hc595_write8(uint8_t val)
{
    int i = 0;
    
    __g_val8 = val;

    for (i = 0; i < 8; i++) {
        gpio_set_value(__GPIO_595_SHCP, 0);
        if (val & 0x80) {
            gpio_set_value(__GPIO_595_DS, 1);
        } else {
            gpio_set_value(__GPIO_595_DS, 0);
        }
        val <<= 1;
        gpio_set_value(__GPIO_595_SHCP, 1);
    }
    gpio_set_value(__GPIO_595_STCP, 0);
    gpio_set_value(__GPIO_595_STCP, 1);
    gpio_set_value(__GPIO_595_STCP, 0);

    return 0;
}


int hc595_write16(uint16_t val)
{
    int i = 0;

    __g_val16 = val;

    for (i = 0; i < 16; i++) {
        gpio_set_value(__GPIO_595_SHCP, 0);
        if (val & 0x8000) {
            gpio_set_value(__GPIO_595_DS, 1);
        } else {
            gpio_set_value(__GPIO_595_DS, 0);
        }
        val <<= 1;
        gpio_set_value(__GPIO_595_SHCP, 1);
    }
    gpio_set_value(__GPIO_595_STCP, 0);
    gpio_set_value(__GPIO_595_STCP, 1);
    gpio_set_value(__GPIO_595_STCP, 0);

    return 0;
}



int hc595_write16_bit(uint8_t bit, uint8_t val)
{
	if (val == 1) {
		__g_val16 |= (1 << bit);
	} else {
		__g_val16 &= ~(1 << bit);
	}

	hc595_write16(__g_val16);

	return 0;
}


/*************************************************************************/
int epd_port_init(void)
{
    int  err = 0;
    bool val = false;
    val  = gpio_is_valid(__GPIO_EPD_XCL);
    val &= gpio_is_valid(__GPIO_EPD_EN_S);
    val &= gpio_is_valid(__GPIO_EPD_EN_D);
    if (!val) {
        printk("epd port not valid.\n");
        return -1;
    }
    err  = gpio_request(__GPIO_EPD_XCL,  "xcl");
    err |= gpio_request(__GPIO_EPD_EN_S, "en_s");
    err |= gpio_request(__GPIO_EPD_EN_D, "en_d");
    if (err != 0) {
        printk("epd port requst failed. %d\n", err);
        return -1;
    }

    gpio_direction_output(__GPIO_EPD_XCL, 0);
    gpio_direction_output(__GPIO_EPD_EN_S, 0);
    gpio_direction_output(__GPIO_EPD_EN_D, 0);


    //all pins output low default
    hc595_write16(0x0000);

    printk("epd port init successful.\n");
	return 0;
}


int epd_power_on(void)
{
	msleep(200);
	__EPD_EN_S_H();
	msleep(200);
	__EPD_EN_D_H();
	return 0;
}

int epd_power_off(void)
{
	__EPD_EN_D_L();
	msleep(200);
	__EPD_EN_S_L();
	return 0;
}


int epd_init(void)
{
	int err = 0;

	epd_port_init();
	epd_power_on();

	//port default state
	__EPD_CKV_L();
	__EPD_SPV_H();
	__EPD_MODE_L();
	__EPD_XSTL2_H();
	__EPD_XOE_L();
	__EPD_XLE_L();
	__EPD_XCL_L();
	return 0;
}


int epd_uninit(void)
{
	//port uninit
	epd_power_off();

    gpio_free(__GPIO_EPD_XCL);
    gpio_free(__GPIO_EPD_EN_S);
    gpio_free(__GPIO_EPD_EN_D);
    printk("epd port uninit.\n");

    return 0;
}


int epd_test(void)
{
	int i = 0;
	int j = 0;

	__EPD_MODE_H();

	__EPD_CKV_H();
	usleep_range(100, 150);
	__EPD_SPV_L();
	usleep_range(100, 150);
	__EPD_CKV_L();
	usleep_range(100, 150);
	__EPD_CKV_H();
	usleep_range(100, 150);
	__EPD_SPV_H();
	usleep_range(100, 150);
	__EPD_CKV_L();
	usleep_range(100, 150);
	__EPD_CKV_H();
	usleep_range(200, 250);
	__EPD_CKV_L();
	usleep_range(100, 150);
	__EPD_CKV_H();
	usleep_range(200, 250);
	__EPD_CKV_L();
	usleep_range(100, 150);

	for (i = 0; i < 600; i++) {
		__EPD_CKV_H();

		/****/
		__EPD_XSTL2_L();
		__EPD_XOE_L();

		for (j = 0; j < 400; j++) {
			__EPD_XCL_L();
			__EPD_DATA(0X55);
			__EPD_XCL_H();	
		}
		__EPD_XLE_H();	
		__EPD_XLE_L();
		__EPD_XOE_H();
		/****/

		__EPD_CKV_L();
	}

	usleep_range(100, 150);
	__EPD_MODE_L();
}






/*************************************************************************/
uint16_t wave[4] = {0x55aa, 0xaa55, 0x55aa, 0xaa55};

static int __init empty_init(void)
{
	int i = 0;
	int err = 0;

    err = hc595_init();

    epd_init();
    msleep(500);
    for (i = 0; i < 25; ++i)
    {
    	epd_test();
    }
	
    // for (i = 0; i < 100; i++) {
	    // hc595_write16(i | (i << 8));
	    // gpio_set_value(__GPIO_EPD_XCL, i % 2);
    	// usleep_range(1000, 1500);
    	// msleep(200);
    // }

    printk("---dirver init---\r\n");
    return 0;
}

static void __exit empty_exit(void)
{
    hc595_write16(0x0000);
    epd_uninit();
    hc595_uninit();
    printk("---dirver exit---\r\n");
}

module_init(empty_init);
module_exit(empty_exit);

MODULE_LICENSE("GPL");
