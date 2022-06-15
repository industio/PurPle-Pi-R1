/*
 * (C) Copyright 2002-2008
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* Pull in the current config to define the default environment */
#ifndef __ASSEMBLY__
#define __ASSEMBLY__ /* get only #defines from config.h */
#include <config.h>
#undef	__ASSEMBLY__
#else
#include <config.h>
#endif
#include <stdint.h>
/*
 * To build the utility with the static configuration
 * comment out the next line.
 * See included "fw_env.config" sample file
 * for notes on configuration.
 */
//#define CONFIG_MSTAR_NAND
#define CONFIG_MSTAR_NOR

#if defined(CONFIG_MSTAR_NAND)||defined(CONFIG_MSTAR_NOR)
#define CONFIG_FILE     "./fw_env.config"
#else
#define CONFIG_FILE     "/etc/fw_env.config"
#endif

#ifndef CONFIG_FILE
#if defined( CONFIG_MSTAR_NAND)
#define HAVE_REDUND /* For systems with 2 env sectors */
#define DEVICE1_NAME      "/dev/mtd6"
#define DEVICE2_NAME      "/dev/mtd7"
#define DEVICE1_OFFSET    0x0000
#define ENV1_SIZE         0x1000
#define DEVICE1_ESIZE     0x20000
#define DEVICE1_ENVSECTORS     1
#define DEVICE2_OFFSET    0x0000
#define ENV2_SIZE         0x1000
#define DEVICE2_ESIZE     0x20000
#define DEVICE2_ENVSECTORS     1
#elif defined(CONFIG_MSTAR_NOR)
#define HAVE_REDUND /* For systems with 2 env sectors */
#define DEVICE1_NAME      "/dev/mtd0"
#define DEVICE2_NAME      "/dev/mtd0"
#define DEVICE1_OFFSET    0x6E000
#define ENV1_SIZE         0x1000
#define DEVICE1_ESIZE     0x1000
#define DEVICE1_ENVSECTORS     1
#define DEVICE2_OFFSET    0x6F000
#define ENV2_SIZE         0x1000
#define DEVICE2_ESIZE     0x1000
#define DEVICE2_ENVSECTORS     1
#else
#define HAVE_REDUND /* For systems with 2 env sectors */
#define DEVICE1_NAME      "/dev/mtd1"
#define DEVICE2_NAME      "/dev/mtd2"
#define DEVICE1_OFFSET    0x0000
#define ENV1_SIZE         0x4000
#define DEVICE1_ESIZE     0x4000
#define DEVICE1_ENVSECTORS     2
#define DEVICE2_OFFSET    0x0000
#define ENV2_SIZE         0x4000
#define DEVICE2_ESIZE     0x4000
#define DEVICE2_ENVSECTORS     2
#endif
#endif

#ifndef CONFIG_BAUDRATE
#define CONFIG_BAUDRATE		115200
#endif

#ifndef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#ifndef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND							\
	"bootp; "								\
	"setenv bootargs root=/dev/nfs nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; "	\
	"bootm"
#endif

extern int   fw_printenv(int argc, char *argv[]);
extern char *fw_getenv  (char *name);
extern int fw_setenv  (int argc, char *argv[]);
extern int fw_parse_script(char *fname);
extern int fw_env_open(void);
extern int fw_env_write(char *name, char *value);
extern int fw_env_close(void);

extern unsigned	long  crc32	 (unsigned long, const unsigned char *, unsigned);
