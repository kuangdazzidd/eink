#include "hc595.h"
#include <linux/gpio.h>

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
    int i    = 0;
    uint16_t val16 = 0;
    // __g_val8 = val;

    // for (i = 0; i < 8; i++) {
    //     gpio_set_value(__GPIO_595_SHCP, 0);
    //     if (val & 0x80) {
    //         gpio_set_value(__GPIO_595_DS, 1);
    //     } else {
    //         gpio_set_value(__GPIO_595_DS, 0);
    //     }
    //     val <<= 1;
    //     gpio_set_value(__GPIO_595_SHCP, 1);
    // }
    // gpio_set_value(__GPIO_595_STCP, 0);
    // gpio_set_value(__GPIO_595_STCP, 1);
    // gpio_set_value(__GPIO_595_STCP, 0);

    val16 = ((__g_val16 & 0xff00) | val);
    hc595_write16(val16);

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