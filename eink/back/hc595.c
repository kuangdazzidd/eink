#include "hc595.h"
#include <linux/gpio.h>


int hc595_init(void)
{
	int  err = 0;
	bool val = false;
    val  = gpio_is_valid(__GPIO_SHCP);
    val &= gpio_is_valid(__GPIO_STCP);
    val &= gpio_is_valid(__GPIO_DS);
    if (!val) {
    	printk("hc595 port not valid.\n");
    	return -1;
    }
    err  = gpio_request(__GPIO_SHCP, "shcp");
    err |= gpio_request(__GPIO_STCP, "stcp");
    err |= gpio_request(__GPIO_DS,   "ds");
    if (err != 0) {
    	printk("hc595 port requst failed. %d\n", err);
    	return -1;
    }

    gpio_direction_output(__GPIO_SHCP, 0);
    gpio_direction_output(__GPIO_STCP, 0);
    gpio_direction_output(__GPIO_DS, 0);

	printk("hc595 port init successful.\n");

    return 0;
}


int hc595_uninit(void)
{
    gpio_free(__GPIO_SHCP);
    gpio_free(__GPIO_STCP);
    gpio_free(__GPIO_DS);

	printk("hc595 port uninit.\n");

    return 0;
}


int hc595_write8(char val)
{
	int i = 0;
	
	for (i = 0; i < 8; i++) {
		gpio_set_value(__GPIO_SHCP, 0);
		if (val & 0x80) {
			gpio_set_value(__GPIO_DS, 1);
		} else {
			gpio_set_value(__GPIO_DS, 0);
		}
		val <<= 1;
		gpio_set_value(__GPIO_SHCP, 1);
	}
	gpio_set_value(__GPIO_STCP, 0);
	gpio_set_value(__GPIO_STCP, 1);
	gpio_set_value(__GPIO_STCP, 0);

	return 0;
}


int hc595_write16(short val)
{
	int i = 0;

	for (i = 0; i < 16; i++) {
		gpio_set_value(__GPIO_SHCP, 0);
		if (val & 0x80) {
			gpio_set_value(__GPIO_DS, 1);
		} else {
			gpio_set_value(__GPIO_DS, 0);
		}
		val <= 1;
		gpio_set_value(__GPIO_SHCP, 1);
	}
	gpio_set_value(__GPIO_STCP, 0);
	gpio_set_value(__GPIO_STCP, 1);
	gpio_set_value(__GPIO_STCP, 0);

	return 0;
}