#define FLAG_BYTE               0x7E
enum CMD_TYPE
{
	BSL_PKT_TYPE_MIN, /* the bottom of the DL packet type range */

	/* Link Control */
	BSL_CMD_CONNECT = BSL_PKT_TYPE_MIN,   /* 0x00

	/* Data Download */
	BSL_CMD_START_DATA,   /* 0x01 the start flag of the data downloading  */
	BSL_CMD_MIDST_DATA,   /* 0x02 the midst flag of the data downloading  */
	BSL_CMD_END_DATA, /* 0x03 the end flag of the data downloading */
	BSL_CMD_EXEC_DATA,    /* 0x04 Execute from a certain address */
	/* End of Data Download command */

	BSL_CMD_NORMAL_RESET, /* 0x05 Reset to normal mode */
	BSL_CMD_READ_FLASH,   /* 0x06 Read flash content */
	BSL_CMD_READ_CHIP_TYPE,   /* 0x07 Read chip type */
	BSL_CMD_READ_NVITEM,  /* 0x08 Lookup a nvitem in specified area */
	BSL_CMD_CHANGE_BAUD,  /* 0x09 Change baudrate */
	BSL_CMD_ERASE_FLASH,  /* 0x0A Erase an area of flash */
	BSL_CMD_REPARTITION,  /* 0x0B Repartition nand flash */
	BSL_CMD_READ_FLASH_TYPE,  /* 0x0C Read flash type */
	BSL_CMD_READ_FLASH_INFO,  /* 0x0D Read flash infomation */
	BSL_CMD_READ_SECTOR_SIZE = 0xF,   /* 0x0F Read Nor flash sector size */
	BSL_CMD_READ_START = 0x10,    /* 0x10 Read flash start */
	BSL_CMD_READ_MIDST,   /* 0x11 Read flash midst */
	BSL_CMD_READ_END, /* 0x12 Read flash end */

	BSL_CMD_KEEP_CHARGE = 0x13,   /* keep charge */
	BSL_CMD_EXTTABLE = 0x14,  /* 0x14 Set ExtTable */
	BSL_CMD_READ_FLASH_UID = 0x15,    /* 0x15 Read flash UID */
	BSL_CMD_READ_SOFTSIM_EID = 0x16,  /* 0x16 Read softSIM EID */
	BSL_CMD_POWER_OFF = 0x17, /* 0x17 Power Off */
	BSL_CMD_CHECK_ROOT = 0x19,    /* 0x19 Check Root */
	BSL_CMD_READ_CHIP_UID = 0x1A, /* 0x1A Read Chip UID */
	BSL_CMD_ENABLE_WRITE_FLASH = 0x1B,    /* 0x1B Enable flash */
	BSL_CMD_ENABLE_SECUREBOOT = 0x1C, /* 0x1C Enable secure boot */
	BSL_CMD_READ_RF_TRANSCEIVER_TYPE = 0x24,  /* 0x24 Read RF transceiver type */

	BSL_CMD_CHECK_BAUD = FLAG_BYTE,   /* CheckBaud command,for internal use */
	BSL_CMD_END_PROCESS = 0x7F,   /* 0x7F End flash process */
	/* End of the Command can be received by phone */

	/* Start of the Command can be transmited by phone */
	BSL_REP_TYPE_MIN = 0x80,

	BSL_REP_ACK = BSL_REP_TYPE_MIN,   /* The operation acknowledge */
	BSL_REP_VER,      /* 0x81 */

	/* the operation not acknowledge */
	/* system  */
	BSL_REP_INVALID_CMD,  /* 0x82 */
	BSL_REP_UNKNOW_CMD,   /* 0x83 */
	BSL_REP_OPERATION_FAILED, /* 0x84 */

	/* Link Control */
	BSL_REP_NOT_SUPPORT_BAUDRATE, /* 0x85 */

	/* Data Download */
	BSL_REP_DOWN_NOT_START,   /* 0x86 */
	BSL_REP_DOWN_MULTI_START, /* 0x87 */
	BSL_REP_DOWN_EARLY_END,   /* 0x88 */
	BSL_REP_DOWN_DEST_ERROR,  /* 0x89 */
	BSL_REP_DOWN_SIZE_ERROR,  /* 0x8A */
	BSL_REP_VERIFY_ERROR, /* 0x8B */
	BSL_REP_NOT_VERIFY,   /* 0x8C */

	/* Phone Internal Error */
	BSL_PHONE_NOT_ENOUGH_MEMORY,  /* 0x8D */
	BSL_PHONE_WAIT_INPUT_TIMEOUT, /* 0x8E */

	/* Phone Internal return value */
	BSL_PHONE_SUCCEED,    /* 0x8F */
	BSL_PHONE_VALID_BAUDRATE, /* 0x90 */
	BSL_PHONE_REPEAT_CONTINUE,    /* 0x91 */
	BSL_PHONE_REPEAT_BREAK,   /* 0x92 */

	/* End of the Command can be transmited by phone */
	BSL_REP_READ_FLASH,   /* 0x93 */
	BSL_REP_READ_CHIP_TYPE,   /* 0x94 */
	BSL_REP_READ_NVITEM,  /* 0x95 */

	BSL_REP_INCOMPATIBLE_PARTITION,   /* 0x96 */
	BSL_REP_UNKNOWN_DEVICE,   /* 0x97 */
	BSL_REP_INVALID_DEVICE_SIZE,  /* 0x98 */
	BSL_REP_ILLEGAL_SDRAM,    /* 0x99 */
	BSL_WRONG_SDRAM_PARAMETER,    /* 0x9A */
	BSL_REP_READ_FLASH_INFO,  /* 0x9B */
	BSL_REP_READ_SECTOR_SIZE = 0x9C,  /* 0x9C */
	BSL_REP_READ_FLASH_TYPE = 0x9D,   /* 0x9D */
	BSL_REP_READ_FLASH_UID = 0x9E,    /* 0x9E */
	BSL_REP_READ_SOFTSIM_EID = 0x9F,  /* 0x9F */

	/* information returned from FDL when downloading fixed NV */
	BSL_ERROR_CHECKSUM = 0xA0,
	BSL_CHECKSUM_DIFF = 0xA1,
	BSL_WRITE_ERROR = 0xA2,
	BSL_CHIPID_NOT_MATCH = 0xA3,
	BSL_FLASH_CFG_ERROR = 0xA4,
	BSL_REP_DOWN_STL_SIZE_ERROR = 0xA5,
	BSL_REP_PHONE_IS_ROOTED = 0xA7,   /* 0xA7 Phone has been rooted */
	BSL_REP_SEC_VERIFY_ERROR = 0xAA,  /* 0xAA Security data verify error */
	BSL_REP_READ_CHIP_UID = 0xAB, /* 0xAB Received Chip UID */
	BSL_REP_NOT_ENABLE_WRITE_FLASH = 0xAC,    /* 0xAC Not enable to write flash */
	BSL_REP_ENABLE_SECUREBOOT_ERROR = 0xAD,   /* 0xAD Enable secure boot fail */
	BSL_REP_FLASH_WRITTEN_PROTECTION = 0xB3,  /* 0xB3 Flash written protection */
	BSL_REP_FLASH_INITIALIZING_FAIL = 0xB4,   /* 0xB4 Flash initializing failed */
	BSL_REP_RF_TRANSCEIVER_TYPE = 0xB5,   /* 0xB5 RF transceiver type */

	BSL_REP_UNSUPPROT_COMMAND = 0xFE, /* 0xFE Software has not supported this feature */
	BSL_REP_LOG = 0xFF,   //FDL can output some log info use this type
	BSL_PKT_TYPE_MAX = 0x100,

	BSL_UART_SEND_ERROR,
	BSL_REP_DECODE_ERROR, // Decode or verify received buffer error
	BSL_REP_INCOMPLETE_DATA,  // Received buffer is not in the format we want
	BSL_REP_READ_ERROR,
	BSL_REP_TOO_MUCH_DATA,
	BSL_USER_CANCEL,
	BSL_REP_SIZE_ZERO,
	BSL_REP_PORT_ERROR
};

extern int cmd_connect(int timeout);
extern int cmd_check_bandrate(int timeout);
