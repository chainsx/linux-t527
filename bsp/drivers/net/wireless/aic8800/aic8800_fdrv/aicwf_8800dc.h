#ifndef AICWF_8800DC_H_
#define AICWF_8800DC_H_

#include <linux/types.h>
#include "rwnx_defs.h"
#include "aic_bsp_export.h"

#ifdef CONFIG_DPD
int aicwf_fdrv_dpd_result_apply_8800dc(struct rwnx_hw * rwnx_hw, rf_misc_ram_lite_t * dpd_res);
#ifndef CONFIG_FORCE_DPD_CALIB
int aicwf_fdrv_dpd_result_load_8800dc(struct rwnx_hw *rwnx_hw, rf_misc_ram_lite_t *dpd_res);
#endif
#endif
int aicwf_fdrv_misc_ram_init_8800dc(struct rwnx_hw *rwnx_hw);

int aicwf_set_rf_config_8800dc(struct rwnx_hw *rwnx_hw, struct mm_set_rf_calib_cfm *cfm);

#ifdef AICWF_USB_SUPPORT

#include "lmac_msg.h"
#include "rwnx_msg_tx.h"
#include "aicwf_usb.h"
#include <linux/firmware.h>

typedef u32 (*array2_tbl_t)[2];
typedef u32 (*array3_tbl_t)[3];

struct aicwf_firmware {
	const char *desc;
	const char *bt_adid;
	const char *bt_patch;
	const char *bt_table;
	const char *wl_fw;
	const char *wl_table;
	const char *wl_calib;
};

int rwnx_plat_bin_fw_upload_android(struct rwnx_hw *aicdev, u32 fw_addr, const char *filename);
int aicfw_8800dc_fw_init(struct rwnx_hw *rwnx_hw);

#endif

#endif
