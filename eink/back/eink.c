#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include "hc595.h"

static int __init empty_init(void)
{
	int err = 0;

    err = hc595_init();

    printk("---dirver init---\r\n");
    return 0;
}

static void __exit empty_exit(void)
{
    hc595_uninit();

    printk("---dirver exit---\r\n");
}

module_init(empty_init);
module_exit(empty_exit);

MODULE_LICENSE("GPL");
