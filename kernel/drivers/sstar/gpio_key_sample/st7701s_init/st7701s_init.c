/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Copyright (c) 2020 IDO GPIO Key Devices Inc.
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/timer.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include <linux/io.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <uapi/linux/input-event-codes.h>
#include <linux/workqueue.h>
#include "../drivers/sstar/include/infinity2m/gpio.h"

#define  SPI_CLK   PAD_GPIO6 
#define  SPI_DO    PAD_GPIO7 
#define  SPI_CS    PAD_GPIO11
#define  LCD_RST   PAD_GPIO10 

static void Delay(int ms)
{
	mdelay(ms);
}

static void SPI_SendData(unsigned char i)
{  
	unsigned char n;

	for(n=0; n<8; n++)   
	{  
		if(i&0x80)
		{
			//SPI_DI=1;
			gpio_set_value(SPI_DO, 1);   //gpio_set_value(SPI_CS, 1);  //数据输出 DO -- 输出
		}
		else
		{
			//SPI_DI=0;
			gpio_set_value(SPI_DO, 0);   //gpio_set_value(SPI_CS, 0);  //数据输出 DO -- 输出
		}

		i<<= 1;
		Delay(1);
		//SPI_CLK=0;//_nop_(); _nop_();_nop_();_nop_();
		gpio_set_value(SPI_CLK, 0);   //时钟 CLK -- 输出
		Delay(1);
		//SPI_CLK=1;//_nop_();_nop_();_nop_();_nop_();
		gpio_set_value(SPI_CLK, 1);   //时钟 CLK -- 输出
		Delay(1);
	}
}

//-----------------------------------------------
static void SPI_WriteComm(unsigned char i)
{
	//SPI_CS=0;
	gpio_set_value(SPI_CS, 0);   //片选 CS
	Delay(1);

	//SPI_DI=0;
	gpio_set_value(SPI_DO, 0);  //数据输出 DO -- 输出
	Delay(1);

	//SPI_CLK=0;//_nop_(); _nop_();_nop_();_nop_();
	gpio_set_value(SPI_CLK, 0);   //时钟 CLK -- 输出
	Delay(1);

	//SPI_CLK=1;//_nop_();_nop_();_nop_();_nop_();
	gpio_set_value(SPI_CLK, 1);   //时钟 CLK -- 输出
	Delay(1);

	SPI_SendData(i);
	Delay(1);

	//SPI_CS=1;	
	gpio_set_value(SPI_CS, 1);   //片选 CS
	Delay(1);    

}

//-----------------------------------------------
static void SPI_WriteData(unsigned char i)
{

	//	SPI_CS=0;
	gpio_set_value(SPI_CS, 0);   //片选 CS 
	Delay(1);
	//	SPI_DI=1;
	gpio_set_value(SPI_DO, 1);  //数据输出 DO -- 输出
	Delay(1);

	//	SPI_CLK=0;//_nop_(); _nop_();_nop_();_nop_();
	gpio_set_value(SPI_CLK, 0);   //时钟 CLK -- 输出
	Delay(1);

	//	SPI_CLK=1;//_nop_();_nop_();_nop_();_nop_();
	gpio_set_value(SPI_CLK, 1);   //时钟 CLK -- 输出
	Delay(1);

	SPI_SendData(i);
	Delay(1);

	//  SPI_CS=1;
	gpio_set_value(SPI_CS, 1);   //片选 CS 
	Delay(1);

}

static void BT040WV3_SD05_init(void)
{
	gpio_set_value(LCD_RST, 1);   
	mdelay(80);
	gpio_set_value(LCD_RST, 0);   
	mdelay(80);
	gpio_set_value(LCD_RST, 1);   	
	mdelay(150);
#if 1
	//customer init code
	SPI_WriteComm(0x11);
	mdelay(120);

	SPI_WriteComm(0xFF);
	SPI_WriteData(0x77);
	SPI_WriteData(0x01);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x10);
	SPI_WriteComm(0xC0);
	SPI_WriteData(0x3B);
	SPI_WriteData(0x00);

	SPI_WriteComm(0xC1);
	SPI_WriteData(0x0D);
	SPI_WriteData(0x02);

	SPI_WriteComm(0xC2);
	SPI_WriteData(0x21);
	SPI_WriteData(0x08);

	SPI_WriteComm(0xC3); 
	//SPI_WriteData(0x80);  
	//SPI_WriteData(0x10); 
	//SPI_WriteData(0x08);
	SPI_WriteData(0x0C);
	//SPI_WriteData(0x82);
	//SPI_WriteData(0x50);
	//SPI_WriteData(0x1E);

	//add
	/*
	SPI_WriteComm(0xCC);
	SPI_WriteData(0x10);	
	SPI_WriteComm(0xCD);
	SPI_WriteData(0x08);	
	*/

	SPI_WriteComm(0xB0);
	SPI_WriteData(0x00);
	SPI_WriteData(0x11);
	SPI_WriteData(0x18);
	SPI_WriteData(0x0E);
	SPI_WriteData(0x11);
	SPI_WriteData(0x06);
	SPI_WriteData(0x07);
	SPI_WriteData(0x08);
	SPI_WriteData(0x07);
	SPI_WriteData(0x22);
	SPI_WriteData(0x04);
	SPI_WriteData(0x12);
	SPI_WriteData(0x0F);
	SPI_WriteData(0xAA);
	SPI_WriteData(0x31);
	SPI_WriteData(0x18);

	SPI_WriteComm(0xB1);
	SPI_WriteData(0x00);
	SPI_WriteData(0x11);
	SPI_WriteData(0x19);
	SPI_WriteData(0x0E);
	SPI_WriteData(0x12);
	SPI_WriteData(0x07);
	SPI_WriteData(0x08);
	SPI_WriteData(0x08);
	SPI_WriteData(0x08);
	SPI_WriteData(0x22);
	SPI_WriteData(0x04);
	SPI_WriteData(0x11);
	SPI_WriteData(0x11);
	SPI_WriteData(0xA9);
	SPI_WriteData(0x32);
	SPI_WriteData(0x18);

	SPI_WriteComm(0xFF);
	SPI_WriteData(0x77);
	SPI_WriteData(0x01);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x11);

	SPI_WriteComm(0xB0);
	SPI_WriteData(0x60);

	SPI_WriteComm(0xB1);
	SPI_WriteData(0x30);

	SPI_WriteComm(0xB2);
	SPI_WriteData(0x87);

	SPI_WriteComm(0xB3);
	SPI_WriteData(0x80);

	SPI_WriteComm(0xB5);
	SPI_WriteData(0x49);

	SPI_WriteComm(0xB7);
	SPI_WriteData(0x85);

	SPI_WriteComm(0xB8);
	SPI_WriteData(0x21);

	SPI_WriteComm(0xC1);
	SPI_WriteData(0x78);

	SPI_WriteComm(0xC2);
	SPI_WriteData(0x78);
	mdelay(20);

	SPI_WriteComm(0xE0);
	SPI_WriteData(0x00);
	SPI_WriteData(0x1B);
	SPI_WriteData(0x02);

	SPI_WriteComm(0xE1);
	SPI_WriteData(0x08);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x07);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x44);
	SPI_WriteData(0x44);

	SPI_WriteComm(0xE2);
	SPI_WriteData(0x11);
	SPI_WriteData(0x11);
	SPI_WriteData(0x44);
	SPI_WriteData(0x44);
	SPI_WriteData(0xED);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0xEC);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);

	SPI_WriteComm(0xE3);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x11);
	SPI_WriteData(0x11);

	SPI_WriteComm(0xE4);
	SPI_WriteData(0x44);
	SPI_WriteData(0x44);

	SPI_WriteComm(0xE5);
	SPI_WriteData(0x0A);
	SPI_WriteData(0xE9);
	SPI_WriteData(0xD8);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x0C);
	SPI_WriteData(0xEB);
	SPI_WriteData(0xD8);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x0E);
	SPI_WriteData(0xED);
	SPI_WriteData(0xD8);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x10);
	SPI_WriteData(0xEF);
	SPI_WriteData(0xD8);
	SPI_WriteData(0xA0);

	SPI_WriteComm(0xE6);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x11);
	SPI_WriteData(0x11);

	SPI_WriteComm(0xE7);
	SPI_WriteData(0x44);
	SPI_WriteData(0x44);

	SPI_WriteComm(0xE8);
	SPI_WriteData(0x09);
	SPI_WriteData(0xE8);
	SPI_WriteData(0xD8);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x0B);
	SPI_WriteData(0xEA);
	SPI_WriteData(0xD8);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x0D);
	SPI_WriteData(0xEC);
	SPI_WriteData(0xD8);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x0F);
	SPI_WriteData(0xEE);
	SPI_WriteData(0xD8);
	SPI_WriteData(0xA0);

	SPI_WriteComm(0xEB);
	SPI_WriteData(0x02);
	SPI_WriteData(0x00);
	SPI_WriteData(0xE4);
	SPI_WriteData(0xE4);
	SPI_WriteData(0x88);
	SPI_WriteData(0x00);
	SPI_WriteData(0x40);
	SPI_WriteComm(0xEC);
	SPI_WriteData(0x3C);
	SPI_WriteData(0x00);

	SPI_WriteComm(0xED);
	SPI_WriteData(0xAB);
	SPI_WriteData(0x89);
	SPI_WriteData(0x76);
	SPI_WriteData(0x54);
	SPI_WriteData(0x02);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0x20);
	SPI_WriteData(0x45);
	SPI_WriteData(0x67);
	SPI_WriteData(0x98);
	SPI_WriteData(0xBA);

	SPI_WriteComm(0xFF);
	SPI_WriteData(0x77);
	SPI_WriteData(0x01);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);

	SPI_WriteComm(0x11);
	mdelay (120);

	SPI_WriteComm(0x3A);
	SPI_WriteData(0x66);

	SPI_WriteComm(0x36);//配置 RGB 顺序
	//SPI_WriteData(0x00);
	SPI_WriteData(0x08);
		//SPI_WriteData(0x18);

	SPI_WriteComm(0x29);
#else
	//xnp init code
	SPI_WriteComm (0xFF);
	SPI_WriteData (0x77);
	SPI_WriteData (0x01);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x10);

	SPI_WriteComm (0xC0);
	SPI_WriteData (0x3B);
	SPI_WriteData (0x00);

	SPI_WriteComm (0xC1);
	SPI_WriteData (0x0B);
	SPI_WriteData (0x02);

	SPI_WriteComm (0xC2);
	SPI_WriteData (0x00);
	SPI_WriteData (0x02);

	SPI_WriteComm(0xC3);
	SPI_WriteData(0x0c);
	//SPI_WriteData(0x10);
	//SPI_WriteData(0x08);

	SPI_WriteComm (0xCC);
	SPI_WriteData (0x10);

	SPI_WriteComm (0xCD);
	SPI_WriteData (0x08);

	SPI_WriteComm (0xB0);
	SPI_WriteData (0x00);
	SPI_WriteData (0x1D);
	SPI_WriteData (0x29);
	SPI_WriteData (0x12);
	SPI_WriteData (0x17);
	SPI_WriteData (0x0B);
	SPI_WriteData (0x18);
	SPI_WriteData (0x09);
	SPI_WriteData (0x08);
	SPI_WriteData (0x2A);
	SPI_WriteData (0x06);
	SPI_WriteData (0x14);
	SPI_WriteData (0x11);
	SPI_WriteData (0x19);
	SPI_WriteData (0x1C);
	SPI_WriteData (0x18);

	SPI_WriteComm (0xB1);
	SPI_WriteData (0x00);
	SPI_WriteData (0x1D);
	SPI_WriteData (0x29);
	SPI_WriteData (0x12);
	SPI_WriteData (0x16);
	SPI_WriteData (0x0A);
	SPI_WriteData (0x18);
	SPI_WriteData (0x08);
	SPI_WriteData (0x09);
	SPI_WriteData (0x2A);
	SPI_WriteData (0x07);
	SPI_WriteData (0x13);
	SPI_WriteData (0x12);
	SPI_WriteData (0x19);
	SPI_WriteData (0x1D);
	SPI_WriteData (0x18);

	SPI_WriteComm (0xFF);
	SPI_WriteData (0x77);
	SPI_WriteData (0x01);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x11);

	SPI_WriteComm (0xB0);
	SPI_WriteData (0x5D);
	SPI_WriteComm (0xB1);
	SPI_WriteData (0x43);
	SPI_WriteComm (0xB2);
	SPI_WriteData (0x81);
	SPI_WriteComm (0xB3);
	SPI_WriteData (0x80);
	SPI_WriteComm (0xB5);
	SPI_WriteData (0x43);
	SPI_WriteComm (0xB7);
	SPI_WriteData (0x85);
	SPI_WriteComm (0xB8);
	SPI_WriteData (0x20);
	SPI_WriteComm (0xC1);
	SPI_WriteData (0x78);
	SPI_WriteComm (0xC2);
	SPI_WriteData (0x78);
	SPI_WriteComm (0xD0);
	SPI_WriteData (0x88);
	mdelay (20);
	SPI_WriteComm (0xE0);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x02);

	SPI_WriteComm (0xE1);
	SPI_WriteData (0x03);
	SPI_WriteData (0xA0);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x04);
	SPI_WriteData (0xA0);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x20);
	SPI_WriteData (0x20);

	SPI_WriteComm (0xE2);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);

	SPI_WriteComm (0xE3);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x11);
	SPI_WriteData (0x00);

	SPI_WriteComm (0xE4);
	SPI_WriteData (0x22);
	SPI_WriteData (0x00);
	SPI_WriteComm (0xE5);
	SPI_WriteData(0x05);
	SPI_WriteData(0xEC);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x07);
	SPI_WriteData(0xEE);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);

	SPI_WriteComm(0xE6);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x11);
	SPI_WriteData(0x00);

	SPI_WriteComm(0xE7);
	SPI_WriteData(0x22);
	SPI_WriteData(0x00);

	SPI_WriteComm(0xE8);
	SPI_WriteData(0x06);
	SPI_WriteData(0xED);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x08);
	SPI_WriteData(0xEF);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xA0);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);

	SPI_WriteComm(0xEB);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x40);
	SPI_WriteData(0x40);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);

	SPI_WriteComm(0xED);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xBA);
	SPI_WriteData(0x0A);
	SPI_WriteData(0xBF);
	SPI_WriteData(0x45);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0x54);
	SPI_WriteData(0xFB);
	SPI_WriteData(0xA0);
	SPI_WriteData(0xAB);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);
	SPI_WriteData(0xFF);

	SPI_WriteComm(0xEF);
	SPI_WriteData(0x10);
	SPI_WriteData(0x0D);
	SPI_WriteData(0x04);
	SPI_WriteData(0x08);
	SPI_WriteData(0x3F);
	SPI_WriteData(0x1F);
	SPI_WriteComm(0xFF);
	SPI_WriteData(0x77);
	SPI_WriteData(0x01);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x13);
	SPI_WriteComm(0xEF);
	SPI_WriteData(0x08);
	SPI_WriteComm(0xFF);
	SPI_WriteData(0x77);
	SPI_WriteData(0x01);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);
	SPI_WriteData(0x00);

	SPI_WriteComm(0x11);
	mdelay (120);
	SPI_WriteComm(0x29);
	SPI_WriteComm(0x36);
	SPI_WriteData(0x00);
	SPI_WriteComm(0x3A);
	SPI_WriteData(0x60);
#endif
}

static int ido_7701s_probe(struct platform_device *pdev)
{	

	if ( 0 == gpio_request(SPI_CLK ,"spi_clk")) {
		gpio_direction_output(SPI_CLK, 1);
	} else {
		printk("*******spi_clk request error********************\r\n");
		return -1;
	}

	if ( 0 == gpio_request(SPI_DO ,"spi_do")) {
		gpio_direction_output(SPI_DO, 1);
	} else {
		printk("*******spi_do request error********************\r\n");
		return -1;
	}

	if ( 0 == gpio_request(SPI_CS ,"spi_cs")) {
		gpio_direction_output(SPI_CS, 1);
	} else {
		printk("*******spi_cs request error********************\r\n");
		return -1;
	}

	if ( 0 == gpio_request(LCD_RST ,"lcd_rst")) {
		gpio_direction_output(LCD_RST, 1);
	} else {
		printk("*******lcd_rst request error********************\r\n");
		return -1;
	}

	BT040WV3_SD05_init();

	printk(KERN_INFO "[%s] succeed.\n", __func__);
	return 0;
}


static int ido_7701s_remove(struct platform_device *pdev)
{

	printk(KERN_INFO "[%s].\n", __func__);
	return 0;
}

static const struct of_device_id ido_of_match[] = {
	{ .compatible = "ido,7701s" },
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, ido_of_match);

static struct platform_driver ido_7701s_driver = {
	.probe  = ido_7701s_probe,
	.remove = ido_7701s_remove,
	.driver = {
		.name   = "7701s",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(ido_of_match),
	},
};

static int __init ido_7701s_init(void)
{
	platform_driver_register(&ido_7701s_driver);
	printk(KERN_INFO "[%s]\n", __func__);
	return 0;
}

static void __exit ido_7701s_exit(void)
{
	platform_driver_unregister(&ido_7701s_driver);
	printk(KERN_INFO "[%s].\n", __func__);
}

module_init(ido_7701s_init);
module_exit(ido_7701s_exit);

MODULE_AUTHOR("Lyle");
MODULE_DESCRIPTION("ido-7701s driver");
MODULE_LICENSE("GPL");
