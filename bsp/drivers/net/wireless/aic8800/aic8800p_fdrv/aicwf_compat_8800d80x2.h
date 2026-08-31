#ifndef _AICWF_COMPAT_8800D80X2_H_
#define _AICWF_COMPAT_8800D80X2_H_

#include <linux/types.h>

int aicwf_set_rf_config_8800d80x2(struct rwnx_hw *rwnx_hw, struct mm_set_rf_calib_cfm *cfm);
int aicwf_8800d80x2_fw_init(struct rwnx_hw *rwnx_hw);

#endif
