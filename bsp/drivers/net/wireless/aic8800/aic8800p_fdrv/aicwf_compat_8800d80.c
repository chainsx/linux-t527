#include "rwnx_main.h"
#include "rwnx_msg_tx.h"
#include "reg_access.h"
#include "aicwf_compat_8800d80.h"

#define RWNX_PCIE_FW_NAME                      "aic8800d80/pcie/fmacfw_8800d80_pcie.bin"
#define RWNX_PCIE_FW_BT_NAME                   "aic8800d80/pcie/fmacfwbt_8800d80_pcie.bin"
#define RWNX_PCIE_RF_FW_NAME                   "aic8800d80/pcie/lmacfw_rf_pcie.bin"
#define FW_PATCH_BASE_NAME_8800D80_U02         "aic8800d80/pcie/fw_patch_8800d80_u02.bin"
#define FW_EXT_PATCH_BASE_NAME_8800D80_U02     "aic8800d80/pcie/fw_patch_8800d80_u02_ext"
#define FW_ADID_BASE_NAME_8800D80_U02          "aic8800d80/pcie/fw_adid_8800d80_u02.bin"
#define FW_PATCH_TABLE_NAME_8800D80_U02        "aic8800d80/pcie/fw_patch_table_8800d80_u02.bin"
#define FW_POWERLIMIT_NAME_8800D80             "aic8800d80/pcie/aic_powerlimit_8800d80.txt"
#define HW_CONFIG_8800D80                      "aic8800d80/pcie/aichw.conf"

static u32 adaptivity_patch_tbl[][2] = {
	{0x000C, 0x0000320A}, //linkloss_thd
	{0x009C, 0x00000000}, //ac_param_conf
	{0x0168, 0x00010000}, //tx_adaptivity_en
};

static u32 patch_tbl[][2] = {
	#if CFG_USER_EXT_FLAGS_EN
	{0x0188, 0x00000001
		#if CFG_USER_CHAN_MAX_TXPWR_EN
		| USER_CHAN_MAX_TXPWR_EN_FLAG
		#endif
		#if CFG_USER_TX_USE_ANA_F
		| USER_TX_USE_ANA_F_FLAG
		#endif
	}, // user_ext_flags
	#endif
};

static struct bt_patch_file_name bt_patch_name[] = {
	[0] = {
		.fw_adid           = FW_ADID_BASE_NAME_8800D80_U02,
		.fw_patch          = FW_PATCH_BASE_NAME_8800D80_U02,
		.fw_patch_table    = FW_PATCH_TABLE_NAME_8800D80_U02,
		.bt_ext_patch      = FW_EXT_PATCH_BASE_NAME_8800D80_U02,
	},
};

static void rwnx_update_flash(struct rwnx_hw *rwnx_hw)
{
#ifdef CONFIG_UPG_FLASH
	u32 otad_base;
	u32 gen2_base;
	int ret;
#endif
	u32 bond_id;

	aicwf_pcie_tran(rwnx_hw->pcidev, (void *)0x40500004, &bond_id, 4, AIC_TRAN_EMB2DRV, 1);
	AICWFDBG(LOGINFO, "0x40500004 bond_id %x >>17 %x\n", bond_id, (bond_id >> 17));

	if (((bond_id >> 17) & 0x01UL) == 0x00UL) {
		chip_mcu_id = 1;
	}

	AICWFDBG(LOGINFO, "M chip is %u\n ", chip_mcu_id);

#ifdef CONFIG_UPG_FLASH
	if (chip_mcu_id) {
		aicwf_pcie_tran(rwnx_hw->pcidev, (void *)0x8006000, &otad_base, 4, AIC_TRAN_EMB2DRV, 1);
		AICWFDBG(LOGINFO, "0x8006000 otad_base %x fw_flsupg %d\n", otad_base, fw_flsupg);
		aicwf_pcie_tran(rwnx_hw->pcidev, (void *)0x807f074, &gen2_base, 4, AIC_TRAN_EMB2DRV, 1);
		AICWFDBG(LOGINFO, "0x807f074 gen2_base %x, FLASH_SET_GEN2 %d FLASH_SET_GEN1 %d\n", gen2_base, FLASH_SET_GEN2, FLASH_SET_GEN1);

		//only use for M80x2P
		if (otad_base != 0x474d4943) {
			fw_flsupg = 1;
			if (FLASH_SET_GEN2) {
				fw_flggen2 = 1;
			}
			if (FLASH_SET_GEN1) {
				fw_flggen1 = 1;
			}
		} else {
			if (FLASH_SET_GEN2) {
				if (((gen2_base >> 24) & 0xFF) != 0xA3)
					fw_flggen2 = 1;
			}
			if (FLASH_SET_GEN1) {
				if (((gen2_base >> 24) & 0xFF) == 0xA3)
					fw_flggen1 = 1;
			}
		}
	}
#endif
}

static int aicwifi_patch_config(struct rwnx_hw *rwnx_hw)
{
	u32 fw_addr = testmode ? RAM_LMAC_RF_FW_ADDR : RAM_FMAC_FW_ADDR;
	const u32 rd_patch_addr = fw_addr + 0x0198;
	u32 aic_patch_addr;
	u32 config_base, aic_patch_str_base;
	#if (NEW_PATCH_BUFFER_MAP)
	u32 patch_buff_addr, patch_buff_base, rd_version_addr, rd_version_val;
	#endif
	#ifdef CONFIG_USB_BT
	uint32_t start_addr = 0x0017B000;
	#else
	uint32_t start_addr = 0x00176000;
	#endif
	u32 patch_addr = start_addr;
	u32 patch_cnt = sizeof(patch_tbl)/sizeof(u32)/2;
	int cnt = 0;
	//adap test
	int adap_patch_cnt = 0;
	int tmp_cnt = 0;
	int tbl_idx = 0;
	volatile u32 *patch_0 = NULL;
	volatile u32 *patch_1 = NULL;
	volatile u32 *patch_2 = NULL;
	volatile u32 *patch_3 = NULL;

	if (adap_test) {
		adap_patch_cnt = sizeof(adaptivity_patch_tbl)/sizeof(u32)/2;
	}

	aic_patch_addr = rd_patch_addr + 8;
	config_base = *((volatile u32 *) (rwnx_hw->pcidev->pci_bar0_vaddr + rd_patch_addr));
	aic_patch_str_base = *((volatile u32 *) (rwnx_hw->pcidev->pci_bar0_vaddr + aic_patch_addr));
	AICWFDBG(LOGINFO, "%s: cfg_base:%x,patch_str_base:%x,adap_test=%d\n", __func__, config_base, aic_patch_str_base, adap_test);

	#if (NEW_PATCH_BUFFER_MAP)
	rd_version_addr = fw_addr + 0x01C;
	rd_version_val = *((volatile u32 *) (rwnx_hw->pcidev->pci_bar0_vaddr + rd_version_addr));
	AICWFDBG(LOGINFO, "rd_version_val=%08X\n", rd_version_val);
	rwnx_hw->pcidev->fw_version_uint = rd_version_val;
	if (rd_version_val > 0x06090100) {
		patch_buff_addr = rd_patch_addr + 12;
		patch_buff_base = *((volatile u32 *) (rwnx_hw->pcidev->pci_bar0_vaddr + patch_buff_addr));
		patch_addr = start_addr = patch_buff_base;
		AICWFDBG(LOGINFO, "%s: patch_buff_base:%x\n", __func__, patch_buff_base);
	}
	#endif

	patch_0 = (volatile u32 *) (rwnx_hw->pcidev->pci_bar0_vaddr + AIC_PATCH_ADDR(magic_num));
	patch_1 = (volatile u32 *) (rwnx_hw->pcidev->pci_bar0_vaddr + AIC_PATCH_ADDR(magic_num_2));
	if ((patch_cnt + adap_patch_cnt) == 0) {
		*patch_0 = 0;
		*patch_1 = 0;
		return 0;
	}
	*patch_0 = AIC_PATCH_MAGIG_NUM;
	*patch_1 = AIC_PATCH_MAGIG_NUM_2;
	patch_2 = (volatile u32 *)(rwnx_hw->pcidev->pci_bar0_vaddr + AIC_PATCH_ADDR(pair_start));
	*patch_2 = patch_addr;
	patch_3 = (volatile u32 *)(rwnx_hw->pcidev->pci_bar0_vaddr + AIC_PATCH_ADDR(pair_count));
	*patch_3 = patch_cnt + adap_patch_cnt;

	for (cnt = 0; cnt < patch_cnt; cnt++) {
		volatile u32 *patch_4 = NULL;
		volatile u32 *patch_5 = NULL;
		patch_4 = (volatile u32 *)(rwnx_hw->pcidev->pci_bar0_vaddr + start_addr + 8 * cnt);
		*patch_4 = patch_tbl[cnt][0]+config_base;
		patch_5 = (volatile u32 *) (rwnx_hw->pcidev->pci_bar0_vaddr + start_addr + 8 * cnt + 4);
		*patch_5 = patch_tbl[cnt][1];
		AICWFDBG(LOGDEBUG, "0x%x: 0x%x\n", start_addr + 8 * cnt, patch_tbl[cnt][0] + config_base);
	}
	if (adap_test) {
		tmp_cnt = patch_cnt + adap_patch_cnt;
		for (cnt = patch_cnt; cnt < tmp_cnt; cnt++) {
			volatile u32 *patch_6 = NULL;
			volatile u32 *patch_7 = NULL;
			tbl_idx = cnt - patch_cnt;
			patch_6 = (volatile u32 *)(rwnx_hw->pcidev->pci_bar0_vaddr + start_addr + 8 * cnt);
			*patch_6 = adaptivity_patch_tbl[tbl_idx][0] + config_base;
			patch_7 = (volatile u32 *)(rwnx_hw->pcidev->pci_bar0_vaddr + start_addr + 8 * cnt + 4);
			*patch_7 = adaptivity_patch_tbl[tbl_idx][1];
		}
	}

	for (cnt = 0; cnt < 4; cnt++) {
		volatile u32 *patch_8 = (volatile u32 *)(rwnx_hw->pcidev->pci_bar0_vaddr + AIC_PATCH_ADDR(block_size[cnt]));
		*patch_8 = 0;
	}

	return 0;
}

static int aicbt_init(struct rwnx_hw *rwnx_hw)
{
	int err, cnt;
	u32 bt_init_st;
	ktime_t time_ini, time_cur;

	struct aicbt_patch_table *head = NULL;
	struct aicbt_patch_info_t patch_info = {
		.info_len          = 0,
		.adid_addrinf      = 0,
		.addr_adid         = 0,
		.patch_addrinf     = 0,
		.addr_patch        = 0,
		.reset_addr        = 0,
		.reset_val         = 0,
		.adid_flag_addr    = 0,
		.adid_flag         = 0,
	};

	struct aicbt_info_t aicbt_info = {
		.btmode        = AICBT_BTMODE_DEFAULT,
		.btport        = AICBT_BTPORT_DEFAULT,
		.uart_baud     = AICBT_UART_BAUD_DEFAULT,
		.uart_flowctrl = AICBT_UART_FC_DEFAULT,
		.lpm_enable    = AICBT_LPM_ENABLE_DEFAULT,
		.txpwr_lvl     = AICBT_TXPWR_LVL_DEFAULT,
	};

	struct bt_patch_file_name *bt_patch_file = NULL;

#ifdef CONFIG_LOAD_BT_CONF
	aicbt_parse_config(rwnx_hw, HW_CONFIG_8800D80, &aicbt_info);
#endif

	aicwf_pcie_tran(rwnx_hw->pcidev, (void *)BT_INIT_DONE_ADDR, &bt_init_st, 4, AIC_TRAN_EMB2DRV, 1);
	AICWFDBG(LOGINFO, "bt init %x\n", bt_init_st);

	if ((bt_init_st & BT_INIT_DONE_BIT) != 0)
		return 0;

	AICWFDBG(LOGINFO, "%s chip_id:%d\r\n", __func__, rwnx_hw->pcidev->chip_id);
	bt_patch_file = &bt_patch_name[0];

	head = aicbt_patch_table_alloc(bt_patch_file->fw_patch_table);
	if (head == NULL) {
		AICWFDBG(LOGERROR, "aicbt_patch_table_alloc fail\n");
		return -1;
	}

	patch_info.addr_adid = FW_RAM_ADID_BASE_ADDR_8800D80_U02;
	patch_info.addr_patch = FW_RAM_PATCH_BASE_ADDR_8800D80_U02;

	aicbt_patch_info_unpack(&patch_info, head);
	if (patch_info.info_len == 0) {
		AICWFDBG(LOGERROR, "%s, aicbt_patch_info_unpack fail\n", __func__);
		return -1;
	}

	AICWFDBG(LOGINFO, "addr_adid 0x%x, addr_patch 0x%x\n", patch_info.addr_adid, patch_info.addr_patch);

	if (pcie_plat_bin_fw_upload_android(rwnx_hw, patch_info.addr_adid, (char *)bt_patch_file->fw_adid)) {
		return -1;
	}

	if (pcie_plat_bin_fw_upload_android(rwnx_hw, patch_info.addr_patch, (char *)bt_patch_file->fw_patch)) {
		return -1;
	}

	if (aicbt_ext_patch_data_load(rwnx_hw, &patch_info, bt_patch_file->bt_ext_patch)) {
		return -1;
	}

	if (aicbt_patch_table_load(rwnx_hw, &aicbt_info, head)) {
		return -1;
	}

	cnt = 0;
	time_ini = ktime_get_boottime();
	while ((bt_init_st & BT_INIT_DONE_BIT) == 0) {
		err = aicwf_pcie_tran(rwnx_hw->pcidev, (void *)BT_INIT_DONE_ADDR, &bt_init_st, 4, AIC_TRAN_EMB2DRV, 1);
		cnt++;
		time_cur = ktime_get_boottime();
		if (err || (time_cur - time_ini) > BT_INIT_TO_NS) {
			break;
		}
	}
	AICWFDBG(LOGINFO, "bt init done: err = %d, cnt = %d, st = %x, use %lld ns\n", err, cnt, bt_init_st, (long long)(time_cur - time_ini));

	return 0;
}

int aicwf_8800d80_fw_init(struct rwnx_hw *rwnx_hw)
{
	int ret = 0;
#if (defined(CONFIG_NO_FIRMWARE_RELOAD) || defined(CONFIG_LOWPOWER))
	u32 sysctl;
#endif
	u32 fw_addr = testmode ? RAM_LMAC_RF_FW_ADDR : RAM_FMAC_FW_ADDR;

	RWNX_DBG(RWNX_FN_ENTRY_STR);

	rwnx_update_flash(rwnx_hw);

#if (defined(CONFIG_NO_FIRMWARE_RELOAD) || defined(CONFIG_LOWPOWER))
	*(volatile uint32_t *)&rwnx_hw->ipc_env->shared->fw_init_done = 2;
	writel(4, rwnx_hw->pcidev->emb_tpci + 0x0ec); //generate an empty int
	mdelay(2);
	sysctl = readl(rwnx_hw->pcidev->emb_sctl + 0x10);
	if ((testmode == 0) && (sysctl == 0x6)) {
		aicwifi_patch_config(rwnx_hw);
		AICWFDBG(LOGINFO, "fw already load\n");
		pcie_reset_firmware(rwnx_hw, fw_addr);
#ifdef CONFIG_POWER_LIMIT
		rwnx_plat_powerlimit_load(rwnx_hw, FW_POWERLIMIT_NAME_8800D80);
#endif
		return 0;
	}
#endif

#ifdef CONFIG_USE_BT
	aicbt_init(rwnx_hw);
#endif

	if (testmode == 0)
#ifdef CONFIG_USB_BT
		ret = pcie_plat_bin_fw_upload_android(rwnx_hw, fw_addr, RWNX_PCIE_FW_BT_NAME);
#else
		ret = pcie_plat_bin_fw_upload_android(rwnx_hw, fw_addr, RWNX_PCIE_FW_NAME);
#endif
	else
		ret = pcie_plat_bin_fw_upload_android(rwnx_hw, fw_addr, RWNX_PCIE_RF_FW_NAME);

	if (ret)
		return ret;

	aicwifi_patch_config(rwnx_hw);
	pcie_reset_firmware(rwnx_hw, fw_addr);
	return 0;
}

int aicwf_set_rf_config_8800d80(struct rwnx_hw *rwnx_hw, struct mm_set_rf_calib_cfm *cfm)
{
	if (rwnx_send_txpwr_lvl_v3_req(rwnx_hw))
		return -1;

	if (rwnx_send_txpwr_ofst2x_req(rwnx_hw))
		return -1;

	if (rwnx_send_txpwr_lvl_adj_req(rwnx_hw))
		return -1;

#ifndef CONFIG_RF_CALIB_LATER
	if (rwnx_send_rf_calib_req(rwnx_hw, cfm))
		return -1;
#endif

	return 0;
}
