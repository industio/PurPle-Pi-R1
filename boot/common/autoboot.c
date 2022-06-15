/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <autoboot.h>
#include <bootretry.h>
#include <cli.h>
#include <fdtdec.h>
#include <menu.h>
#include <post.h>
#ifdef CONFIG_SSTAR_IR
#include <ms_ir.h>
#endif

#include <stdlib.h>

#if defined(CONFIG_MS_PARTITION)
#include "part_mxp.h"
#endif


#if defined(CONFIG_CMD_MTDPARTS)
#include <jffs2/jffs2.h>
/* partition handling routines */
int mtdparts_init(void);
int find_dev_and_part(const char *id, struct mtd_device **dev,
        u8 *part_num, struct part_info **part);
#endif


DECLARE_GLOBAL_DATA_PTR;

#define MAX_DELAY_STOP_STR 32

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
static int stored_bootdelay;

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
# if defined(CONFIG_AUTOBOOT_KEYED)
static int abortboot_keyed(int bootdelay)
{
	int abort = 0;
	uint64_t etime = 2;//endtick(bootdelay);
	struct {
		char *str;
		u_int len;
		int retry;
	}
	delaykey[] = {
		{ .str = getenv("bootdelaykey"),  .retry = 1 },
//		{ .str = getenv("bootdelaykey2"), .retry = 1 },
//		{ .str = getenv("bootstopkey"),   .retry = 0 },
//		{ .str = getenv("bootstopkey2"),  .retry = 0 },
	};

	char presskey[MAX_DELAY_STOP_STR]={0};
	u_int presskey_len = 0;
	u_int presskey_max = 0;
	u_int i;

#ifndef CONFIG_ZERO_BOOTDELAY_CHECK
	if (bootdelay == 0)
		return 0;
#endif

#  ifdef CONFIG_AUTOBOOT_PROMPT
	printf(CONFIG_AUTOBOOT_PROMPT);
#  endif

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	if (delaykey[0].str == NULL)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#if 0
#  ifdef CONFIG_AUTOBOOT_DELAY_STR2
	if (delaykey[1].str == NULL)
		delaykey[1].str = CONFIG_AUTOBOOT_DELAY_STR2;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	if (delaykey[2].str == NULL)
		delaykey[2].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR2
	if (delaykey[3].str == NULL)
		delaykey[3].str = CONFIG_AUTOBOOT_STOP_STR2;
#  endif
#endif
	for (i = 0; i < sizeof(delaykey) / sizeof(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == NULL ?
				    0 : strlen(delaykey[i].str);
		delaykey[i].len = delaykey[i].len > MAX_DELAY_STOP_STR ?
				    MAX_DELAY_STOP_STR : delaykey[i].len;

		presskey_max = presskey_max > delaykey[i].len ?
				    presskey_max : delaykey[i].len;

		debug_bootkeys("%s key:<%s>\n",
			       delaykey[i].retry ? "delay" : "stop",
			       delaykey[i].str ? delaykey[i].str : "NULL");
	}

	/* In order to keep up with incoming data, check timeout only
	 * when catch up.
	 */
	do {
		if (tstc()) {
			if (presskey_len < presskey_max) {
				presskey[presskey_len++] = getc();
			} else {
				for (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getc();
			}
		}

		for (i = 0; i < sizeof(delaykey) / sizeof(delaykey[0]); i++) {
			if (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				if (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
		etime--;
	} while (!abort && etime);

#if 0// if DEBUG_BOOTKEYS
	if (!abort)
		debug_bootkeys("key timeout\n");

#endif

#ifdef CONFIG_SILENT_CONSOLE
	if (abort)
		gd->flags &= ~GD_FLG_SILENT;
#endif
	if (tstc()) {
	    (void) getc();  /* consume input	*/
	}
	return abort;
}

# else	/* !defined(CONFIG_AUTOBOOT_KEYED) */

#ifdef CONFIG_MENUKEY
static int menukey;
#endif

static int abortboot_normal(int bootdelay)
{
	int abort = 0;
	unsigned long ts;

#ifdef CONFIG_MENUPROMPT
	printf(CONFIG_MENUPROMPT);
#else
	if (bootdelay >= 0)
		printf("Hit any key to stop autoboot: %2d ", bootdelay);
#endif

#if defined CONFIG_ZERO_BOOTDELAY_CHECK
	/*
	 * Check if key already pressed
	 * Don't check if bootdelay < 0
	 */
	if (bootdelay >= 0) {
		if (tstc()) {	/* we got a key press	*/
			(void) getc();  /* consume input	*/
			puts("\b\b\b 0");
			abort = 1;	/* don't auto boot	*/
		}
	}
#endif

	while ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		do {
			if (tstc()) {	/* we got a key press	*/
				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
# ifdef CONFIG_MENUKEY
				menukey = getc();
# else
				(void) getc();  /* consume input	*/
# endif
				break;
			}
			udelay(10000);
		} while (!abort && get_timer(ts) < 1000);

		printf("\b\b\b%2d ", bootdelay);
	}

	putc('\n');

#ifdef CONFIG_SILENT_CONSOLE
	if (abort)
		gd->flags &= ~GD_FLG_SILENT;
#endif

	return abort;
}
# endif	/* CONFIG_AUTOBOOT_KEYED */

static int abortboot(int bootdelay)
{
#ifdef CONFIG_AUTOBOOT_KEYED
	return abortboot_keyed(bootdelay);
#else
	return abortboot_normal(bootdelay);
#endif
}

static void process_fdt_options(const void *blob)
{
#if defined(CONFIG_OF_CONTROL)
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = fdtdec_get_config_int(gd->fdt_blob, "kernel-offset", 0);
	if (addr)
		setenv_addr("kernaddr", (void *)(CONFIG_SYS_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = fdtdec_get_config_int(gd->fdt_blob, "rootdisk-offset", 0);
	if (addr)
		setenv_addr("rootaddr", (void *)(CONFIG_SYS_TEXT_BASE + addr));
#endif /* CONFIG_OF_CONTROL */
}

const char *bootdelay_process(void)
{
	char *s;
	int bootdelay;
#ifdef CONFIG_BOOTCOUNT_LIMIT
	unsigned long bootcount = 0;
	unsigned long bootlimit = 0;
#endif /* CONFIG_BOOTCOUNT_LIMIT */

#ifdef CONFIG_BOOTCOUNT_LIMIT
	bootcount = bootcount_load();
	bootcount++;
	bootcount_store(bootcount);
	setenv_ulong("bootcount", bootcount);
	bootlimit = getenv_ulong("bootlimit", 10, 0);
#endif /* CONFIG_BOOTCOUNT_LIMIT */

	s = getenv("bootdelay");
	bootdelay = s ? (int)simple_strtol(s, NULL, 10) : CONFIG_BOOTDELAY;

#ifdef CONFIG_OF_CONTROL
	bootdelay = fdtdec_get_config_int(gd->fdt_blob, "bootdelay",
			bootdelay);
#endif

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

#if defined(CONFIG_MENU_SHOW)
	bootdelay = menu_show(bootdelay);
#endif
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	if (gd->flags & GD_FLG_POSTFAIL) {
		s = getenv("failbootcmd");
	} else
#endif /* CONFIG_POST */
#ifdef CONFIG_BOOTCOUNT_LIMIT
	if (bootlimit && (bootcount > bootlimit)) {
		printf("Warning: Bootlimit (%u) exceeded. Using altbootcmd.\n",
		       (unsigned)bootlimit);
		s = getenv("altbootcmd");
	} else
#endif /* CONFIG_BOOTCOUNT_LIMIT */
        char* boot_cmd_line = getenv("bootcmd");
        char* env = getenv("use_bak_rootfs");

        if(!strcmp(env, "1") && (strstr(boot_cmd_line,"rootfs_bak") == NULL))//replace rootfs with rootfs_bak
        {
            char *str_start = strstr(boot_cmd_line, "rootfs");
            if(str_start)
            {
                int num_start_bytes = str_start - boot_cmd_line;
                int cmd_len = strlen(boot_cmd_line);
                int str1_len = strlen("_bak");
                int str2_len = strlen("rootfs");

                char *tmp_buf = malloc(cmd_len + str1_len);
                strncpy(tmp_buf, boot_cmd_line, num_start_bytes);
                strcpy(tmp_buf + num_start_bytes, "rootfs_bak");
                strncpy(tmp_buf + num_start_bytes + str1_len + str2_len, str_start + str2_len,cmd_len - num_start_bytes - str2_len);
                tmp_buf[cmd_len + str1_len] = '\0';
                setenv("bootcmd", tmp_buf);
                saveenv();
                s = tmp_buf;
            }
        }
        else if(!strcmp(env, "0") && (strstr(boot_cmd_line,"rootfs_bak") != NULL)) //replace rootfs_bak with rootfs
        {
            char *str_start = strstr(boot_cmd_line, "rootfs_bak");
            if(str_start)
            {
                int num_start_bytes = str_start - boot_cmd_line;
                int cmd_len = strlen(boot_cmd_line);
                int str1_len = strlen("_bak");
                int str2_len = strlen("rootfs");
                char *tmp_buf = malloc(cmd_len);

                strncpy(tmp_buf, boot_cmd_line, num_start_bytes + str2_len);
                strncpy(tmp_buf + num_start_bytes + strlen("rootfs"),str_start + str1_len + str2_len,cmd_len - num_start_bytes - str1_len - str2_len);

                tmp_buf[cmd_len - str1_len] = '\0';
                setenv("bootcmd", tmp_buf);
                saveenv();
                s = tmp_buf;
            }
        }
        else
        {
            s = boot_cmd_line;
        }

	process_fdt_options(gd->fdt_blob);
	stored_bootdelay = bootdelay;

	return s;
}

int get_partition_size(char* strENVName)
{
        u32    size;
#if defined(CONFIG_CMD_MTDPARTS) || defined(CONFIG_MS_SPINAND)

        struct mtd_device *dev;
        struct part_info *part;
        u8 pnum;
        int ret;

        ret = mtdparts_init();
        if (ret)
            return ret;

        ret = find_dev_and_part(strENVName, &dev, &pnum, &part);
        if (ret)
        {
            debug("failed to get_partition_size with name: %s\n", strENVName);
            return ret;
        }

        if (dev->id->type != MTD_DEV_TYPE_NAND)
        {
            puts("not a NAND device\n");
            return -1;
        }

        size = part->size;
#elif defined(CONFIG_MS_PARTITION)
        u32 idx;

        mxp_record rec;
        mxp_load_table();
        idx=mxp_get_record_index(strENVName);
        if(idx<0)
        {
            debug("can not found mxp record: %s\n", strENVName);
            return 0;
        }

        if(0 != mxp_get_record_by_index(idx,&rec))
        {
            debug("failed to get_partition_size with name: %s\n", strENVName);
            return 0;
        }
        size = rec.size;
#else
        size = 0x00680000;
#endif
        return size;

}

void autoboot_command(const char *s)
{
    int size = 0;

#if defined(CONFIG_MS_USB) || defined(CONFIG_MS_SDMMC) || defined(CONFIG_MS_EMMC)
    char *env;
#endif

#ifdef CONFIG_SSTAR_IR
    u8 key_value = 0;
#endif

#if defined(CONFIG_MS_MAC_BURN)
    	//当我执行这里时，每次自动更新SD卡固件时，因为这里会先执行，所以出现一个问题:
	//在开机时，烧录了mac地址,但在更新SD卡固件后,mac地址又被刷掉了
	//所以,mac地址不应该在这里烧录,应该在SD卡固件升级最后阶段(set_config),执行macburn
	//run_command("macburn", 0);
#endif


	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	if (stored_bootdelay != -1 && s && !abortboot(stored_bootdelay)) {
#if defined(CONFIG_AUTOBOOT_KEYED) && !defined(CONFIG_AUTOBOOT_KEYED_CTRLC)
		int prev = disable_ctrlc(1);	/* disable Control C checking */
#endif

#ifdef CONFIG_CMD_CUSTOMER_INIT
		run_command("customer_init", 0);
#endif

#ifdef CONFIG_CMD_NETUPGRADE
		run_command("net_upgrade", 0);
#endif
#if defined(CONFIG_MS_USB)
        env = getenv("usbautoupgrade");
        if(!strcmp(env, "1"))
        {
            run_command("setenv usbautoupgrade 2", 0);
            run_command("saveenv",0);
            run_command("usbstar", 0);
        }
        if(!strcmp(env, "2"))
        {
            run_command("setenv usbautoupgrade 1", 0);
            run_command("saveenv",0);
        }

#endif
#if defined(CONFIG_MS_SDMMC)
        env = getenv("sdautoupgrade");
        if(!strcmp(env, "1"))
        {
            run_command("setenv sdautoupgrade 2", 0);
            run_command("saveenv",0);
            run_command("sdstar", 0);
        }
        if(!strcmp(env, "2"))
        {
            run_command("setenv sdautoupgrade 1", 0);
            run_command("saveenv",0);
        }
#endif
#if defined(CONFIG_MS_EMMC)
        env = getenv("emmcautoupgrade");
        if(!strcmp(env, "1"))
        {
            run_command("setenv emmcautoupgrade 2", 0);
            run_command("saveenv",0);
            run_command("emmcstar", 0);
        }
        if(!strcmp(env, "2"))
        {
            run_command("setenv emmcautoupgrade 1", 0);
            run_command("saveenv",0);
        }
#endif

#ifdef CONFIG_SSTAR_IR
        _mdrv_ir_init();

        while(1){
            //mdelay(100);
            if(!_mdrv_get_key(&key_value)){
                if(key_value == START_UP){
                    break;
                }
            }
        }
#endif
		dcache_enable();
		icache_enable();

		run_command_list(s, -1, 0);

		dcache_disable();
		icache_disable();

#if defined(CONFIG_AUTOBOOT_KEYED) && !defined(CONFIG_AUTOBOOT_KEYED_CTRLC)
		disable_ctrlc(prev);	/* restore Control C checking */
#endif
	}


#ifdef CONFIG_MENUKEY
	if (menukey == CONFIG_MENUKEY) {
		s = getenv("menucmd");
		if (s)
			run_command_list(s, -1, 0);
	}
#endif /* CONFIG_MENUKEY */
}
