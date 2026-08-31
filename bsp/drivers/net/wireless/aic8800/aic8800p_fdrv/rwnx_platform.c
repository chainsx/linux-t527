/**
 ******************************************************************************
 *
 * @file rwnx_platform.c
 *
 * Copyright (C) RivieraWaves 2012-2019
 *
 ******************************************************************************
 */

#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/kthread.h>

#include "rwnx_platform.h"
#include "reg_access.h"
#include "hal_desc.h"
#include "rwnx_main.h"
#include "rwnx_pci.h"
#ifndef CONFIG_RWNX_FHOST
#include "ipc_host.h"
#endif /* !CONFIG_RWNX_FHOST */
#include "rwnx_msg_tx.h"

#ifdef AICWF_SDIO_SUPPORT
#include "aicwf_sdio.h"
#endif

#ifdef AICWF_USB_SUPPORT
#include "aicwf_usb.h"
#endif
#include "aicwf_compat_8800dc.h"
#include "aicwf_compat_8800d80.h"
#include "aicwf_compat_8800d80x2.h"
#include "aic_priv_cmd.h"

#ifdef CONFIG_USE_FW_REQUEST
#include <linux/firmware.h>
#endif

extern int testmode;
extern int fw_flsupg;
extern int fw_flggen1;
extern int fw_flggen2;

struct rwnx_plat *g_rwnx_plat;

typedef struct {
	char ccode[3];
	Regions_code region;
} reg_table;

/* If the region conflicts with the kernel, the actual authentication standard prevails */
reg_table reg_tables[] = {
	{.ccode = "CN", .region = REGIONS_SRRC},
	{.ccode = "US", .region = REGIONS_FCC},
	{.ccode = "DE", .region = REGIONS_ETSI},
	{.ccode = "00", .region = REGIONS_DEFAULT},
	{.ccode = "WW", .region = REGIONS_DEFAULT},
	{.ccode = "XX", .region = REGIONS_DEFAULT},
	{.ccode = "JP", .region = REGIONS_JP},
	{.ccode = "AD", .region = REGIONS_ETSI},
	{.ccode = "AE", .region = REGIONS_ETSI},
	{.ccode = "AF", .region = REGIONS_ETSI},
	{.ccode = "AI", .region = REGIONS_ETSI},
	{.ccode = "AL", .region = REGIONS_ETSI},
	{.ccode = "AM", .region = REGIONS_ETSI},
	{.ccode = "AN", .region = REGIONS_ETSI},
	{.ccode = "AR", .region = REGIONS_FCC},
	{.ccode = "AS", .region = REGIONS_FCC},
	{.ccode = "AT", .region = REGIONS_ETSI},
	{.ccode = "AU", .region = REGIONS_ETSI},
	{.ccode = "AW", .region = REGIONS_ETSI},
	{.ccode = "AZ", .region = REGIONS_ETSI},
	{.ccode = "BA", .region = REGIONS_ETSI},
	{.ccode = "BB", .region = REGIONS_FCC},
	{.ccode = "BD", .region = REGIONS_JP},
	{.ccode = "BE", .region = REGIONS_ETSI},
	{.ccode = "BF", .region = REGIONS_FCC},
	{.ccode = "BG", .region = REGIONS_ETSI},
	{.ccode = "BH", .region = REGIONS_ETSI},
	{.ccode = "BL", .region = REGIONS_ETSI},
	{.ccode = "BM", .region = REGIONS_FCC},
	{.ccode = "BN", .region = REGIONS_JP},
	{.ccode = "BO", .region = REGIONS_JP},
	{.ccode = "BR", .region = REGIONS_FCC},
	{.ccode = "BS", .region = REGIONS_FCC},
	{.ccode = "BT", .region = REGIONS_ETSI},
	{.ccode = "BW", .region = REGIONS_ETSI},
	{.ccode = "BY", .region = REGIONS_ETSI},
	{.ccode = "BZ", .region = REGIONS_JP},
	{.ccode = "CA", .region = REGIONS_FCC},
	{.ccode = "CF", .region = REGIONS_FCC},
	{.ccode = "CH", .region = REGIONS_ETSI},
	{.ccode = "CI", .region = REGIONS_FCC},
	{.ccode = "CL", .region = REGIONS_ETSI},
	{.ccode = "CO", .region = REGIONS_FCC},
	{.ccode = "CR", .region = REGIONS_FCC},
	{.ccode = "CU", .region = REGIONS_FCC},
	{.ccode = "CX", .region = REGIONS_FCC},
	{.ccode = "CY", .region = REGIONS_ETSI},
	{.ccode = "CZ", .region = REGIONS_ETSI},
	{.ccode = "DK", .region = REGIONS_ETSI},
	{.ccode = "DM", .region = REGIONS_FCC},
	{.ccode = "DO", .region = REGIONS_FCC},
	{.ccode = "DZ", .region = REGIONS_JP},
	{.ccode = "EC", .region = REGIONS_FCC},
	{.ccode = "EE", .region = REGIONS_ETSI},
	{.ccode = "EG", .region = REGIONS_ETSI},
	{.ccode = "ES", .region = REGIONS_ETSI},
	{.ccode = "ET", .region = REGIONS_ETSI},
	{.ccode = "FI", .region = REGIONS_ETSI},
	{.ccode = "FM", .region = REGIONS_FCC},
	{.ccode = "FR", .region = REGIONS_ETSI},
	{.ccode = "GB", .region = REGIONS_ETSI},
	{.ccode = "GD", .region = REGIONS_FCC},
	{.ccode = "GE", .region = REGIONS_ETSI},
	{.ccode = "GF", .region = REGIONS_ETSI},
	{.ccode = "GH", .region = REGIONS_FCC},
	{.ccode = "GI", .region = REGIONS_ETSI},
	{.ccode = "GL", .region = REGIONS_ETSI},
	{.ccode = "GP", .region = REGIONS_ETSI},
	{.ccode = "GR", .region = REGIONS_ETSI},
	{.ccode = "GT", .region = REGIONS_DEFAULT},
	{.ccode = "GU", .region = REGIONS_FCC},
	{.ccode = "GY", .region = REGIONS_DEFAULT},
	{.ccode = "HK", .region = REGIONS_ETSI},
	{.ccode = "HN", .region = REGIONS_FCC},
	{.ccode = "HR", .region = REGIONS_ETSI},
	{.ccode = "HT", .region = REGIONS_FCC},
	{.ccode = "HU", .region = REGIONS_ETSI},
	{.ccode = "ID", .region = REGIONS_ETSI},
	{.ccode = "IE", .region = REGIONS_ETSI},
	{.ccode = "IL", .region = REGIONS_ETSI},
	{.ccode = "IN", .region = REGIONS_ETSI},
	{.ccode = "IQ", .region = REGIONS_ETSI},
	{.ccode = "IR", .region = REGIONS_ETSI},
	{.ccode = "IS", .region = REGIONS_ETSI},
	{.ccode = "IT", .region = REGIONS_ETSI},
	{.ccode = "JM", .region = REGIONS_FCC},
	{.ccode = "JO", .region = REGIONS_ETSI},
	{.ccode = "KE", .region = REGIONS_ETSI},
	{.ccode = "KG", .region = REGIONS_ETSI},
	{.ccode = "KH", .region = REGIONS_ETSI},
	{.ccode = "KN", .region = REGIONS_ETSI},
	{.ccode = "KP", .region = REGIONS_JP},
	{.ccode = "KR", .region = REGIONS_KCC},
	{.ccode = "KW", .region = REGIONS_ETSI},
	{.ccode = "KY", .region = REGIONS_FCC},
	{.ccode = "KZ", .region = REGIONS_DEFAULT},
	{.ccode = "LB", .region = REGIONS_ETSI},
	{.ccode = "LC", .region = REGIONS_ETSI},
	{.ccode = "LI", .region = REGIONS_ETSI},
	{.ccode = "LK", .region = REGIONS_FCC},
	{.ccode = "LS", .region = REGIONS_ETSI},
	{.ccode = "LT", .region = REGIONS_ETSI},
	{.ccode = "LU", .region = REGIONS_ETSI},
	{.ccode = "LV", .region = REGIONS_ETSI},
	{.ccode = "LY", .region = REGIONS_ETSI},
	{.ccode = "MA", .region = REGIONS_ETSI},
	{.ccode = "MC", .region = REGIONS_ETSI},
	{.ccode = "MD", .region = REGIONS_ETSI},
	{.ccode = "ME", .region = REGIONS_ETSI},
	{.ccode = "MF", .region = REGIONS_ETSI},
	{.ccode = "MH", .region = REGIONS_FCC},
	{.ccode = "MK", .region = REGIONS_ETSI},
	{.ccode = "MN", .region = REGIONS_ETSI},
	{.ccode = "MO", .region = REGIONS_ETSI},
	{.ccode = "MP", .region = REGIONS_FCC},
	{.ccode = "MQ", .region = REGIONS_ETSI},
	{.ccode = "MR", .region = REGIONS_ETSI},
	{.ccode = "MT", .region = REGIONS_ETSI},
	{.ccode = "MU", .region = REGIONS_FCC},
	{.ccode = "MV", .region = REGIONS_ETSI},
	{.ccode = "MW", .region = REGIONS_ETSI},
	{.ccode = "MX", .region = REGIONS_FCC},
	{.ccode = "MY", .region = REGIONS_FCC},
	{.ccode = "NA", .region = REGIONS_ETSI},
	{.ccode = "NG", .region = REGIONS_ETSI},
	{.ccode = "NI", .region = REGIONS_FCC},
	{.ccode = "NL", .region = REGIONS_ETSI},
	{.ccode = "NO", .region = REGIONS_ETSI},
	{.ccode = "NP", .region = REGIONS_JP},
	{.ccode = "NZ", .region = REGIONS_ETSI},
	{.ccode = "OM", .region = REGIONS_ETSI},
	{.ccode = "PA", .region = REGIONS_FCC},
	{.ccode = "PE", .region = REGIONS_FCC},
	{.ccode = "PF", .region = REGIONS_ETSI},
	{.ccode = "PG", .region = REGIONS_FCC},
	{.ccode = "PH", .region = REGIONS_FCC},
	{.ccode = "PK", .region = REGIONS_DEFAULT},
	{.ccode = "PL", .region = REGIONS_ETSI},
	{.ccode = "PM", .region = REGIONS_ETSI},
	{.ccode = "PR", .region = REGIONS_FCC},
	{.ccode = "PT", .region = REGIONS_ETSI},
	{.ccode = "PW", .region = REGIONS_FCC},
	{.ccode = "PY", .region = REGIONS_FCC},
	{.ccode = "QA", .region = REGIONS_ETSI},
	{.ccode = "RE", .region = REGIONS_ETSI},
	{.ccode = "RO", .region = REGIONS_ETSI},
	{.ccode = "RS", .region = REGIONS_ETSI},
	{.ccode = "RU", .region = REGIONS_ETSI},
	{.ccode = "RW", .region = REGIONS_FCC},
	{.ccode = "SA", .region = REGIONS_ETSI},
	{.ccode = "SE", .region = REGIONS_ETSI},
	{.ccode = "SG", .region = REGIONS_ETSI},
	{.ccode = "SI", .region = REGIONS_ETSI},
	{.ccode = "SK", .region = REGIONS_ETSI},
	{.ccode = "SM", .region = REGIONS_ETSI},
	{.ccode = "SN", .region = REGIONS_FCC},
	{.ccode = "SR", .region = REGIONS_ETSI},
	{.ccode = "SV", .region = REGIONS_FCC},
	{.ccode = "SY", .region = REGIONS_ETSI},
	{.ccode = "TC", .region = REGIONS_FCC},
	{.ccode = "TD", .region = REGIONS_ETSI},
	{.ccode = "TG", .region = REGIONS_ETSI},
	{.ccode = "TH", .region = REGIONS_FCC},
	{.ccode = "TJ", .region = REGIONS_ETSI},
	{.ccode = "TM", .region = REGIONS_ETSI},
	{.ccode = "TN", .region = REGIONS_ETSI},
	{.ccode = "TR", .region = REGIONS_ETSI},
	{.ccode = "TT", .region = REGIONS_FCC},
	{.ccode = "TW", .region = REGIONS_FCC},
	{.ccode = "TZ", .region = REGIONS_ETSI},
	{.ccode = "UA", .region = REGIONS_ETSI},
	{.ccode = "UG", .region = REGIONS_FCC},
	{.ccode = "UY", .region = REGIONS_FCC},
	{.ccode = "UZ", .region = REGIONS_ETSI},
	{.ccode = "VC", .region = REGIONS_ETSI},
	{.ccode = "VE", .region = REGIONS_FCC},
	{.ccode = "VI", .region = REGIONS_FCC},
	{.ccode = "VN", .region = REGIONS_JP},
	{.ccode = "VU", .region = REGIONS_FCC},
	{.ccode = "WF", .region = REGIONS_ETSI},
	{.ccode = "WS", .region = REGIONS_ETSI},
	{.ccode = "YE", .region = REGIONS_DEFAULT},
	{.ccode = "YT", .region = REGIONS_ETSI},
	{.ccode = "ZA", .region = REGIONS_ETSI},
	{.ccode = "ZM", .region = REGIONS_ETSI},
	{.ccode = "FO", .region = REGIONS_ETSI},
	{.ccode = "FK", .region = REGIONS_ETSI},
	{.ccode = "ZW", .region = REGIONS_ETSI},
};

uint8_t get_ccode_region(const char *ccode)
{
	int i, cnt;

	cnt = sizeof(reg_tables) / sizeof(reg_tables[0]);

	for (i = 0; i < cnt; i++) {
		if (reg_tables[i].ccode[0] == ccode[0] &&
			reg_tables[i].ccode[1] == ccode[1]) {
			return reg_tables[i].region;
		}
	}
	AICWFDBG(LOGDEBUG, "use default region\r\n");
	return REGIONS_DEFAULT;
}

u8 get_region_index(char *name)
{
	if (strncmp(name, "SRRC", 4) == 0)
		return REGIONS_SRRC;
	else if (strncmp(name, "FCC", 3) == 0)
		return REGIONS_FCC;
	else if (strncmp(name, "ETSI", 4) == 0)
		return REGIONS_ETSI;
	else if (strncmp(name, "JP", 2) == 0)
		return REGIONS_JP;
	else if (strncmp(name, "KCC", 3) == 0)
		return REGIONS_KCC;
	else if (strncmp(name, "UNSET", 5) == 0)
		return REGIONS_DEFAULT;

	return REGIONS_DEFAULT;
}

#ifdef CONFIG_POWER_LIMIT
#define POWER_LIMIT_INVALID_VAL     POWER_LEVEL_INVALID_VAL

#define POWER_LIMIT_CC_MATCHED_BIT  (0x1U << 0)

#define MAX_2_4G_BW_NUM    2
#define MAX_5G_BW_NUM      3
#define MAX_REGION_NUM     6

typedef struct {
	u8_l ch_cnt_2g4[MAX_2_4G_BW_NUM];
	u8_l ch_cnt_5g[MAX_5G_BW_NUM];
	u8_l ch_num_2g4[MAX_2_4G_BW_NUM][MAC_DOMAINCHANNEL_24G_MAX];
	u8_l ch_num_5g[MAX_5G_BW_NUM][MAC_DOMAINCHANNEL_5G_MAX];
	s8_l max_pwr_2g4[MAX_2_4G_BW_NUM][MAC_DOMAINCHANNEL_24G_MAX];
	s8_l max_pwr_5g[MAX_5G_BW_NUM][MAC_DOMAINCHANNEL_5G_MAX];
} txpwr_lmt_info_t;

typedef struct {
	u32_l flags;
	txpwr_lmt_info_t txpwr_lmt[MAX_REGION_NUM];
} powerlimit_info_t;

powerlimit_info_t powerlimit_info = {0,};
#endif

typedef u32 (*array2_tbl_t)[2];


#ifdef CONFIG_RWNX_TL4
/**
 * rwnx_plat_tl4_fw_upload() - Load the requested FW into embedded side.
 *
 * @rwnx_plat: pointer to platform structure
 * @fw_addr: Virtual address where the fw must be loaded
 * @filename: Name of the fw.
 *
 * Load a fw, stored as a hex file, into the specified address
 */
static int rwnx_plat_tl4_fw_upload(struct rwnx_plat *rwnx_plat, u8 *fw_addr,
								   char *filename)
{
	struct device *dev = rwnx_platform_get_dev(rwnx_plat);
	const struct firmware *fw;
	int err = 0;
	u32 *dst;
	u8 const *file_data;
	char typ0, typ1;
	u32 addr0, addr1;
	u32 dat0, dat1;
	int remain;

	err = request_firmware(&fw, filename, dev);
	if (err) {
		return err;
	}
	file_data = fw->data;
	remain = fw->size;

	/* Copy the file on the Embedded side */
	dev_dbg(dev, "\n### Now copy %s firmware, @ = %p\n", filename, fw_addr);

	/* Walk through all the lines of the configuration file */
	while (remain >= 16) {
		u32 data, offset;

		if (sscanf(file_data, "%c:%08X %04X", &typ0, &addr0, &dat0) != 3)
			break;
		if ((addr0 & 0x01) != 0) {
			addr0 = addr0 - 1;
			dat0 = 0;
		} else {
			file_data += 16;
			remain -= 16;
		}
		if ((remain < 16) ||
			(sscanf(file_data, "%c:%08X %04X", &typ1, &addr1, &dat1) != 3) ||
			(typ1 != typ0) || (addr1 != (addr0 + 1))) {
			typ1 = typ0;
			addr1 = addr0 + 1;
			dat1 = 0;
		} else {
			file_data += 16;
			remain -= 16;
		}

		if (typ0 == 'C') {
			offset = 0x00200000;
			if ((addr1 % 4) == 3)
				offset += 2*(addr1 - 3);
			else
				offset += 2*(addr1 + 1);

			data = dat1 | (dat0 << 16);
		} else {
			offset = 2*(addr1 - 1);
			data = dat0 | (dat1 << 16);
		}
		dst = (u32 *)(fw_addr + offset);
		*dst = data;
	}

	release_firmware(fw);

	return err;
}
#endif

static int pcie_send_dbg_mem_write_req(struct rwnx_hw *rwnx_hw, u32 mem_addr, u32 mem_data)
{
	int ret = 0;
	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80) {
		volatile u32 *write_addr;
		volatile u32 *write_data = &mem_data;
		int bar_index;

		if (mem_addr < 0x40000000) {
			write_addr = (volatile u32 *)(rwnx_hw->pcidev->pci_bar0_vaddr + mem_addr - 0);
			bar_index = 0;
		} else if (mem_addr < 0x40700000) {
			write_addr = (volatile u32 *)(rwnx_hw->pcidev->pci_bar1_vaddr + mem_addr - 0x40000000);
			bar_index = 1;
		} else {
			write_addr = (volatile u32 *)(rwnx_hw->pcidev->pci_bar2_vaddr + mem_addr - 0x40700000);
			bar_index = 2;
		}
		*write_addr = *write_data;
	} else if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2) {
		u8  rem = 1;
		ret = aicwf_pcie_tran(rwnx_hw->pcidev, (void *)mem_addr, &mem_data, 4, AIC_TRAN_DRV2EMB, rem);
		if (ret != 0) {
			return ret;
		}
	}
	return ret;
}

static int pcie_send_dbg_mem_block_write_req(struct rwnx_hw *rwnx_hw, u32 mem_addr,
									  u32 mem_size, u32 *mem_data)
{
	int err = 0;
	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80) {
		volatile u32 *write_addr = (volatile u32 *)(rwnx_hw->pcidev->pci_bar0_vaddr + mem_addr);
		volatile u32 *write_data = mem_data;
		int i = 0;
		for (i = 0; i < mem_size; i += 4) {
			*write_addr++ = *write_data++;
		}
	} else {
		u8 rem = 1;
		err = aicwf_pcie_tran(rwnx_hw->pcidev, (void *)mem_addr, mem_data, mem_size, AIC_TRAN_DRV2EMB, rem);
	}
	return err;
}

/**
 * pcie_plat_bin_fw_upload_android() - Load the requested binary FW into embedded side.
 *
 * @rwnx_hw: Main driver data
 * @fw_addr: Address where the fw must be loaded
 * @filename: Name of the fw.
 *
 * Load a fw, stored as a binary file, into the specified address
 */
int pcie_plat_bin_fw_upload_android(struct rwnx_hw *rwnx_hw, u32 fw_addr,
							   const char *filename)
{
	unsigned int i = 0;
	int size;
	u32 *dst = NULL;
	int err = 0;

	const struct firmware *fw = NULL;
	int ret = request_firmware(&fw, filename, NULL);

	AICWFDBG(LOGINFO, "rwnx_request_firmware, name: %s\n", filename);
	if (ret < 0) {
		AICWFDBG(LOGERROR, "Load %s fail\n", filename);
		return ret;
	}

	size = fw->size;
	dst = (u32 *)fw->data;

	if (size <= 0) {
		AICWFDBG(LOGERROR, "wrong size of firmware file\n");
		release_firmware(fw);
		return -1;
	}

	/* Copy the file on the Embedded side */
	if (size > 1024) {// > 1KB data
		for (i = 0; i < (size - 1024); i += 1024) {//each time write 1KB
			err = pcie_send_dbg_mem_block_write_req(rwnx_hw, fw_addr + i, 1024, dst + i / 4);
			if (err) {
				AICWFDBG(LOGERROR, "bin upload fail: %x, err:%d\r\n", fw_addr + i, err);
				break;
			}
		}
	}

	if (!err && (i < size)) {// <1KB data
		err = pcie_send_dbg_mem_block_write_req(rwnx_hw, fw_addr + i, size - i, dst + i / 4);
		if (err) {
			AICWFDBG(LOGERROR, "bin upload fail: %x, err:%d\r\n", fw_addr + i, err);
		}
	}

	release_firmware(fw);
	return err;
}

int pcie_reset_firmware(struct rwnx_hw *rwnx_hw, u32 fw_addr)
{
	int err = 0;
	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80) {
		volatile unsigned int *reset_addr = (volatile unsigned int *)(rwnx_hw->pcidev->pci_bar1_vaddr + 0x500044);
		volatile unsigned int *reset_bit = (volatile unsigned int *)(rwnx_hw->pcidev->pci_bar1_vaddr + 0x500128);
		volatile unsigned int *clear_bit = (volatile unsigned int *)(rwnx_hw->pcidev->pci_bar1_vaddr + 0x50012c);

		reset_addr[0] = fw_addr; //reset firmware to fw_addr
		reset_bit[0] = (0X01<<5);
		clear_bit[0] = (0X01<<5);
	} else {
		writel(fw_addr, rwnx_hw->pcidev->emb_sctl + 0x044);
		writel((1<<5),  rwnx_hw->pcidev->emb_sctl + 0x128);
		writel((1<<5),  rwnx_hw->pcidev->emb_sctl + 0x12c);
	}

	if (testmode == 0 && !fw_flsupg && !fw_flggen2 && !fw_flggen1) {
		//mdelay(300);
		while (*(volatile uint32_t *)&rwnx_hw->ipc_env->shared->fw_init_done != 1) {
			AICWFDBG(LOGVERBOS, "fw init done=%d\n", *(volatile uint32_t *)&rwnx_hw->ipc_env->shared->fw_init_done);
			msleep(5);
		}
	} else
		mdelay(5000);

	AICWFDBG(LOGINFO, "fw init done\n");

	return err;
}

nvram_info_t nvram_info = {
	.txpwr_idx = {
		.enable           = 1,
		.dsss             = 9,
		.ofdmlowrate_2g4  = 8,
		.ofdm64qam_2g4    = 8,
		.ofdm256qam_2g4   = 8,
		.ofdm1024qam_2g4  = 8,
		.ofdmlowrate_5g   = 11,
		.ofdm64qam_5g     = 10,
		.ofdm256qam_5g    = 9,
		.ofdm1024qam_5g   = 9
	},
	.txpwr_lvl_v2 = {
		.enable             = 1,
		.pwrlvl_11b_11ag_2g4 =
			//1M,   2M,   5M5,  11M,  6M,   9M,   12M,  18M,  24M,  36M,  48M,  54M
			{ 20,   20,   20,   20,   20,   20,   20,   20,   18,   18,   16,   16},
		.pwrlvl_11n_11ac_2g4 =
			//MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9
			{ 20,   20,   20,   20,   18,   18,   16,   16,   16,   16},
		.pwrlvl_11ax_2g4 =
			//MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9, MCS10,MCS11
			{ 20,   20,   20,   20,   18,   18,   16,   16,   16,   16,   15,   15},
	},
	.txpwr_lvl_v3 = {
		.enable             = 1,
		.pwrlvl_11b_11ag_2g4 =
			//1M,   2M,   5M5,  11M,  6M,   9M,   12M,  18M,  24M,  36M,  48M,  54M
			{ 20,   20,   20,   20,   20,   20,   20,   20,   18,   18,   16,   16},
		.pwrlvl_11n_11ac_2g4 =
			//MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9
			{ 20,   20,   20,   20,   18,   18,   16,   16,   16,   16},
		.pwrlvl_11ax_2g4 =
			//MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9, MCS10,MCS11
			{ 20,   20,   20,   20,   18,   18,   16,   16,   16,   16,   15,   15},
		 .pwrlvl_11a_5g =
			//NA,   NA,   NA,   NA,   6M,   9M,   12M,  18M,  24M,  36M,  48M,  54M
			{ 0x80, 0x80, 0x80, 0x80, 20,   20,   20,   20,   18,   18,   16,   16},
		.pwrlvl_11n_11ac_5g =
			//MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9
			{ 20,   20,   20,   20,   18,   18,   16,   16,   16,   15},
		.pwrlvl_11ax_5g =
			//MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9, MCS10,MCS11
			{ 20,   20,   20,   20,   18,   18,   16,   16,   16,   15,   14,   14},
	},
	.txpwr_lvl_v4 = {
		.enable             = 1,
		.pwrlvl_11b_11ag_2g4 =
			//1M,   2M,   5M5,  11M,  6M,   9M,   12M,  18M,  24M,  36M,  48M,  54M
			{ 20,   20,   20,   20,   20,   20,   20,   20,   18,   18,   16,   16},
		.pwrlvl_11n_11ac_2g4 =
			//MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9
			{ 20,   20,   20,   20,   18,   18,   16,   16,   16,   16},
		.pwrlvl_11ax_2g4 =
			//MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9, MCS10,MCS11
			{ 20,   20,   20,   20,   18,   18,   16,   16,   16,   16,   15,   15},
		.pwrlvl_11a_5g =
			//6M,   9M,   12M,  18M,  24M,  36M,  48M,  54M
			{ 20,   20,   20,   20,   18,   18,   16,   16},
		.pwrlvl_11n_11ac_5g =
			//MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9
			{ 20,   20,   20,   20,   18,   18,   16,   16,   16,   15},
		.pwrlvl_11ax_5g =
			//MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9, MCS10,MCS11
			{ 20,   20,   20,   20,   18,   18,   16,   16,   16,   15,   14,   14},
	},
	.txpwr_loss = {
		.loss_enable_2g4 = 0,
		.loss_value_2g4  = 0,
		.loss_enable_5g  = 0,
		.loss_value_5g   = 0,
	},
	.txpwr_ofst = {
		.enable       = 1,
		.chan_1_4     = 0,
		.chan_5_9     = 0,
		.chan_10_13   = 0,
		.chan_36_64   = 0,
		.chan_100_120 = 0,
		.chan_122_140 = 0,
		.chan_142_165 = 0,
	},
	.txpwr_ofst2x = {
		.enable       = 0,
		.pwrofst2x_tbl_2g4 = {
			// ch1-4, ch5-9, ch10-13
			{   0,    0,    0   }, // 11b
			{   0,    0,    0   }, // ofdm_highrate
			{   0,    0,    0   }, // ofdm_lowrate
		},
		.pwrofst2x_tbl_5g = {
			// ch42,  ch58, ch106,ch122,ch138,ch155
			{   0,    0,    0,    0,    0,    0   }, // ofdm_lowrate
			{   0,    0,    0,    0,    0,    0   }, // ofdm_highrate
			{   0,    0,    0,    0,    0,    0   }, // ofdm_midrate
		},
	},
	.txpwr_ofst2x_v2 = {
		.enable        = 0,
		.pwrofst_flags = 0,
		.pwrofst2x_tbl_2g4_ant0 = {
			// 11b, ofdm_highrate, ofdm_lowrate
			{   0,    0,    0   }, // ch1-4
			{   0,    0,    0   }, // ch5-9
			{   0,    0,    0   }, // ch10-13
		},
		.pwrofst2x_tbl_2g4_ant1 = {
			// 11b, ofdm_highrate, ofdm_lowrate
			{   0,    0,    0   }, // ch1-4
			{   0,    0,    0   }, // ch5-9
			{   0,    0,    0   }, // ch10-13
		},
		.pwrofst2x_tbl_5g_ant0 = {
			// ofdm_highrate, ofdm_lowrate, ofdm_midrate
			{   0,    0,    0   }, // ch42
			{   0,    0,    0   }, // ch58
			{   0,    0,    0   }, // ch106
			{   0,    0,    0   }, // ch122
			{   0,    0,    0   }, // ch138
			{   0,    0,    0   }, // ch155
		},
		.pwrofst2x_tbl_5g_ant1 = {
			// ofdm_highrate, ofdm_lowrate, ofdm_midrate
			{   0,    0,    0   }, // ch42
			{   0,    0,    0   }, // ch58
			{   0,    0,    0   }, // ch106
			{   0,    0,    0   }, // ch122
			{   0,    0,    0   }, // ch138
			{   0,    0,    0   }, // ch155
		},
		.pwrofst2x_tbl_6g_ant0 = {   0,   }, // ofdm_highrate: 6e_ch7 ~ 6e_ch229
		.pwrofst2x_tbl_6g_ant1 = {   0,   }, // ofdm_highrate: 6e_ch7 ~ 6e_ch229
	},
	.xtal_cap = {
		.enable        = 0,
		.xtal_cap      = 24,
		.xtal_cap_fine = 31,
	},
};

void get_userconfig_txpwr_ofst(txpwr_ofst_conf_t *txpwr_ofst)
{
	memcpy(txpwr_ofst, &(nvram_info.txpwr_ofst), sizeof(txpwr_ofst_conf_t));
}

void get_userconfig_txpwr_ofst2x(txpwr_ofst2x_conf_t *txpwr_ofst2x)
{
	*txpwr_ofst2x = nvram_info.txpwr_ofst2x;
}

void get_userconfig_txpwr_ofst2x_v2(txpwr_ofst2x_conf_v2_t *txpwr_ofst2x_v2)
{
	*txpwr_ofst2x_v2 = nvram_info.txpwr_ofst2x_v2;
}

void get_userconfig_xtal_cap(xtal_cap_conf_t *xtal_cap)
{
	*xtal_cap = nvram_info.xtal_cap;
}

void get_userconfig_txpwr_idx(txpwr_idx_conf_t *txpwr_idx)
{
	memcpy(txpwr_idx, &(nvram_info.txpwr_idx), sizeof(txpwr_idx_conf_t));
}

void get_userconfig_txpwr_lvl_v2(txpwr_lvl_conf_v2_t *txpwr_lvl_v2)
{
	*txpwr_lvl_v2 = nvram_info.txpwr_lvl_v2;
}

void get_userconfig_txpwr_lvl_v3(txpwr_lvl_conf_v3_t *txpwr_lvl_v3)
{
	*txpwr_lvl_v3 = nvram_info.txpwr_lvl_v3;
}

void get_userconfig_txpwr_lvl_v4(txpwr_lvl_conf_v4_t *txpwr_lvl_v4)
{
	*txpwr_lvl_v4 = nvram_info.txpwr_lvl_v4;
}

void get_userconfig_txpwr_lvl_adj(txpwr_lvl_adj_conf_t *txpwr_lvl_adj)
{
	*txpwr_lvl_adj = nvram_info.txpwr_lvl_adj;
}

void get_userconfig_txpwr_loss(txpwr_loss_conf_t *txpwr_loss)
{
	memcpy(txpwr_loss, &(nvram_info.txpwr_loss), sizeof(txpwr_loss_conf_t));
}

s8_l get_txpwr_max(s8_l power)
{
	int i = 0;

	if (g_rwnx_plat->pcidev->chip_id == PRODUCT_ID_AIC8800D80) {
		for (i = 0; i <= 11; i++) {
			if (power < nvram_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[i])
				power = nvram_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[i];
		}
		for (i = 0; i <= 9; i++) {
			if (power < nvram_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[i])
				power = nvram_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[i];
		}
		for (i = 0; i <= 11; i++) {
			if (power < nvram_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[i])
				power = nvram_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[i];
		}
		for (i = 4; i <= 11; i++) {
			if (power < nvram_info.txpwr_lvl_v3.pwrlvl_11a_5g[i])
				power = nvram_info.txpwr_lvl_v3.pwrlvl_11a_5g[i];
		}
		for (i = 0; i <= 9; i++) {
			if (power < nvram_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[i])
				power = nvram_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[i];
		}
		for (i = 0; i <= 11; i++) {
			if (power < nvram_info.txpwr_lvl_v3.pwrlvl_11ax_5g[i])
				power = nvram_info.txpwr_lvl_v3.pwrlvl_11ax_5g[i];
		}

		if ((nvram_info.txpwr_loss.loss_enable_2g4 == 1) ||
			(nvram_info.txpwr_loss.loss_enable_5g == 1)) {
		if (nvram_info.txpwr_loss.loss_value_2g4 <
			nvram_info.txpwr_loss.loss_value_5g)
			power += nvram_info.txpwr_loss.loss_value_5g;
		else
			power += nvram_info.txpwr_loss.loss_value_2g4;
		}
	} else {
		for (i = 0; i <= 11; i++) {
			if (power < nvram_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[i])
				power = nvram_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[i];
		}
		for (i = 0; i <= 9; i++) {
			if (power < nvram_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[i])
				power = nvram_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[i];
		}
		for (i = 0; i <= 11; i++) {
			if (power < nvram_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[i])
				power = nvram_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[i];
		}
		for (i = 0; i <= 7; i++) {
			if (power < nvram_info.txpwr_lvl_v4.pwrlvl_11a_5g[i])
				power = nvram_info.txpwr_lvl_v4.pwrlvl_11a_5g[i];
		}
		for (i = 0; i <= 9; i++) {
			if (power < nvram_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[i])
				power = nvram_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[i];
		}
		for (i = 0; i <= 11; i++) {
			if (power < nvram_info.txpwr_lvl_v4.pwrlvl_11ax_5g[i])
				power = nvram_info.txpwr_lvl_v4.pwrlvl_11ax_5g[i];
		}

		if ((nvram_info.txpwr_loss.loss_enable_2g4 == 1) ||
			(nvram_info.txpwr_loss.loss_enable_5g == 1)) {
		if (nvram_info.txpwr_loss.loss_value_2g4 <
			nvram_info.txpwr_loss.loss_value_5g)
			power += nvram_info.txpwr_loss.loss_value_5g;
		else
			power += nvram_info.txpwr_loss.loss_value_2g4;
		}
	}

	// AICWFDBG(LOGINFO, "%s:txpwr_max:%d\r\n", __func__, power);
	return power;
}

void set_txpwr_loss_ofst(s8_l value)
{
	int i = 0;

	if (g_rwnx_plat->pcidev->chip_id == PRODUCT_ID_AIC8800D80) {
		for (i = 0; i <= 11; i++) {
			nvram_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[i] += value;
		}
		for (i = 0; i <= 9; i++) {
			nvram_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[i] += value;
		}
		for (i = 0; i <= 11; i++) {
			nvram_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[i] += value;
		}
		for (i = 4; i <= 11; i++) {
			nvram_info.txpwr_lvl_v3.pwrlvl_11a_5g[i] += value;
		}
		for (i = 0; i <= 9; i++) {
			nvram_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[i] += value;
		}
		for (i = 0; i <= 11; i++) {
			nvram_info.txpwr_lvl_v3.pwrlvl_11ax_5g[i] += value;
		}
	} else if (g_rwnx_plat->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2) {
		for (i = 0; i <= 11; i++) {
			nvram_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[i] += value;
		}
		for (i = 0; i <= 9; i++) {
			nvram_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[i] += value;
		}
		for (i = 0; i <= 11; i++) {
			nvram_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[i] += value;
		}
		for (i = 0; i <= 7; i++) {
			nvram_info.txpwr_lvl_v4.pwrlvl_11a_5g[i] += value;
		}
		for (i = 0; i <= 9; i++) {
			nvram_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[i] += value;
		}
		for (i = 0; i <= 11; i++) {
			nvram_info.txpwr_lvl_v4.pwrlvl_11ax_5g[i] += value;
		}
	}
	AICWFDBG(LOGINFO, "%s:value:%d\r\n", __func__, value);
}

#define MATCH_NODE(type, node, cfg_key) {cfg_key, offsetof(type, node)}

struct parse_match_t {
	char keyname[64];
	int  offset;
};

static const char *parse_key_prefix[] = {
	[0x01] = "module0_",
	[0x21] = "module1_",
	[0xFF] = "",
};

static const struct parse_match_t parse_match_tab[] = {
	MATCH_NODE(nvram_info_t, txpwr_idx.enable,           "enable"),
	MATCH_NODE(nvram_info_t, txpwr_idx.dsss,             "dsss"),
	MATCH_NODE(nvram_info_t, txpwr_idx.ofdmlowrate_2g4,  "ofdmlowrate_2g4"),
	MATCH_NODE(nvram_info_t, txpwr_idx.ofdm64qam_2g4,    "ofdm64qam_2g4"),
	MATCH_NODE(nvram_info_t, txpwr_idx.ofdm256qam_2g4,   "ofdm256qam_2g4"),
	MATCH_NODE(nvram_info_t, txpwr_idx.ofdm1024qam_2g4,  "ofdm1024qam_2g4"),
	MATCH_NODE(nvram_info_t, txpwr_idx.ofdmlowrate_5g,   "ofdmlowrate_5g"),
	MATCH_NODE(nvram_info_t, txpwr_idx.ofdm64qam_5g,     "ofdm64qam_5g"),
	MATCH_NODE(nvram_info_t, txpwr_idx.ofdm256qam_5g,    "ofdm256qam_5g"),
	MATCH_NODE(nvram_info_t, txpwr_idx.ofdm1024qam_5g,   "ofdm1024qam_5g"),

	{"lvl_11b_11ag_1m_2g4",  offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11b_11ag_2g4) + 0},
	{"lvl_11b_11ag_2m_2g4",  offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11b_11ag_2g4) + 1},
	{"lvl_11b_11ag_5m5_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11b_11ag_2g4) + 2},
	{"lvl_11b_11ag_11m_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11b_11ag_2g4) + 3},
	{"lvl_11b_11ag_6m_2g4",  offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11b_11ag_2g4) + 4},
	{"lvl_11b_11ag_9m_2g4",  offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11b_11ag_2g4) + 5},
	{"lvl_11b_11ag_12m_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11b_11ag_2g4) + 6},
	{"lvl_11b_11ag_18m_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11b_11ag_2g4) + 7},
	{"lvl_11b_11ag_24m_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11b_11ag_2g4) + 8},
	{"lvl_11b_11ag_36m_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11b_11ag_2g4) + 9},
	{"lvl_11b_11ag_48m_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11b_11ag_2g4) + 10},
	{"lvl_11b_11ag_54m_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11b_11ag_2g4) + 11},

	{"lvl_11n_11ac_mcs0_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11n_11ac_2g4) + 0},
	{"lvl_11n_11ac_mcs1_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11n_11ac_2g4) + 1},
	{"lvl_11n_11ac_mcs2_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11n_11ac_2g4) + 2},
	{"lvl_11n_11ac_mcs3_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11n_11ac_2g4) + 3},
	{"lvl_11n_11ac_mcs4_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11n_11ac_2g4) + 4},
	{"lvl_11n_11ac_mcs5_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11n_11ac_2g4) + 5},
	{"lvl_11n_11ac_mcs6_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11n_11ac_2g4) + 6},
	{"lvl_11n_11ac_mcs7_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11n_11ac_2g4) + 7},
	{"lvl_11n_11ac_mcs8_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11n_11ac_2g4) + 8},
	{"lvl_11n_11ac_mcs9_2g4", offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11n_11ac_2g4) + 9},

	{"lvl_11ax_mcs0_2g4",     offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11ax_2g4) + 0},
	{"lvl_11ax_mcs1_2g4",     offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11ax_2g4) + 1},
	{"lvl_11ax_mcs2_2g4",     offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11ax_2g4) + 2},
	{"lvl_11ax_mcs3_2g4",     offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11ax_2g4) + 3},
	{"lvl_11ax_mcs4_2g4",     offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11ax_2g4) + 4},
	{"lvl_11ax_mcs5_2g4",     offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11ax_2g4) + 5},
	{"lvl_11ax_mcs6_2g4",     offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11ax_2g4) + 6},
	{"lvl_11ax_mcs7_2g4",     offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11ax_2g4) + 7},
	{"lvl_11ax_mcs8_2g4",     offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11ax_2g4) + 8},
	{"lvl_11ax_mcs9_2g4",     offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11ax_2g4) + 9},
	{"lvl_11ax_mcs10_2g4",    offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11ax_2g4) + 10},
	{"lvl_11ax_mcs11_2g4",    offsetof(nvram_info_t, txpwr_lvl_v2.pwrlvl_11ax_2g4) + 11},

	{"lvl_11a_1m_5g",         offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11a_5g) + 0},
	{"lvl_11a_2m_5g",         offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11a_5g) + 1},
	{"lvl_11a_5m5_5g",        offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11a_5g) + 2},
	{"lvl_11a_11m_5g",        offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11a_5g) + 3},
	{"lvl_11a_6m_5g",         offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11a_5g) + 4},
	{"lvl_11a_9m_5g",         offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11a_5g) + 5},
	{"lvl_11a_12m_5g",        offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11a_5g) + 6},
	{"lvl_11a_18m_5g",        offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11a_5g) + 7},
	{"lvl_11a_24m_5g",        offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11a_5g) + 8},
	{"lvl_11a_36m_5g",        offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11a_5g) + 9},
	{"lvl_11a_48m_5g",        offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11a_5g) + 10},
	{"lvl_11a_54m_5g",        offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11a_5g) + 11},

	{"lvl_11n_11ac_mcs0_5g",  offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11n_11ac_5g) + 0},
	{"lvl_11n_11ac_mcs1_5g",  offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11n_11ac_5g) + 1},
	{"lvl_11n_11ac_mcs2_5g",  offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11n_11ac_5g) + 2},
	{"lvl_11n_11ac_mcs3_5g",  offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11n_11ac_5g) + 3},
	{"lvl_11n_11ac_mcs4_5g",  offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11n_11ac_5g) + 4},
	{"lvl_11n_11ac_mcs5_5g",  offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11n_11ac_5g) + 5},
	{"lvl_11n_11ac_mcs6_5g",  offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11n_11ac_5g) + 6},
	{"lvl_11n_11ac_mcs7_5g",  offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11n_11ac_5g) + 7},
	{"lvl_11n_11ac_mcs8_5g",  offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11n_11ac_5g) + 8},
	{"lvl_11n_11ac_mcs9_5g",  offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11n_11ac_5g) + 9},

	{"lvl_11ax_mcs0_5g",      offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11ax_5g) + 0},
	{"lvl_11ax_mcs1_5g",      offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11ax_5g) + 1},
	{"lvl_11ax_mcs2_5g",      offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11ax_5g) + 2},
	{"lvl_11ax_mcs3_5g",      offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11ax_5g) + 3},
	{"lvl_11ax_mcs4_5g",      offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11ax_5g) + 4},
	{"lvl_11ax_mcs5_5g",      offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11ax_5g) + 5},
	{"lvl_11ax_mcs6_5g",      offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11ax_5g) + 6},
	{"lvl_11ax_mcs7_5g",      offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11ax_5g) + 7},
	{"lvl_11ax_mcs8_5g",      offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11ax_5g) + 8},
	{"lvl_11ax_mcs9_5g",      offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11ax_5g) + 9},
	{"lvl_11ax_mcs10_5g",     offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11ax_5g) + 10},
	{"lvl_11ax_mcs11_5g",     offsetof(nvram_info_t, txpwr_lvl_v3.pwrlvl_11ax_5g) + 11},

	{"lvl_11a_mcs0_6g",       offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11a_6g) + 0},
	{"lvl_11a_mcs1_6g",       offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11a_6g) + 1},
	{"lvl_11a_mcs2_6g",       offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11a_6g) + 2},
	{"lvl_11a_mcs3_6g",       offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11a_6g) + 3},
	{"lvl_11a_mcs4_6g",       offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11a_6g) + 4},
	{"lvl_11a_mcs5_6g",       offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11a_6g) + 5},
	{"lvl_11a_mcs6_6g",       offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11a_6g) + 6},
	{"lvl_11a_mcs7_6g",       offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11a_6g) + 7},

	{"lvl_11n_11ac_mcs0_6g",  offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11n_11ac_6g) + 0},
	{"lvl_11n_11ac_mcs1_6g",  offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11n_11ac_6g) + 1},
	{"lvl_11n_11ac_mcs2_6g",  offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11n_11ac_6g) + 2},
	{"lvl_11n_11ac_mcs3_6g",  offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11n_11ac_6g) + 3},
	{"lvl_11n_11ac_mcs4_6g",  offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11n_11ac_6g) + 4},
	{"lvl_11n_11ac_mcs5_6g",  offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11n_11ac_6g) + 5},
	{"lvl_11n_11ac_mcs6_6g",  offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11n_11ac_6g) + 6},
	{"lvl_11n_11ac_mcs7_6g",  offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11n_11ac_6g) + 7},
	{"lvl_11n_11ac_mcs8_6g",  offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11n_11ac_6g) + 8},
	{"lvl_11n_11ac_mcs9_6g",  offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11n_11ac_6g) + 9},

	{"lvl_11ax_mcs0_6g",      offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11ax_6g) + 0},
	{"lvl_11ax_mcs1_6g",      offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11ax_6g) + 1},
	{"lvl_11ax_mcs2_6g",      offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11ax_6g) + 2},
	{"lvl_11ax_mcs3_6g",      offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11ax_6g) + 3},
	{"lvl_11ax_mcs4_6g",      offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11ax_6g) + 4},
	{"lvl_11ax_mcs5_6g",      offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11ax_6g) + 5},
	{"lvl_11ax_mcs6_6g",      offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11ax_6g) + 6},
	{"lvl_11ax_mcs7_6g",      offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11ax_6g) + 7},
	{"lvl_11ax_mcs8_6g",      offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11ax_6g) + 8},
	{"lvl_11ax_mcs9_6g",      offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11ax_6g) + 9},
	{"lvl_11ax_mcs10_6g",     offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11ax_6g) + 10},
	{"lvl_11ax_mcs11_6g",     offsetof(nvram_info_t, txpwr_lvl_v4.pwrlvl_11ax_6g) + 11},

	MATCH_NODE(nvram_info_t, txpwr_lvl_adj.enable,                       "lvl_adj_enable"),
	MATCH_NODE(nvram_info_t, txpwr_lvl_adj.pwrlvl_adj_tbl_2g4[0],        "lvl_adj_2g4_chan_1_4"),
	MATCH_NODE(nvram_info_t, txpwr_lvl_adj.pwrlvl_adj_tbl_2g4[1],        "lvl_adj_2g4_chan_5_9"),
	MATCH_NODE(nvram_info_t, txpwr_lvl_adj.pwrlvl_adj_tbl_2g4[2],        "lvl_adj_2g4_chan_10_13"),
	MATCH_NODE(nvram_info_t, txpwr_lvl_adj.pwrlvl_adj_tbl_5g[0],         "lvl_adj_5g__chan_42"),
	MATCH_NODE(nvram_info_t, txpwr_lvl_adj.pwrlvl_adj_tbl_5g[1],         "lvl_adj_5g__chan_58"),
	MATCH_NODE(nvram_info_t, txpwr_lvl_adj.pwrlvl_adj_tbl_5g[2],         "lvl_adj_5g__chan_106"),
	MATCH_NODE(nvram_info_t, txpwr_lvl_adj.pwrlvl_adj_tbl_5g[3],         "lvl_adj_5g__chan_122"),
	MATCH_NODE(nvram_info_t, txpwr_lvl_adj.pwrlvl_adj_tbl_5g[4],         "lvl_adj_5g__chan_138"),
	MATCH_NODE(nvram_info_t, txpwr_lvl_adj.pwrlvl_adj_tbl_5g[5],         "lvl_adj_5g__chan_155"),

	MATCH_NODE(nvram_info_t, txpwr_loss.loss_enable_2g4, "loss_enable_2g4"),
	MATCH_NODE(nvram_info_t, txpwr_loss.loss_value_2g4,  "loss_value_2g4"),
	MATCH_NODE(nvram_info_t, txpwr_loss.loss_enable_5g,  "loss_enable_5g"),
	MATCH_NODE(nvram_info_t, txpwr_loss.loss_value_5g,   "loss_value_5g"),

	MATCH_NODE(nvram_info_t, txpwr_ofst.enable,          "ofst_enable"),
	MATCH_NODE(nvram_info_t, txpwr_ofst.chan_1_4,        "ofst_chan_1_4"),
	MATCH_NODE(nvram_info_t, txpwr_ofst.chan_5_9,        "ofst_chan_5_9"),
	MATCH_NODE(nvram_info_t, txpwr_ofst.chan_10_13,      "ofst_chan_10_13"),
	MATCH_NODE(nvram_info_t, txpwr_ofst.chan_36_64,      "ofst_chan_36_64"),
	MATCH_NODE(nvram_info_t, txpwr_ofst.chan_100_120,    "ofst_chan_100_120"),
	MATCH_NODE(nvram_info_t, txpwr_ofst.chan_122_140,    "ofst_chan_122_140"),
	MATCH_NODE(nvram_info_t, txpwr_ofst.chan_142_165,    "ofst_chan_142_165"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.enable,        "ofst2x_enable"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_2g4[0][0], "ofst_2g4_11b_chan_1_4"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_2g4[0][1], "ofst_2g4_11b_chan_5_9"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_2g4[0][2], "ofst_2g4_11b_chan_10_13"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_2g4[1][0], "ofst_2g4_ofdm_highrate_chan_1_4"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_2g4[1][1], "ofst_2g4_ofdm_highrate_chan_5_9"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_2g4[1][2], "ofst_2g4_ofdm_highrate_chan_10_13"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_2g4[2][0], "ofst_2g4_ofdm_lowrate_chan_1_4"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_2g4[2][1], "ofst_2g4_ofdm_lowrate_chan_5_9"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_2g4[2][2], "ofst_2g4_ofdm_lowrate_chan_10_13"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[0][0],  "ofst_5g_ofdm_lowrate_chan_42"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[0][1],  "ofst_5g_ofdm_lowrate_chan_58"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[0][2],  "ofst_5g_ofdm_lowrate_chan_106"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[0][3],  "ofst_5g_ofdm_lowrate_chan_122"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[0][4],  "ofst_5g_ofdm_lowrate_chan_138"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[0][5],  "ofst_5g_ofdm_lowrate_chan_155"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[1][0],  "ofst_5g_ofdm_highrate_chan_42"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[1][1],  "ofst_5g_ofdm_highrate_chan_58"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[1][2],  "ofst_5g_ofdm_highrate_chan_106"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[1][3],  "ofst_5g_ofdm_highrate_chan_122"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[1][4],  "ofst_5g_ofdm_highrate_chan_138"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[1][5],  "ofst_5g_ofdm_highrate_chan_155"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[2][0],  "ofst_5g_ofdm_midrate_chan_42"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[2][1],  "ofst_5g_ofdm_midrate_chan_58"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[2][2],  "ofst_5g_ofdm_midrate_chan_106"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[2][3],  "ofst_5g_ofdm_midrate_chan_122"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[2][4],  "ofst_5g_ofdm_midrate_chan_138"),
	MATCH_NODE(nvram_info_t, txpwr_ofst2x.pwrofst2x_tbl_5g[2][5],  "ofst_5g_ofdm_midrate_chan_155"),

	MATCH_NODE(nvram_info_t, xtal_cap.enable,            "xtal_enable"),
	MATCH_NODE(nvram_info_t, xtal_cap.xtal_cap,          "xtal_cap"),
	MATCH_NODE(nvram_info_t, xtal_cap.xtal_cap_fine,     "xtal_cap_fine"),
};

static int parse_key_val(const char *str, const char *key, char *val)
{
	const char *p = NULL;
	const char *dst = NULL;
	int keysize = 0;
	int bufsize = 0;

	if (str == NULL || key == NULL || val == NULL)
		return -1;

	keysize = strlen(key);
	bufsize = strlen(str);
	if (bufsize <= keysize)
		return -1;

	p = str;
	while (*p != 0 && *p == ' ')
		p++;

	if (*p == '#')
		return -1;

	if (str + bufsize - p <= keysize)
		return -1;

	if (strncmp(p, key, keysize) != 0)
		return -1;

	p += keysize;

	while (*p != 0 && *p == ' ')
		p++;

	if (*p != '=')
		return -1;

	p++;
	while (*p != 0 && *p == ' ')
		p++;

	if (*p == '"')
		p++;

	dst = p;
	while (*p != 0 && *p != '#')
		p++;

	p--;
	while (*p == ' ')
		p--;

	if (*p == '"')
		p--;

	while (*p == '\r' || *p == '\n')
		p--;

	p++;
	strncpy(val, dst, p - dst);
	val[p - dst] = 0;
	return 0;
}

int rwnx_atoi(char *value)
{
	int len = 0;
	int i = 0;
	int result = 0;
	int flag = 1;

	if (value[0] == '-') {
		flag = -1;
		value++;
	}
	len = strlen(value);

	for (i = 0; i < len; i++) {
		result = result * 10;
		if (value[i] >= 48 && value[i] <= 57) {
			result += value[i] - 48;
		} else {
			result = 0;
			break;
		}
	}

	return result * flag;
}

void rwnx_plat_userconfig_parsing(struct rwnx_hw *rwnx_hw, char *buffer, int size)
{
	char conf[100], keyname[64];
	char *line;
	char *data;
	int  i = 0, err, len = 0;
	long val;
	u8   efuse_idx = 0;

	if (size <= 0) {
		pr_err("Config buffer size %d error\n", size);
		return;
	}

	if (rwnx_hw->vendor_info > (sizeof(parse_key_prefix) / sizeof(parse_key_prefix[0]) - 1)) {
		pr_err("Unsuppor vendor info config\n");
		return;
	}

#ifdef AICWF_PCIE_SUPPORT
	efuse_idx = 0xFF;
#else
	efuse_idx = rwnx_hw->vendor_info;
	if (rwnx_hw->chipid == PRODUCT_ID_AIC8800DC ||
		rwnx_hw->chipid == PRODUCT_ID_AIC8800DW ||
		rwnx_hw->chipid == PRODUCT_ID_AIC8800D80 ||
		rwnx_hw->chipid == PRODUCT_ID_AIC8800D81) {
		efuse_idx = 0xFF;
	} else  if (rwnx_hw->vendor_info == 0x00) {
		printk("Empty efuse, using module0 config\n");
		efuse_idx = 0x01;
	}
#endif

	data = vmalloc(size + 1);
	if (!data) {
		pr_err("vmalloc fail\n");
		return;
	}

	memcpy(data, buffer, size);
	buffer = data;
	while (1) {
		line = buffer;
		if (*line == 0)
			break;

		while (*buffer != '\r' && *buffer != '\n' && *buffer != 0 && len++ < size)
			buffer++;

		while ((*buffer == '\r' || *buffer == '\n') && len++ < size)
			*buffer++ = 0;

		if (len >= size)
			*buffer = 0;

		// store value to data struct
		for (i = 0; i < sizeof(parse_match_tab) / sizeof(parse_match_tab[0]); i++) {
			sprintf(&keyname[0], "%s%s", parse_key_prefix[efuse_idx], parse_match_tab[i].keyname);
			if (parse_key_val(line, keyname, conf) == 0) {
				if (kstrtol(conf, 0, &val))
					err = kstrtol(conf, 16, &val);
				*(unsigned long *)((unsigned long)&nvram_info + parse_match_tab[i].offset) = val;
				AICWFDBG(LOGINFO, "%s, %s = %ld\n",  __func__, parse_match_tab[i].keyname, val);
				break;
			}
		}
	}

	memcpy(&(nvram_info.txpwr_lvl_v3), &(nvram_info.txpwr_lvl_v2), sizeof(txpwr_lvl_conf_v2_t));
	memcpy(&(nvram_info.txpwr_lvl_v4), &(nvram_info.txpwr_lvl_v3), sizeof(txpwr_lvl_conf_v3_t));
	vfree(data);
}

int rwnx_plat_userconfig_upload_android(struct rwnx_hw *rwnx_hw, char *filename)
{
	int size;
	char *dst = NULL;

	const struct firmware *fw = NULL;
	int ret = request_firmware(&fw, filename, NULL);

	AICWFDBG(LOGINFO, "userconfig file path:%s\r\n", filename);

	if (ret < 0) {
		printk("Load %s fail\n", filename);
		return ret;
	}

	size = fw->size;
	dst = (char *)fw->data;

	if (size <= 0) {
		AICWFDBG(LOGERROR, "wrong size of firmware file\n");
		release_firmware(fw);
		return -1;
	}

	rwnx_plat_userconfig_parsing(rwnx_hw, (char *)dst, size);

	release_firmware(fw);

	AICWFDBG(LOGTRACE, "userconfig download complete\n\n");

	return 0;
}

#define FW_USERCONFIG_NAME_8800D    "aic_userconfig.txt"
#define FW_USERCONFIG_NAME_8800DC   "aic8800dc/aic_userconfig_8800dc.txt"
#define FW_USERCONFIG_NAME_8800DW   "aic8800dc/aic_userconfig_8800dw.txt"
#if defined (AICWF_SDIO_SUPPORT)
#define FW_USERCONFIG_NAME_8800D80  "aic8800d80/aic_userconfig_8800d80.txt"
#elif defined (AICWF_USB_SUPPORT)
#define FW_USERCONFIG_NAME_8800D80  "aic8800d80/usb/aic_userconfig_8800d80.txt"
#elif defined (AICWF_PCIE_SUPPORT)
#define FW_USERCONFIG_NAME_8800D80   "aic8800d80/pcie/aic_userconfig_8800d80.txt"
#define FW_USERCONFIG_NAME_8800D80X2 "aic8800d80x2/pcie/aic_userconfig_8800d80x2.txt"
#endif

/**
 * rwnx_plat_fmac_load() - Load FW code
 *
 * @rwnx_hw: Main driver data
 */
static int rwnx_plat_fmac_load(struct rwnx_hw *rwnx_hw)
{
	int ret = 0;

	RWNX_DBG(RWNX_FN_ENTRY_STR);
#if defined AICWF_USB_SUPPORT
	if (rwnx_hw->chipid == PRODUCT_ID_AIC8800DC || rwnx_hw->chipid == PRODUCT_ID_AIC8800DW)
		ret = aicfw_8800dc_fw_init(rwnx_hw);

	if (ret) {
		printk(">>>%s:aicfw_8800xx_fw_init fail\n\n", __func__);
		return ret;
	}

	if (rwnx_hw->chipid == PRODUCT_ID_AIC8800D || rwnx_hw->chipid == PRODUCT_ID_AIC8801)
		ret = rwnx_plat_userconfig_upload_android(rwnx_hw, FW_USERCONFIG_NAME_8800D);
	else if (rwnx_hw->chipid == PRODUCT_ID_AIC8800DC)
		ret = rwnx_plat_userconfig_upload_android(rwnx_hw, FW_USERCONFIG_NAME_8800DC);
	else if (rwnx_hw->chipid == PRODUCT_ID_AIC8800DW)
		ret = rwnx_plat_userconfig_upload_android(rwnx_hw, FW_USERCONFIG_NAME_8800DW);
	else if (rwnx_hw->chipid == PRODUCT_ID_AIC8800D80 || rwnx_hw->chipid == PRODUCT_ID_AIC8800D81)
		ret = rwnx_plat_userconfig_upload_android(rwnx_hw, FW_USERCONFIG_NAME_8800D80);
	else if (rwnx_hw->chipid == PRODUCT_ID_AIC8800D80X2)
		ret = rwnx_plat_userconfig_upload_android(rwnx_hw, FW_USERCONFIG_NAME_8800D80X2);
#elif defined AICWF_PCIE_SUPPORT
	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80)
		ret = aicwf_8800d80_fw_init(rwnx_hw);
	else if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2)
		ret = aicwf_8800d80x2_fw_init(rwnx_hw);

	if (ret) {
		printk(">>>%s:aicfw_8800xx_fw_init fail\n\n", __func__);
		return ret;
	}

	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80)
		ret = rwnx_plat_userconfig_upload_android(rwnx_hw, FW_USERCONFIG_NAME_8800D80);
	else if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2)
		ret = rwnx_plat_userconfig_upload_android(rwnx_hw, FW_USERCONFIG_NAME_8800D80X2);
#endif

	return ret;
}

/**
 * rwnx_platform_reset() - Reset the platform
 *
 * @rwnx_plat: platform data
 */
static int rwnx_platform_reset(struct rwnx_plat *rwnx_plat)
{
	u32 regval;

#if defined(AICWF_USB_SUPPORT) || defined(AICWF_SDIO_SUPPORT) || defined(AICWF_PCIE_SUPPORT)
	return 0;
#endif

	/* the doc states that SOFT implies FPGA_B_RESET
	 * adding FPGA_B_RESET is clearer */
	RWNX_REG_WRITE(SOFT_RESET | FPGA_B_RESET, rwnx_plat,
				   RWNX_ADDR_SYSTEM, SYSCTRL_MISC_CNTL_ADDR);
	msleep(100);

	regval = RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM, SYSCTRL_MISC_CNTL_ADDR);

	if (regval & SOFT_RESET) {
		dev_err(rwnx_platform_get_dev(rwnx_plat), "reset: failed\n");
		return -EIO;
	}

	RWNX_REG_WRITE(regval & ~FPGA_B_RESET, rwnx_plat,
				   RWNX_ADDR_SYSTEM, SYSCTRL_MISC_CNTL_ADDR);
	msleep(100);
	return 0;
}

/**
 * rwmx_platform_save_config() - Save hardware config before reload
 *
 * @rwnx_plat: Pointer to platform data
 *
 * Return configuration registers values.
 */
static void *rwnx_term_save_config(struct rwnx_plat *rwnx_plat)
{
	const u32 *reg_list;
	u32 *reg_value, *res;
	int i, size = 0;

	if (rwnx_plat->get_config_reg) {
		size = rwnx_plat->get_config_reg(rwnx_plat, &reg_list);
	}

	if (size <= 0)
		return NULL;

	res = kmalloc(sizeof(u32) * size, GFP_KERNEL);
	if (!res)
		return NULL;

	reg_value = res;
	for (i = 0; i < size; i++) {
		*reg_value++ = RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM,
									 *reg_list++);
	}

	return res;
}

#ifdef CONFIG_POWER_LIMIT
#define GetLineFromBuffer(buffer)   strsep(&buffer, "\n")

static int isAllSpaceOrTab(uint8_t *data, uint8_t size)
{
	uint8_t cnt = 0, NumOfSpaceAndTab = 0;
	while (size > cnt) {
		if (data[cnt] == ' ' || data[cnt] == '\t' || data[cnt] == '\0')
			++NumOfSpaceAndTab;
		++cnt;
	}
	return size == NumOfSpaceAndTab;
}

static int IsCommentString(char *szStr)
{
	if (*szStr == '#' && *(szStr + 1) == ' ')
		return 1;
	else
		return 0;
}

static int ParseQualifiedString(char *In, u32 *Start, char *Out, char LeftQualifier, char RightQualifier)
{
	u32 i = 0, j = 0;
	char c = In[(*Start)++];
	if (c != LeftQualifier)
		return 0;
	i = (*Start);
	c = In[(*Start)++];
	while (c != RightQualifier && c != '\0')
		c = In[(*Start)++];
	if (c == '\0')
		return 0;
	j = (*Start) - 2;
	strncpy((char *)Out, (const char *)(In + i), j - i + 1);
	return 1;
}

static int GetU1ByteIntegerFromStringInDecimal(char *Str, u8 *pInt)
{
	u16 i = 0;
	*pInt = 0;
	while (Str[i] != '\0') {
		if (Str[i] >= '0' && Str[i] <= '9') {
			*pInt *= 10;
			*pInt += (Str[i] - '0');
		} else
			return 0;
		++i;
	}
	return 1;
}

static int GetS1ByteIntegerFromStringInDecimal(char *str, s8 *val)
{
	u8 negative = 0;
	u16 i = 0;
	*val = 0;
	while (str[i] != '\0') {
		if (i == 0 && (str[i] == '+' || str[i] == '-')) {
			if (str[i] == '-')
				negative = 1;
		} else if (str[i] >= '0' && str[i] <= '9') {
			*val *= 10;
			*val += (str[i] - '0');
		} else
			return 0;
		++i;
	}
	if (negative)
		*val = -*val;
	return 1;
}

int8_t rwnx_plat_powerlimit_save(u8_l band, char *channel, u8_l bw, char *limit, char *name)
{
	u8 channel_num, powerLimit_val, reg_idx;

	if (GetU1ByteIntegerFromStringInDecimal((char *)channel, &channel_num) == 0
		|| GetS1ByteIntegerFromStringInDecimal((char *)limit, &powerLimit_val) == 0) {
		AICWFDBG(LOGERROR, "Illegal index of power limit table [ch %s][val %s]\n", channel, limit);
		return -1;
	}

	reg_idx = get_region_index(name);

	if (band == PHY_BAND_2G4) {
		uint8_t cur_idx = powerlimit_info.txpwr_lmt[reg_idx].ch_cnt_2g4[bw];
		AICWFDBG(LOGINFO, "reg_idx=%d, band=%d, bw=%d, cur_idx=%d, ch=%s, pwr=%s\n",
				reg_idx, band, bw, cur_idx, channel, limit);
		if (cur_idx < MAC_DOMAINCHANNEL_24G_MAX) {
			powerlimit_info.txpwr_lmt[reg_idx].ch_num_2g4[bw][cur_idx] = channel_num;
			powerlimit_info.txpwr_lmt[reg_idx].max_pwr_2g4[bw][cur_idx] = powerLimit_val;
			powerlimit_info.txpwr_lmt[reg_idx].ch_cnt_2g4[bw]++;
		} else {
			AICWFDBG(LOGERROR, "band %d chan_cnt reached %d\n", band, MAC_DOMAINCHANNEL_24G_MAX);
			AICWFDBG(LOGERROR, "channel=%s(%d) powerLimit=%s(%d)\n", channel, channel_num, limit, powerLimit_val);
		}
	} else if (band == PHY_BAND_5G) {
		uint8_t cur_idx = powerlimit_info.txpwr_lmt[reg_idx].ch_cnt_5g[bw];
		AICWFDBG(LOGINFO, "reg_idx=%d, band=%d, bw=%d, cur_idx=%d, ch=%s, pwr=%s\n",
				reg_idx, band, bw, cur_idx, channel, limit);
		if (cur_idx < MAC_DOMAINCHANNEL_5G_MAX) {
			powerlimit_info.txpwr_lmt[reg_idx].ch_num_5g[bw][cur_idx] = channel_num;
			powerlimit_info.txpwr_lmt[reg_idx].max_pwr_5g[bw][cur_idx] = powerLimit_val;
			powerlimit_info.txpwr_lmt[reg_idx].ch_cnt_5g[bw]++;
		} else {
			AICWFDBG(LOGERROR, "band %d chan_cnt reached %d\n", band, MAC_DOMAINCHANNEL_5G_MAX);
			AICWFDBG(LOGERROR, "channel=%s(%d) powerLimit=%s(%d)\n", channel, channel_num, limit, powerLimit_val);
		}
	}

	return 0;
}

#ifdef CONFIG_POWER_LIMIT
void rwnx_plat_powerlimit_parsing(char *buffer, int size)
{
#define LD_STAGE_EXC_MAPPING    0
#define LD_STAGE_TAB_DEFINE     1
#define LD_STAGE_TAB_START      2
#define LD_STAGE_COLUMN_DEFINE  3
#define LD_STAGE_CH_ROW         4
	uint8_t loadingStage = LD_STAGE_EXC_MAPPING;
	uint32_t i = 0, forCnt = 0;
	uint32_t i_cc;
	char *szLine, *ptmp;
	char band[10], colNumBuf[10], bandwidth[10];
	uint8_t colNum = 0, band_cc = 0, bw_cc = 0;
	char **reg_name = NULL;
	// clear powerlimit info at first
	memset((void *)&powerlimit_info, 0, sizeof(powerlimit_info_t));
	ptmp = buffer;
	for (szLine = GetLineFromBuffer(ptmp); szLine != NULL; szLine = GetLineFromBuffer(ptmp)) {
		if (isAllSpaceOrTab(szLine, sizeof(*szLine)))
			continue;
		if (IsCommentString(szLine))
			continue;

		if (loadingStage == LD_STAGE_EXC_MAPPING) {
			if (szLine[0] == '#' || szLine[1] == '#') {
				loadingStage = LD_STAGE_TAB_DEFINE;
			} else {
				continue;
			}
		}

		if (loadingStage == LD_STAGE_TAB_DEFINE) {
			/* read "##	2.4G" */
			if (szLine[0] != '#' || szLine[1] != '#')
				continue;

			/* skip the space */
			i = 2;
			while (szLine[i] == ' ' || szLine[i] == '\t')
				++i;

			szLine[--i] = ' '; /* return the space in front of the regulation info */

			/* Parse the label of the table */
			memset((void *)band, 0, 10);
			memset((void *)bandwidth, 0, 10);
			memset((void *)colNumBuf, 0, 10);

			if (!ParseQualifiedString(szLine, &i, band, ' ', ',')) {
				AICWFDBG(LOGERROR, "Fail to parse band!\n");
				goto exit;
			}
			if (strncmp(band, "2.4G", 4) == 0) {
				band_cc = PHY_BAND_2G4;
			} else if (strncmp(band, "5G", 2) == 0) {
				band_cc = PHY_BAND_5G;
			}

			if (!ParseQualifiedString(szLine, &i, bandwidth, ' ', ',')) {
				AICWFDBG(LOGERROR, "Fail to parse bandwidth!\n");
				goto exit;
			}
			if (strncmp(bandwidth, "20M", 3) == 0)
				bw_cc = PHY_CHNL_BW_20;
			else if (strncmp(bandwidth, "40M", 3) == 0)
				bw_cc = PHY_CHNL_BW_40;
			else if (strncmp(bandwidth, "80M", 3) == 0)
				bw_cc = PHY_CHNL_BW_80;

			if (!ParseQualifiedString(szLine, &i, colNumBuf, '#', '#')) {
				AICWFDBG(LOGERROR, "Fail to parse column number!\n");
				goto exit;
			}
			if (!GetU1ByteIntegerFromStringInDecimal(colNumBuf, &colNum)) {
			AICWFDBG(LOGERROR, "Column number \"%s\" is not unsigned decimal\n", colNumBuf);
			goto exit;
			}
			if (colNum == 0) {
				AICWFDBG(LOGERROR, "Column number is 0\n");
				goto exit;
			}

			AICWFDBG(LOGINFO, "band=%s, bandwidth=%s, colnum=%d\n", band, bandwidth, colNum);
			loadingStage = LD_STAGE_TAB_START;
		} else if (loadingStage == LD_STAGE_TAB_START) {
			/* read "##	START" */
			if (szLine[0] != '#' || szLine[1] != '#')
				continue;

			/* skip the space */
			i = 2;
			while (szLine[i] == ' ' || szLine[i] == '\t')
				++i;

			if (strncmp((u8 *)(szLine + i), "START", 5)) {
				AICWFDBG(LOGERROR, "Missing \"##   START\" label\n");
				goto exit;
			}

			loadingStage = LD_STAGE_COLUMN_DEFINE;
		} else if (loadingStage == LD_STAGE_COLUMN_DEFINE) {
			/* read "##  FCC  ETSI" */
			if (szLine[0] != '#' || szLine[1] != '#')
				continue;

			/* skip the space */
			i = 2;
			while (szLine[i] == ' ' || szLine[i] == '\t')
				++i;

			reg_name = (char **)kmalloc(sizeof(char *) * colNum, GFP_KERNEL);
			if (!reg_name) {
				AICWFDBG(LOGERROR, "reg_name alloc fail\n");
				goto exit;
			}

			for (forCnt = 0; forCnt < colNum; ++forCnt) {
				/* skip the space */
				while (szLine[i] == ' ' || szLine[i] == '\t')
					i++;
				i_cc = i;

				while (szLine[i] != ' ' && szLine[i] != '\t' && szLine[i] != '\0')
					i++;

				reg_name[forCnt] = (char *)kmalloc(i - i_cc + 1, GFP_KERNEL);
				if (!reg_name[forCnt]) {
					AICWFDBG(LOGERROR, "reg_name element alloc fail\n");
					goto exit;
				}

				strncpy(reg_name[forCnt], szLine + i_cc, i - i_cc);
				reg_name[forCnt][i - i_cc] = '\0';
				AICWFDBG(LOGINFO, "reg_name: %s\n", reg_name[forCnt]);

			}

			loadingStage = LD_STAGE_CH_ROW;
		} else if (loadingStage == LD_STAGE_CH_ROW) {
			char channel[10] = {0}, powerLimit[10] = {0};
			u8 cnt = 0;

			/* the table ends */
			if (szLine[0] == '#' && szLine[1] == '#') {
				i = 2;
				while (szLine[i] == ' ' || szLine[i] == '\t')
					++i;

				if (strncmp((u8 *)(szLine + i), "END", 3) == 0) {
					loadingStage = LD_STAGE_TAB_DEFINE;
					if (reg_name) {
						for (forCnt = 0; forCnt < colNum; ++forCnt) {
							if (reg_name[forCnt]) {
								kfree(reg_name[forCnt]);
								reg_name[forCnt] = NULL;
							}
						}
						kfree(reg_name);
						reg_name = NULL;
					}
					colNum = 0;
					continue;
				} else {
					AICWFDBG(LOGERROR, "Missing \"##   END\" label\n");
					goto exit;
				}
			}

			if ((szLine[0] != 'c' && szLine[0] != 'C') ||
				(szLine[1] != 'h' && szLine[1] != 'H')) {
				AICWFDBG(LOGERROR, "Wrong channel prefix: '%c','%c'(%d,%d)\n", szLine[0], szLine[1], szLine[0], szLine[1]);
				continue;
			}
			i = 2;/* move to the  location behind 'h' */

			/* load the channel number */
			cnt = 0;
			while (szLine[i] >= '0' && szLine[i] <= '9') {
				channel[cnt] = szLine[i];
				++cnt;
				++i;
			}

			for (forCnt = 0; forCnt < colNum; ++forCnt) {
				/* skip the space between channel number and the power limit value */
				while (szLine[i] == ' ' || szLine[i] == '\t')
					++i;

				/* load the power limit value */
				memset((void *)powerLimit, 0, 10);

				if (szLine[i] == 'N' && szLine[i + 1] == 'A') {
					/*
					* means channel not available
					*/
					sprintf(powerLimit, "%d", POWER_LIMIT_INVALID_VAL);
					i += 2;
				} else if ((szLine[i] >= '0' && szLine[i] <= '9')
					|| szLine[i] == '+' || szLine[i] == '-') {
					/* case of dBm value */
					cnt = 0;
					while ((szLine[i] >= '0' && szLine[i] <= '9')
						|| szLine[i] == '+' || szLine[i] == '-'
					) {
						powerLimit[cnt] = szLine[i];
						++cnt;
						++i;
					}
				} else {
					AICWFDBG(LOGERROR, "Wrong limit expression \"%c%c\"(%d, %d)\n",
						szLine[i], szLine[i + 1], szLine[i], szLine[i + 1]);
					goto exit;
				}

				if (rwnx_plat_powerlimit_save(band_cc, channel, bw_cc, powerLimit, reg_name[forCnt]) < 0)
					goto exit;
			}
		}
	}

	powerlimit_info.flags |= POWER_LIMIT_CC_MATCHED_BIT;
exit:
	if (reg_name) {
		for (forCnt = 0; forCnt < colNum; ++forCnt) {
			if (reg_name[forCnt]) {
				kfree(reg_name[forCnt]);
				reg_name[forCnt] = NULL;
			}
		}
		kfree(reg_name);
		reg_name = NULL;
	}

	return;
}

int rwnx_plat_powerlimit_load(struct rwnx_hw *rwnx_hw, const char *filename)
{
	int size;
	u32 *dst = NULL;
	const struct firmware *fw = NULL;

	AICWFDBG(LOGDEBUG, "powerlimit file path:%s\r\n", filename);

	ret = request_firmware(&fw, filename, NULL);
	if (ret < 0) {
		AICWFDBG(LOGERROR, "Load %s fail\n", filename);
		release_firmware(fw);
		return -1;
	}

	size = fw->size;
	dst = (u32 *)fw->data;
	if (size <= 0) {
		AICWFDBG(LOGERROR, "wrong size of cfg file\n");
		release_firmware(fw);
		return -1;
	}

	AICWFDBG(LOGDEBUG, "### Load file done: %s, size=%d\n", filename, size);

	/* parsing the file */
	rwnx_plat_powerlimit_parsing((char *)dst, size);

	release_firmware(fw);

	AICWFDBG(LOGDEBUG, "powerlimit download complete\n\n");
	return 0;
}
#endif

/// 5G lower bound freq
#define PHY_FREQ_5G 5000

static uint16_t phy_channel_to_freq(uint8_t band, int channel)
{
	if ((band == PHY_BAND_2G4) && (channel >= 1) && (channel <= 14)) {
		if (channel == 14)
			return 2484;
		else
			return 2407 + channel * 5;
	} else if ((band == PHY_BAND_5G) && (channel >= 1) && (channel <= 165)) {
		return PHY_FREQ_5G + channel * 5;
	}
	return 0;
}

int8_t get_powerlimit_by_freq(uint8_t band, uint16_t freq, uint8_t r_idx)
{
	int8_t ret = POWER_LIMIT_INVALID_VAL;
	uint8_t idx;
	if (!(powerlimit_info.flags & POWER_LIMIT_CC_MATCHED_BIT)) {
		AICWFDBG(LOGERROR, "powerlimit flag not set\n");
		return ret;
	}

	if (band == PHY_BAND_2G4) {
		uint8_t idx_cnt = powerlimit_info.txpwr_lmt[r_idx].ch_cnt_2g4[0];
		for (idx = 0; idx < idx_cnt; idx++) {
			int ch_num = powerlimit_info.txpwr_lmt[r_idx].ch_num_2g4[0][idx];
			uint16_t freq_tmp = phy_channel_to_freq(PHY_BAND_2G4, ch_num);
			if (freq == freq_tmp) {
				ret = powerlimit_info.txpwr_lmt[r_idx].max_pwr_2g4[0][idx];
				AICWFDBG(LOGINFO, "[%d]: ch=%d(freq=%d), pwr=%d\n", idx, ch_num, freq, ret);
				break;
			}
		}
		if (idx == idx_cnt)
			AICWFDBG(LOGERROR, "powerlimit search failed: band=%d freq=%d\n", band, freq);
	} else if (band == PHY_BAND_5G) {
		uint8_t idx_cnt = powerlimit_info.txpwr_lmt[r_idx].ch_cnt_5g[0];
		for (idx = 0; idx < idx_cnt; idx++) {
			int ch_num = powerlimit_info.txpwr_lmt[r_idx].ch_num_5g[0][idx];
			uint16_t freq_tmp = phy_channel_to_freq(PHY_BAND_5G, ch_num);
			if (freq == freq_tmp) {
				ret = powerlimit_info.txpwr_lmt[r_idx].max_pwr_5g[0][idx];
				AICWFDBG(LOGINFO, "[%d]: ch=%d(freq=%d), pwr=%d\n", idx, ch_num, freq, ret);
				break;
			}
		}
		if (idx == idx_cnt)
			AICWFDBG(LOGERROR, "powerlimit search failed: band=%d freq=%d\n", band, freq);
	}
	return ret;
}

int8_t get_powerlimit_by_chnum(uint8_t chnum, uint8_t r_idx, uint8_t bw)
{
	int8_t ret = POWER_LIMIT_INVALID_VAL;
	uint8_t idx;
	if (!(powerlimit_info.flags & POWER_LIMIT_CC_MATCHED_BIT)) {
		AICWFDBG(LOGERROR, "powerlimit flag not set\n");
		return ret;
	}

	if (chnum <= 14) {
		uint8_t idx_cnt = powerlimit_info.txpwr_lmt[r_idx].ch_cnt_2g4[bw];
		for (idx = 0; idx < idx_cnt; idx++) {
			uint8_t ch_num = powerlimit_info.txpwr_lmt[r_idx].ch_num_2g4[bw][idx];
			if (chnum == ch_num) {
				ret = powerlimit_info.txpwr_lmt[r_idx].max_pwr_2g4[bw][idx];
				AICWFDBG(LOGINFO, "[%d]: ch=%d, pwr=%d\n", idx, ch_num, ret);
				break;
			}
		}
		if (idx == idx_cnt)
			AICWFDBG(LOGERROR, "%s powerlimit search failed: chnum=%d, please confirm the center frequency\n",
					__func__, chnum);
	} else if (chnum <= 165) {
		uint8_t idx_cnt = powerlimit_info.txpwr_lmt[r_idx].ch_cnt_5g[bw];
		for (idx = 0; idx < idx_cnt; idx++) {
			uint8_t ch_num = powerlimit_info.txpwr_lmt[r_idx].ch_num_5g[bw][idx];
			if (chnum == ch_num) {
				ret = powerlimit_info.txpwr_lmt[r_idx].max_pwr_5g[bw][idx];
				AICWFDBG(LOGINFO, "[%d]: ch=%d, pwr=%d\n", idx, ch_num, ret);
				break;
			}
		}
		if (idx == idx_cnt)
			AICWFDBG(LOGERROR, "%s powerlimit search failed: chnum=%d, please confirm the center frequency\n",
					__func__, chnum);
	}

	return ret;
}
#endif

#ifdef CONFIG_USE_BT
struct aicbsp_info_t aicbsp_info = {
	.hwinfo   = AICBSP_HWINFO_DEFAULT,
	.cpmode   = AICBSP_CPMODE_DEFAULT,
};

int aicbt_patch_table_free(struct aicbt_patch_table **head)
{
	struct aicbt_patch_table *p = *head, *n = NULL;
	while (p) {
		n = p->next;
		vfree(p->name);
		vfree(p->data);
		vfree(p);
		p = n;
	}
	*head = NULL;
	return 0;
}

struct aicbt_patch_table *aicbt_patch_table_alloc(const char *filename)
{
	uint8_t *rawdata = NULL, *p;
	int size;
	struct aicbt_patch_table *head = NULL, *new = NULL, *cur = NULL;

	const struct firmware *fw = NULL;
	int ret = request_firmware(&fw, filename, NULL);

	AICWFDBG(LOGINFO, "rwnx_request_firmware, name: %s\n", filename);
	if (ret < 0) {
		AICWFDBG(LOGERROR, "Load %s fail\n", filename);
		return NULL;
	}

	rawdata = (uint8_t *)fw->data;
	size = fw->size;

	if (size <= 0) {
		AICWFDBG(LOGERROR, "wrong size of firmware file\n");
		goto err;
	}

	p = rawdata;
	if (memcmp(p, AICBT_PT_TAG, sizeof(AICBT_PT_TAG) < 16 ? sizeof(AICBT_PT_TAG) : 16)) {
		AICWFDBG(LOGERROR, "TAG err\n");
		goto err;
	}
	p += 16;

	while (p - rawdata < size) {
		new = (struct aicbt_patch_table *)kzalloc(sizeof(struct aicbt_patch_table), GFP_KERNEL);
		memset(new, 0, sizeof(struct aicbt_patch_table));
		if (head == NULL) {
			head = new;
			cur  = new;
		} else {
			cur->next = new;
			cur = cur->next;
		}

		cur->name = (char *)kzalloc(sizeof(char) * 16, GFP_KERNEL);
		memcpy(cur->name, p, 16);
		p += 16;

		cur->type = *(uint32_t *)p;
		p += 4;

		cur->len = *(uint32_t *)p;
		p += 4;

		cur->data = (uint32_t *)kzalloc(sizeof(uint8_t) * cur->len * 8, GFP_KERNEL);
		memcpy(cur->data, p, cur->len * 8);
		p += cur->len * 8;
	}
	release_firmware(fw);
	return head;

err:
	aicbt_patch_table_free(&head);
	release_firmware(fw);
	return NULL;
}

int aicbt_patch_info_unpack(struct aicbt_patch_info_t *patch_info, struct aicbt_patch_table *head_t)
{
	uint8_t *patch_info_array = (uint8_t *)patch_info;
	int base_len = 0;
	int memcpy_len = 0;

	 if (AICBT_PT_INF == head_t->type) {
		base_len = ((offsetof(struct aicbt_patch_info_t, ext_patch_nb_addr) - offsetof(struct aicbt_patch_info_t, adid_addrinf)) / sizeof(uint32_t)) / 2;
		AICWFDBG(LOGDEBUG, "%s head_t->len:%d base_len:%d\r\n", __func__, head_t->len, base_len);

		if (head_t->len > base_len) {
			patch_info->info_len = base_len;
			memcpy_len = patch_info->info_len + 1;//include ext patch nb
		} else {
			patch_info->info_len = head_t->len;
			memcpy_len = patch_info->info_len;
		}
		head_t->len = patch_info->info_len;
		AICWFDBG(LOGDEBUG, "%s memcpy_len:%d\r\n", __func__, memcpy_len);

		if (patch_info->info_len == 0)
			 return 0;

		memcpy(((patch_info_array) + sizeof(patch_info->info_len)),
			head_t->data,
			memcpy_len * sizeof(uint32_t) * 2);
		AICWFDBG(LOGDEBUG, "%s adid_addrinf:%x addr_adid:%x\r\n", __func__,
			((struct aicbt_patch_info_t *)patch_info_array)->adid_addrinf,
			((struct aicbt_patch_info_t *)patch_info_array)->addr_adid);

		if (patch_info->ext_patch_nb > 0) {
			int index = 0;
			patch_info->ext_patch_param = (uint32_t *)(head_t->data + ((memcpy_len) * 2));

			for (index = 0; index < patch_info->ext_patch_nb; index++) {
				AICWFDBG(LOGDEBUG, "%s id:%x addr:%x\r\n", __func__,
					*(patch_info->ext_patch_param + (index * 2)),
					*(patch_info->ext_patch_param + (index * 2) + 1));
			}
		}
	 }
	 return 0;
}

int aicbt_patch_table_load(struct rwnx_hw *rwnx_hw, struct aicbt_info_t *aicbt_info, struct aicbt_patch_table *head)
{
	struct aicbt_patch_table *p;
	int ret = 0, i;
	uint32_t *data = NULL;

#ifndef CONFIG_LOAD_BT_CONF
	aicbt_info->txpwr_lvl = AICBT_TXPWR_LVL_D80;
#endif

	AICWFDBG(LOGINFO, "%s bt uart baud: %d, flowctrl: %d, lpm_enable: %d, tx_pwr: %d, bt mode:%d.\n", __func__,
			aicbt_info->uart_baud, aicbt_info->uart_flowctrl, aicbt_info->lpm_enable, aicbt_info->txpwr_lvl, aicbt_info->btmode);

	for (p = head; p != NULL; p = p->next) {
		data = p->data;
		if (p->type == AICBT_PT_BTMODE) {
			*(data + 1)  = aicbsp_info.hwinfo < 0;
			*(data + 3)  = aicbsp_info.hwinfo;
			*(data + 5)  = aicbsp_info.cpmode;

			*(data + 7)  = aicbt_info->btmode;
			*(data + 9)  = aicbt_info->btport;
			*(data + 11) = aicbt_info->uart_baud;
			*(data + 13) = aicbt_info->uart_flowctrl;
			*(data + 15) = aicbt_info->lpm_enable;
			*(data + 17) = aicbt_info->txpwr_lvl;
		}

		if (AICBT_PT_VER == p->type) {
			AICWFDBG(LOGINFO, "bt patch version: %s\n", (char *)p->data);
			continue;
		}

		for (i = 0; i < p->len; i++) {
			ret = pcie_send_dbg_mem_write_req(rwnx_hw, *data, *(data + 1));
			if (ret != 0)
				return ret;
			data += 2;
		}
		if (p->type == AICBT_PT_PWRON)
			udelay(500);
	}
	return 0;
}

int aicbt_ext_patch_data_load(struct rwnx_hw *rwnx_hw, struct aicbt_patch_info_t *patch_info, const char *filename)
{
	int ret = 0;
	uint32_t ext_patch_nb = patch_info->ext_patch_nb;
	char ext_patch_file_name[50];
	int index = 0;
	uint32_t id = 0;
	uint32_t addr = 0;

	if (ext_patch_nb > 0) {
		if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2) {
			u8  rem = 1;
			uint32_t mem_w_add = 0;
			uint32_t mem_w_data = 0;
			mem_w_add = 0x40580000;
			mem_w_data = 0x00040220;
			AICWFDBG(LOGDEBUG, "%s addr:0x%x data:0x%x\n", __func__, mem_w_add, mem_w_data);
			ret = aicwf_pcie_tran(rwnx_hw->pcidev, (void *)mem_w_add, &mem_w_data, 4, AIC_TRAN_DRV2EMB, rem);
		}

		for (index = 0; index < patch_info->ext_patch_nb; index++) {
			id = *(patch_info->ext_patch_param + (index * 2));
			addr = *(patch_info->ext_patch_param + (index * 2) + 1);
			memset(ext_patch_file_name, 0, sizeof(ext_patch_file_name));
			sprintf(ext_patch_file_name, "%s%d.bin", filename, id);
			AICWFDBG(LOGDEBUG, "%s ext_patch_file_name:%s ext_patch_id:%x ext_patch_addr:%x\r\n",
					__func__, ext_patch_file_name, id, addr);

			if (pcie_plat_bin_fw_upload_android(rwnx_hw, addr, ext_patch_file_name)) {
				ret = -1;
				break;
			}
		}
	}
	return ret;
}

#ifdef CONFIG_LOAD_BT_CONF
static const struct parse_match_t aicbt_match_tab[] = {
	MATCH_NODE(struct aicbt_info_t, btmode,         "BTMODE"),
	MATCH_NODE(struct aicbt_info_t, btport,         "BTPORT"),
	MATCH_NODE(struct aicbt_info_t, uart_baud,      "UART_BAUD"),
	MATCH_NODE(struct aicbt_info_t, uart_flowctrl,  "UART_FC"),
	MATCH_NODE(struct aicbt_info_t, lpm_enable,     "LPM_ENABLE"),
	MATCH_NODE(struct aicbt_info_t, txpwr_lvl,      "TXPWR_LVL"),
};

int aicbt_parse_config(struct rwnx_hw *rwnx_hw, const char *filename, struct aicbt_info_t *aicbt_info)
{
	int size;
	const struct firmware *fw = NULL;
	int ret = 0;
	char *buffer = NULL;

	char conf[100];
	char *line;
	char *data;
	int  i = 0, err, len = 0;
	long val;

	AICWFDBG(LOGINFO, "rwnx_request_firmware, name: %s\n", filename);
	ret = request_firmware(&fw, filename, NULL);

	if (ret < 0) {
		AICWFDBG(LOGERROR, "Load %s fail\n", filename);
		release_firmware(fw);
		return -1;
	}

	size = fw->size;
	if (size <= 0) {
		AICWFDBG(LOGERROR, "wrong size of firmware file\n");
		release_firmware(fw);
		return -1;
	}

	data = vmalloc(size + 1);
	if (!data) {
		AICWFDBG(LOGERROR, "vmalloc fail\n");
		return -1;
	}

	memcpy(data, fw->data, size);
	buffer = data;
	release_firmware(fw);
	while (1) {
		line = buffer;
		if (*line == 0)
			break;

		while (*buffer != '\r' && *buffer != '\n' && *buffer != 0 && len++ < size)
			buffer++;

		while ((*buffer == '\r' || *buffer == '\n') && len++ < size)
			*buffer++ = 0;

		if (len >= size)
			*buffer = 0;

		// store value to data struct
		for (i = 0; i < sizeof(aicbt_match_tab) / sizeof(aicbt_match_tab[0]); i++) {
			if (parse_key_val(line, aicbt_match_tab[i].keyname, conf) == 0) {
				if (kstrtol(conf, 0, &val))
					err = kstrtol(conf, 16, &val);
				*(unsigned long *)((unsigned long)aicbt_info + aicbt_match_tab[i].offset) = val;
				AICWFDBG(LOGINFO, "%s, %s = %ld\n",  __func__, aicbt_match_tab[i].keyname, val);
				break;
			}
		}
	}

	vfree(data);
	return 0;
}
#endif

#endif

/**
 * rwnx_platform_on() - Start the platform
 *
 * @rwnx_hw: Main driver data
 * @config: Config to restore (NULL if nothing to restore)
 *
 * It starts the platform :
 * - load fw and ucodes
 * - initialize IPC
 * - boot the fw
 * - enable link communication/IRQ
 *
 * Called by 802.11 part
 */
int rwnx_platform_on(struct rwnx_hw *rwnx_hw, void *config)
{
	int ret;
	struct rwnx_plat *rwnx_plat = rwnx_hw->plat;
	(void)ret;

	RWNX_DBG(RWNX_FN_ENTRY_STR);

	if (rwnx_plat->enabled)
		return 0;

	#ifndef CONFIG_ROM_PATCH_EN
	#ifdef CONFIG_DOWNLOAD_FW
	ret = rwnx_plat_fmac_load(rwnx_hw);
	if (ret)
		return ret;
	#endif /* !CONFIG_ROM_PATCH_EN */
	#endif

	rwnx_plat->enabled = true;

	return 0;
}

/**
 * rwnx_platform_off() - Stop the platform
 *
 * @rwnx_hw: Main driver data
 * @config: Updated with pointer to config, to be able to restore it with
 * rwnx_platform_on(). It's up to the caller to free the config. Set to NULL
 * if configuration is not needed.
 *
 * Called by 802.11 part
 */
void rwnx_platform_off(struct rwnx_hw *rwnx_hw, void **config)
{
#if defined(AICWF_USB_SUPPORT) || defined(AICWF_SDIO_SUPPORT)  || defined(AICWF_PCIE_SUPPORT)
	rwnx_hw->plat->enabled = false;
	tasklet_kill(&rwnx_hw->task);
	tasklet_kill(&rwnx_hw->task_txrestart);
	#ifdef CONFIG_RX_TASKLET
	tasklet_kill(&rwnx_hw->task_rx_process);
	#endif
	return ;
#endif

	if (!rwnx_hw->plat->enabled) {
		if (config)
			*config = NULL;
		return;
	}

	if (config)
		*config = rwnx_term_save_config(rwnx_hw->plat);

	rwnx_hw->plat->disable(rwnx_hw);

#ifdef AICWF_PCIE_SUPPORT
	rwnx_ipc_deinit(rwnx_hw);
#endif

	rwnx_platform_reset(rwnx_hw->plat);

	rwnx_hw->plat->enabled = false;
}

/**
 * rwnx_platform_init() - Initialize the platform
 *
 * @rwnx_plat: platform data (already updated by platform driver)
 * @platform_data: Pointer to store the main driver data pointer (aka rwnx_hw)
 *                That will be set as driver data for the platform driver
 * Return: 0 on success, < 0 otherwise
 *
 * Called by the platform driver after it has been probed
 */
int rwnx_platform_init(struct rwnx_plat *rwnx_plat, void **platform_data)
{
	RWNX_DBG(RWNX_FN_ENTRY_STR);

	rwnx_plat->enabled = false;
	g_rwnx_plat = rwnx_plat;

#if defined CONFIG_RWNX_FULLMAC
	return rwnx_cfg80211_init(rwnx_plat, platform_data);
#elif defined CONFIG_RWNX_FHOST
	return rwnx_fhost_init(rwnx_plat, platform_data);
#endif
}

/**
 * rwnx_platform_deinit() - Deinitialize the platform
 *
 * @rwnx_hw: main driver data
 *
 * Called by the platform driver after it is removed
 */
void rwnx_platform_deinit(struct rwnx_hw *rwnx_hw)
{
	RWNX_DBG(RWNX_FN_ENTRY_STR);

#if defined CONFIG_RWNX_FULLMAC
	rwnx_cfg80211_deinit(rwnx_hw);
#elif defined CONFIG_RWNX_FHOST
	rwnx_fhost_deinit(rwnx_hw);
#endif
}

struct device *rwnx_platform_get_dev(struct rwnx_plat *rwnx_plat)
{
#ifdef AICWF_SDIO_SUPPORT
	return rwnx_plat->sdiodev->dev;
#endif
#ifdef AICWF_USB_SUPPORT
	return rwnx_plat->usbdev->dev;
#endif
#ifdef AICWF_PCIE_SUPPORT
	return &rwnx_plat->pcidev->pci_dev->dev;
#endif

	return &(rwnx_plat->pci_dev->dev);
}

#ifndef CONFIG_RWNX_SDM
MODULE_FIRMWARE(RWNX_AGC_FW_NAME);
MODULE_FIRMWARE(RWNX_FCU_FW_NAME);
MODULE_FIRMWARE(RWNX_LDPC_RAM_NAME);
#endif
MODULE_FIRMWARE(RWNX_MAC_FW_NAME);
#ifndef CONFIG_RWNX_TL4
MODULE_FIRMWARE(RWNX_MAC_FW_NAME2);
#endif

