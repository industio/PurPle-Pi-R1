CONFIG_CMD_FAT=y
CONFIG_SYS_SDRAM_BASE="PHYS_SDRAM_1"
CONFIG_BOOTM_NETBSD=y
CONFIG_HARD_I2C=y
CONFIG_BOOTM_VXWORKS=y
CONFIG_TFTP_PORT=y
CONFIG_CMD_MTDPARTS=y
CONFIG_MS_DISPLAY=y
CONFIG_SYS_MAX_NAND_DEVICE=y
CONFIG_SYS_LONGHELP=y
CONFIG_SYS_LOAD_ADDR="(BOOT_PARAMS+BOOT_PARAMS_LEN+0x4000)"
CONFIG_DISPLAY_BOARDINFO=y
CONFIG_CMD_CACHE=y
CONFIG_STACKSIZE="(4*1024*1024)"
CONFIG_BOOTDELAY=0
CONFIG_SYS_NAND_BASE=0
CONFIG_SYS_HELP_CMD_WIDTH=8
CONFIG_NR_DRAM_BANKS=y
CONFIG_FS_FAT=y
CONFIG_BOOTM_RTEMS=y
CONFIG_SYS_CBSIZE=1024
CONFIG_SKIP_LOWLEVEL_INIT=y
CONFIG_CMD_UBI=y
CONFIG_ENV_RANGE=0x20000
CONFIG_BOOTM_LINUX=y
CONFIG_BOARD_LATE_INIT=y
CONFIG_BOOTP_GATEWAY=y
CONFIG_MMC=y
CONFIG_MTD_PARTITIONS=y
CONFIG_MSTAR_ENV_NAND_OFFSET="ms_nand_env_offset"
CONFIG_ZERO_BOOTDELAY_CHECK=y
CONFIG_ENV_OFFSET=$(CONFIG_MSTAR_ENV_NAND_OFFSET)
CONFIG_MS_SHOW_LOGO=y
CONFIG_DM_DEVICE_REMOVE=y
CONFIG_ENV_SIZE=0x1000
CONFIG_CMD_SPINAND_CIS=y
CONFIG_SYS_MALLOC_LEN="(CONFIG_ENV_SIZE + 4*1024*1024)"
CONFIG_INITRD_TAG=y
CONFIG_ARCH_INFINITY2M=y
CONFIG_SYS_HZ_CLOCK=400000000
CONFIG_SYS_TEXT_BASE=0x23E00000
CONFIG_MS_PIUTIMER=y
CONFIG_CMD_SAVEENV=y
CONFIG_SYS_BARGSIZE=$(CONFIG_SYS_CBSIZE)
CONFIG_BOOTM_PLAN9=y
CONFIG_I2C_MULTI_BUS=y
CONFIG_SYS_I2C_MS_SLAVE="(0x7F)"
CONFIG_CMD_NAND=y
CONFIG_AUTOBOOT_DELAY_STR="x0d"
CONFIG_DM_WARN=y
CONFIG_CMD_MEMORY=y
CONFIG_SYS_MAXARGS=64
CONFIG_CMD_RUN=y
CONFIG_SYS_PBSIZE="(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)"
CONFIG_BOARDDIR="board/"
CONFIG_MINIUBOOT=y
CONFIG_CMDLINE_EDITING=y
CONFIG_CMD_USB=y
CONFIG_ENV_OFFSET_REDUND=$(CONFIG_MSTAR_ENV_NAND_REDUND_OFFSET)
CONFIG_UART_CLOCK=172800000
CONFIG_SYS_L2CACHE_OFF=y
CONFIG_CMD_EEPROM=y
CONFIG_SYS_NAND_MAX_CHIPS=y
CONFIG_ZLIB=y
CONFIG_BOOT_TIME_ANALYSIS=0
CONFIG_AUTO_COMPLETE=y
CONFIG_MZ=y
CONFIG_DOS_PARTITION=y
CONFIG_GZIP=y
CONFIG_CMD_SF=y
CONFIG_XZ=y
CONFIG_SYS_BAUDRATE_TABLE="{ 9600, 19200, 38400, 57600, 115200 }"
CONFIG_DM_STDIO=y
CONFIG_UBI_MWRITE=y
CONFIG_IMAGE_FORMAT_LEGACY=y
CONFIG_SYS_I2C_1_PADMUX="(0x1)"
CONFIG_ENV_IS_IN_NAND=y
CONFIG_GENERIC_MMC=y
CONFIG_WDT_CLOCK=$(CONFIG_PIUTIMER_CLOCK)
CONFIG_FAT_WRITE=y
CONFIG_MSTAR_ENV_NAND_REDUND_OFFSET="ms_nand_env_redund_offset"
CONFIG_SYS_I2C=y
CONFIG_MS_PIU_TIMER=y
CONFIG_EXTRA_ENV_SETTINGS="mtdids=" MTDIDS_DEFAULT "0mtdparts=" MTDPARTS_DEFAULT "0partition=nand0,00"
CONFIG_MS_ISP=y
CONFIG_AUTOSCRIPT=y
CONFIG_SYS_INIT_SP_ADDR="(CONFIG_SYS_TEXT_BASE - GENERATED_GBL_DATA_SIZE)"
CONFIG_HW_WATCHDOG_TIMEOUT_S=60
CONFIG_BAUDRATE=115200
CONFIG_SYS_I2C_EEPROM_ADDR_LEN="(2)"
CONFIG_CMDLINE_TAG=y
CONFIG_SYS_BOOT_RAMDISK_HIGH=y
CONFIG_PARTITIONS=y
CONFIG_SYS_MEMTEST_END=0x24000000
CONFIG_CMD_I2C=y
CONFIG_SYS_I2C_MS_SPEED="(300000)"
CONFIG_SYS_NO_FLASH=y
CONFIG_SYS_DEF_EEPROM_ADDR=$(CONFIG_SYS_I2C_EEPROM_ADDR)
CONFIG_SYS_I2C_0_PADMUX="(0x1)"
CONFIG_MTD_DEVICE=y
CONFIG_MS_SDMMC_MAX_READ_BLOCKS=1024
CONFIG_SYS_PROMPT="SigmaStar # "
CONFIG_MS_SRCFG=y
CONFIG_SYS_REDUNDAND_ENVIRONMENT=y
CONFIG_SETUP_MEMORY_TAGS=y
CONFIG_SYS_I2C_EEPROM_ADDR="(0x54)"
CONFIG_SYS_MEMTEST_START=0x20000000
CONFIG_PIUTIMER_CLOCK=12000000
CONFIG_SYS_I2C_MSTAR=y
CONFIG_LMB=y
CONFIG_RBTREE=y
CONFIG_AUTOBOOT_KEYED=y
CONFIG_BOOTSTAGE_USER_COUNT=20
CONFIG_BOOT_TIME_ANALYSIS_USE_RTC=0
CONFIG_UBOOT_RAM_SIZE=0x04000000
CONFIG_CMD_MMC=y
CONFIG_ARMCORTEXA7=y
