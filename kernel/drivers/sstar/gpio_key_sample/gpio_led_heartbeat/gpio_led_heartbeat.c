/*
* gpio_led_heartbeat.c- Industio
*
* Copyright (c) [2019~2020] Industio Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/leds.h>
#include <asm/mach-types.h>
#include "../../../../drivers/sstar/include/infinity2m/gpio.h"

static struct gpio_led gpio_leds[]  = {
        {
                .name                   = "heartbeat",
                .default_trigger        = "heartbeat",
		.gpio                   = PAD_GPIO10,
        },
};

static struct gpio_led_platform_data gpio_led_info = {
        .leds           = gpio_leds,
        .num_leds       = ARRAY_SIZE(gpio_leds),
};

static void led_heartbeat_release(struct device *dev)
{
	;
}

static struct platform_device led_heartbeat = {
        .name   = "leds-gpio",
        .id     = -1,
        .dev.platform_data  = &gpio_led_info,
	.dev.release = led_heartbeat_release,
	.id = -1,
};

static int __init sstar_init(void)
{
	return platform_device_register(&led_heartbeat);
}
static void __exit sstar_exit(void)
{
	platform_device_unregister(&led_heartbeat);
}

module_init(sstar_init);
module_exit(sstar_exit);

MODULE_LICENSE("GPL");
