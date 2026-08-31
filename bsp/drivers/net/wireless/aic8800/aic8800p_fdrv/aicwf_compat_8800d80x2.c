#include "rwnx_main.h"
#include "rwnx_msg_tx.h"
#include "reg_access.h"
#include "aicwf_compat_8800d80x2.h"

#define RWNX_8800D80X2_PCIE_FLASH_FW_NAME      "aic8800d80x2/pcie/fw_flsupg_m80x2p.bin"
#define RWNX_8800D80X2_PCIE_FLASHGEN1_FW_NAME  "aic8800d80x2/pcie/fw_flsgen1_m80x2p.bin"
#define RWNX_8800D80X2_PCIE_FLASHGEN2_FW_NAME  "aic8800d80x2/pcie/fw_flsgen2_m80x2p.bin"

#ifdef CONFIG_FOR_IPCAM
#define RWNX_8800D80X2_PCIE_FW_NAME            "aic8800d80x2/pcie/fmacfw_8800d80x2_pcie_ipc.bin"
#define RWNX_8800D80X2_PCIE_FW_BT_NAME         "aic8800d80x2/pcie/fmacfwbt_8800d80x2_pcie_ipc.bin"
#else
#define RWNX_8800D80X2_PCIE_FW_NAME            "aic8800d80x2/pcie/fmacfw_8800d80x2_pcie.bin"
#define RWNX_8800D80X2_PCIE_FW_BT_NAME         "aic8800d80x2/pcie/fmacfwbt_8800d80x2_pcie.bin"
#endif
#define RWNX_8800D80X2_PCIE_RF_FW_NAME         "aic8800d80x2/pcie/lmacfw_rf_8800d80x2_pcie.bin"
#define FW_PATCH_BASE_NAME_8800D80X2_U05       "aic8800d80x2/pcie/fw_patch_8800d80x2_u05.bin"
#define FW_EXT_PATCH_BASE_NAME_8800D80X2_U05   "aic8800d80x2/pcie/fw_patch_8800d80x2_u05_ext"
#define FW_ADID_BASE_NAME_8800D80X2_U05        "aic8800d80x2/pcie/fw_adid_8800d80x2_u05.bin"
#define FW_PATCH_TABLE_NAME_8800D80X2_U05      "aic8800d80x2/pcie/fw_patch_table_8800d80x2_u05.bin"

#define FW_POWERLIMIT_NAME_8800D80X2           "aic8800d80x2/pcie/aic_powerlimit_8800d80x2.txt"
#define HW_CONFIG_8800D80X2                    "aic8800d80x2/pcie/aichw.conf"

static u32 adaptivity_patch_tbl[][2] = {
	{0x000C, 0x0000320A}, //linkloss_thd
	{0x009C, 0x00000000}, //ac_param_conf
	{0x01D0, 0x00010000}, //tx_adaptivity_en
};

static u32 patch_tbl[][2] = {
	{0x01f0, 0x00000001
		#if CFG_USER_CHAN_MAX_TXPWR_EN
		| USER_CHAN_MAX_TXPWR_EN_FLAG
		#endif
		#if CFG_USER_TX_USE_ANA_F
		| USER_TX_USE_ANA_F_FLAG
		#endif
		#if CFG_USER_APM_PRBRSP_OFFLOAD_DISABLE
		| USER_APM_PRBRSP_OFFLOAD_DISABLE_FLAG
		#endif
	}, // user_ext_flags
#ifdef CONFIG_LOWPOWER
#ifdef CONFIG_DELAYED_BA
	{ 0x22c, 0x08010101}, //08-24Mbps 06-12Mbps 04-6Mbps 00-1Mbps
#else
	{ 0x22c, 0x00000101},
#endif
#else
	{ 0x22c, 0},
#endif
	#ifdef CONFIG_FLASH_CALRES
	{0x0234, 0x0000004F}, // cal_res_stored_in_flash_flags
	#endif

	#ifdef CONFIG_RADAR_OR_IR_DETECT
	{0x0204, 0x01010100},//radar
	#endif
	#ifdef CONFIG_AP_LOWPOWER
	{0x0238, 0x0100000F},
	#endif
};

static struct bt_patch_file_name bt_patch_name[] = {
	[0] = {
		.fw_adid           = FW_ADID_BASE_NAME_8800D80X2_U05,
		.fw_patch          = FW_PATCH_BASE_NAME_8800D80X2_U05,
		.fw_patch_table    = FW_PATCH_TABLE_NAME_8800D80X2_U05,
		.bt_ext_patch      = FW_EXT_PATCH_BASE_NAME_8800D80X2_U05,
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

		printk("fw_flsupg %d\n", fw_flsupg);
		if (fw_flsupg) {
			ret = pcie_plat_bin_fw_upload_android(rwnx_hw, RAM_FMAC_FW_ADDR, RWNX_8800D80X2_PCIE_FLASH_FW_NAME);
			pcie_reset_firmware(rwnx_hw, RAM_FMAC_FW_ADDR);
			fw_flsupg = 0;
			mdelay(1000);
		}

		printk("fw_flggen2 %d\n", fw_flggen2);
		if (fw_flggen2) {
			ret = pcie_plat_bin_fw_upload_android(rwnx_hw, RAM_FMAC_FW_ADDR, RWNX_8800D80X2_PCIE_FLASHGEN2_FW_NAME);
			pcie_reset_firmware(rwnx_hw, RAM_FMAC_FW_ADDR);
			fw_flggen2 = 0;
			mdelay(100);
		}

		printk("fw_flggen1 %d\n", fw_flggen1);
		if (fw_flggen1) {
			ret = pcie_plat_bin_fw_upload_android(rwnx_hw, RAM_FMAC_FW_ADDR, RWNX_8800D80X2_PCIE_FLASHGEN1_FW_NAME);
			pcie_reset_firmware(rwnx_hw, RAM_FMAC_FW_ADDR);
			fw_flggen1 = 0;
			mdelay(100);
		}
	}
#endif
}

static int aicwifi_patch_config(struct rwnx_hw *rwnx_hw)
{
	u32 fw_addr = testmode ? RAM_LMAC_RF_FW_ADDR : RAM_FMAC_FW_ADDR;
	const u32 rd_patch_addr = fw_addr + 0x1a8; // 0x0198; g_wifi_settings
	u32 aic_patch_addr;
	u32 config_base, aic_patch_str_base;
	u32 patch_buff_addr, patch_buff_base;
	uint32_t start_addr;
	u32 patch_addr;
	u32 patch_cnt = sizeof(patch_tbl)/sizeof(u32)/2;
	int cnt = 0;
	//adap test
	int adap_patch_cnt = 0;
	int tmp_cnt = 0;
	int tbl_idx = 0;
	u32 zero = 0;
	u32 temp;

	if (adap_test) {
		adap_patch_cnt = sizeof(adaptivity_patch_tbl)/sizeof(u32)/2;
	}

	aic_patch_addr = rd_patch_addr + 8;//g_aic_patch
	aicwf_pcie_tran(rwnx_hw->pcidev, (void *)rd_patch_addr, &config_base, 4, AIC_TRAN_EMB2DRV, 1);
	aicwf_pcie_tran(rwnx_hw->pcidev, (void *)aic_patch_addr, &aic_patch_str_base, 4, AIC_TRAN_EMB2DRV, 1);
	AICWFDBG(LOGINFO, "%s: cfg_base:%x,patch_str_base:%x,adap_test=%d\n", __func__, config_base, aic_patch_str_base, adap_test);

	if (1) {
		patch_buff_addr = rd_patch_addr + 12; //2k start addr
		aicwf_pcie_tran(rwnx_hw->pcidev, (void *)patch_buff_addr, &patch_buff_base, 4, AIC_TRAN_EMB2DRV, 1);
		patch_addr = start_addr = patch_buff_base;
		AICWFDBG(LOGINFO, "%s: patch_buff_base:%x\n", __func__, patch_buff_base);
	}

	if ((patch_cnt + adap_patch_cnt) == 0) {
		aicwf_pcie_tran(rwnx_hw->pcidev, (void *)AIC_PATCH_ADDR(magic_num), &zero, 4, AIC_TRAN_DRV2EMB, 1);
		aicwf_pcie_tran(rwnx_hw->pcidev, (void *)AIC_PATCH_ADDR(magic_num_2), &zero, 4, AIC_TRAN_DRV2EMB, 1);
		return 0;
	}

	temp = AIC_PATCH_MAGIG_NUM;
	aicwf_pcie_tran(rwnx_hw->pcidev, (void *)AIC_PATCH_ADDR(magic_num), &temp, 4, AIC_TRAN_DRV2EMB, 1);
	temp = AIC_PATCH_MAGIG_NUM_2;
	aicwf_pcie_tran(rwnx_hw->pcidev, (void *)AIC_PATCH_ADDR(magic_num_2), &temp, 4, AIC_TRAN_DRV2EMB, 1);

	temp = patch_addr;
	aicwf_pcie_tran(rwnx_hw->pcidev, (void *)AIC_PATCH_ADDR(pair_start), &temp, 4, AIC_TRAN_DRV2EMB, 1);
	temp = patch_cnt + adap_patch_cnt;;
	aicwf_pcie_tran(rwnx_hw->pcidev, (void *)AIC_PATCH_ADDR(pair_count), &temp, 4, AIC_TRAN_DRV2EMB, 1);

	for (cnt = 0; cnt < patch_cnt; cnt++) {
		temp = patch_tbl[cnt][0]+config_base;
		aicwf_pcie_tran(rwnx_hw->pcidev, (void *)(start_addr+8*cnt), &temp, 4, AIC_TRAN_DRV2EMB, 1);
		temp = patch_tbl[cnt][1];
		aicwf_pcie_tran(rwnx_hw->pcidev, (void *)(start_addr+8*cnt + 4), &temp, 4, AIC_TRAN_DRV2EMB, 1);
		AICWFDBG(LOGTRACE, "0x%x: 0x%x\n", start_addr + 8 * cnt, patch_tbl[cnt][0] + config_base);
	}
	if (adap_test) {
		tmp_cnt = patch_cnt + adap_patch_cnt;
		for (cnt = patch_cnt; cnt < tmp_cnt; cnt++) {
			tbl_idx = cnt - patch_cnt;
			temp = adaptivity_patch_tbl[tbl_idx][0] + config_base;
			aicwf_pcie_tran(rwnx_hw->pcidev, (void *)(start_addr + 8 * cnt), &temp, 4, AIC_TRAN_DRV2EMB, 1);
			temp = adaptivity_patch_tbl[tbl_idx][1];
			aicwf_pcie_tran(rwnx_hw->pcidev, (void *)(start_addr + 8 * cnt + 4), &temp, 4, AIC_TRAN_DRV2EMB, 1);
		}
	}

	for (cnt = 0; cnt < 4; cnt++) {
		aicwf_pcie_tran(rwnx_hw->pcidev, (void *)(AIC_PATCH_ADDR(block_size[cnt])), &zero, 4, AIC_TRAN_DRV2EMB, 1);
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
	aicbt_parse_config(rwnx_hw, HW_CONFIG_8800D80X2, &aicbt_info);
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

int aicwf_8800d80x2_fw_init(struct rwnx_hw *rwnx_hw)
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
		rwnx_plat_powerlimit_load(rwnx_hw, FW_POWERLIMIT_NAME_8800D80X2);
#endif
		return 0;
	}
#endif

#ifdef CONFIG_USE_BT
	aicbt_init(rwnx_hw);
#endif

	if (testmode == 0) {
		ret = pcie_plat_bin_fw_upload_android(rwnx_hw, fw_addr, RWNX_8800D80X2_PCIE_FW_NAME);
	} else {
		ret = pcie_plat_bin_fw_upload_android(rwnx_hw, fw_addr, RWNX_8800D80X2_PCIE_RF_FW_NAME);
	}

	if (ret)
		return ret;

	aicwifi_patch_config(rwnx_hw);
	pcie_reset_firmware(rwnx_hw, fw_addr);
	return 0;
}

int aicwf_set_rf_config_8800d80x2(struct rwnx_hw *rwnx_hw, struct mm_set_rf_calib_cfm *cfm)
{
	if (rwnx_send_txpwr_lvl_v4_req(rwnx_hw))
		return -1;

	if (rwnx_send_txpwr_ofst2x_v2_req(rwnx_hw))
		return -1;

	if (rwnx_send_txpwr_lvl_adj_req(rwnx_hw))
		return -1;

#ifndef CONFIG_RF_CALIB_LATER
	if (rwnx_send_rf_calib_req(rwnx_hw, cfm))
		return -1;
#endif

	return 0;
}
