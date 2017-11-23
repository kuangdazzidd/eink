#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>



/*************************************************************************/
int epd_port_init(void);
int epd_power_on(void);
int epd_power_off(void);
int epd_init(void);
int epd_uninit(void);



/*************************************************************************/
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


#define __BYTE_WHITE    0xAA
#define __BYTE_BLACK    0x55

#define __EPD_WIDTH     600
#define __EPD_HEIGHT    800
#define __FRAME_SIZE    ((__EPD_HEIGHT / 4) * __EPD_WIDTH + 100)

/*************************************************************************/
static uint8_t  __g_line_data[200];
static uint8_t *__gp_frame_buffer = NULL;
//static uint8_t __gp_frame_buffer[600][200] = {__BYTE_BLACK};



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

	__gp_frame_buffer = vmalloc(__FRAME_SIZE);
	if (__gp_frame_buffer == NULL) {
	    printk("__gp_frame_buffer malloc error.\n");
	    return -1;
	}

	memset(__gp_frame_buffer, __BYTE_BLACK, __FRAME_SIZE);
	printk("__gp_frame_buffer malloc successful at %x.\n", __gp_frame_buffer);

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

	if (__gp_frame_buffer != NULL) {
	    vfree(__gp_frame_buffer);
	}

    gpio_free(__GPIO_EPD_XCL);
    gpio_free(__GPIO_EPD_EN_S);
    gpio_free(__GPIO_EPD_EN_D);
    printk("epd port uninit.\n");

    return 0;
}



void epd_vclock_quick(void)
{
    int i;

    for (i = 0; i < 2; i++)
    {
        __EPD_CKV_L();
        usleep_range(10, 15);
        __EPD_CKV_H();
        usleep_range(10, 15);
    }
}


void epd_start_scan(void)
{
    __EPD_MODE_H();
    __EPD_SPV_H();
    epd_vclock_quick();
    __EPD_SPV_L();
    epd_vclock_quick();
    __EPD_SPV_H();
    epd_vclock_quick();
}


void epd_stop_scan(void)
{
    __EPD_MODE_L();
}


void epd_send_one_row(uint8_t *p_data)
{
    int i = 0;
    __EPD_XLE_H();
    __EPD_XCL_H();
    __EPD_XLE_L();
    __EPD_XCL_H();
    __EPD_XLE_L();

    __EPD_XLE_L();
    __EPD_XCL_H();
    __EPD_XLE_L();
    __EPD_XCL_H();
    __EPD_XLE_L();

    __EPD_XOE_H();

    //开始一行数据
    __EPD_XSTL2_L();

    for (i = 0; i < 200; i++) {
        __EPD_DATA(p_data[i]);
        __EPD_XCL_H();
        __EPD_XCL_L();
    }

    __EPD_XSTL2_H();

    __EPD_XCL_H();
    __EPD_XLE_L();
    __EPD_XCL_H();
    __EPD_XLE_L();

    //输出数据
    __EPD_CKV_L();
    __EPD_XOE_L();

    __EPD_XCL_H();
    __EPD_XLE_L();
    __EPD_XCL_H();
    __EPD_XLE_L();

    __EPD_CKV_H();
}



void epd_show_frame(void)
{
    int i = 0;
    epd_start_scan();
    for (i = 0; i < 600; i++) {
        epd_send_one_row(&__gp_frame_buffer[i * (__EPD_HEIGHT / 4)]);
//        epd_send_one_row(__g_line_data);
    }
    epd_stop_scan();
}


void epd_frame_buf_init(void)
{
    int i = 0, j = 0;
    for (i = 0; i < 600; i++) {

    }
}



/*************************************************************************/
int epd_draw_point(uint16_t x, uint16_t y, uint8_t color)
{
    uint8_t  val = 0;
    uint32_t pos = 0;

    if (x < 0| y < 0) {
        return -1;
    }

    pos = (y * (__EPD_HEIGHT / 4)) + (x / 4);

    __gp_frame_buffer[pos] &= ~(0x3 << ((x % 4) * 2));

    if (color == __BYTE_BLACK) {
        __gp_frame_buffer[pos] |= (0x1 << ((x % 4) * 2));
    } else {
        __gp_frame_buffer[pos] |= (0x2 << ((x % 4) * 2));
    }

    return 0;
}


int epd_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color)
{
    return 0;
}


int epd_draw_line_h(uint16_t x, uint16_t y, uint16_t length, uint8_t width, uint8_t color)
{
    int i = 0, j = 0;
    for (j = y; j < (y + width); j++) {
        for (i = x; i < (x + length); i++) {
            epd_draw_point(i, j, color);
        }
    }
    return 0;
}


int epd_draw_line_v(uint16_t x, uint16_t y, uint16_t length, uint8_t width, uint8_t color)
{
    int i = 0, j = 0;
    for (j = x; j < (x + width); j++) {
        for (i = y; i < (y + length); i++) {
            epd_draw_point(j, i, color);
        }
    }
    return 0;
}


int epd_test(void)
{
    int i = 0;

    epd_draw_line_h(200, 300, 400, 3, __BYTE_WHITE);
    epd_draw_line_h(200, 100, 400, 5, __BYTE_WHITE);
    epd_draw_line_h(200, 500, 400, 5, __BYTE_WHITE);
//    epd_draw_line_v(309, 100, 80, 5, __BYTE_BLACK);
    for (i = 0; i < 25; i++) {
        epd_show_frame();
        printk("frame %d.\n", i);
    }
}


/*************************************************************************/
static int __init empty_init(void)
{
	int i = 0;
	int err = 0;

    err = hc595_init();

    epd_init();
    msleep(500);

    epd_test();

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
