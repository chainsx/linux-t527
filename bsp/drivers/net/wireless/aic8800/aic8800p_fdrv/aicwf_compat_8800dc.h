#ifndef AICWF_COMPAT_8800DC_H
#define AICWF_COMPAT_8800DC_H

#include <linux/types.h>

int aicwf_set_rf_config_8800dc(struct rwnx_hw *rwnx_hw, struct mm_set_rf_calib_cfm *cfm);

#endif
