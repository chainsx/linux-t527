/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * core function of edp driver
 *
 * Copyright (c) 2007-2022 Allwinnertech Co., Ltd.
 * Author: huangyongxing <huangyongxing@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <crypto/hash.h>
#include <crypto/sha1_base.h>
#include <crypto/sha1.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <crypto/sha2.h>
#include "sunxi_dptx_hdcp.h"
#include "sunxi_dptx_hdcp2x_crypt.h"

/* use for facsimile test key test */
//#define HDCP2X_USE_FACSIMILE

#define HDCP1X_REAUTH_CNT 5

#define HDCP2X_REAUTH_CNT 3

static s32 hdcp2_read_parse_rxstatus(struct sunxi_dptx_hdcp2_info *info);

/* Public transmitter DCP LLC key - n=384 bytes, e=1 byte */

/*
 * Reference:
 * https://www.digital-cp.com/sites/default/files/HDCP%20on%20HDMI%20Specification%20Rev2_3.pdf
 * Table B.1
 */
static const u8 hdcp2x_tx_kpubdpc[HDCP2X_TX_KPUB_DCP_LLC_N_SIZE
				 + HDCP2X_TX_KPUB_DCP_LLC_E_SIZE] = {
	0xB0, 0xE9, 0xAA, 0x45, 0xF1, 0x29, 0xBA, 0x0A,
	0x1C, 0xBE, 0x17, 0x57, 0x28, 0xEB, 0x2B, 0x4E,
	0x8F, 0xD0, 0xC0, 0x6A, 0xAD, 0x79, 0x98, 0x0F,
	0x8D, 0x43, 0x8D, 0x47, 0x04, 0xB8, 0x2B, 0xF4,
	0x15, 0x21, 0x56, 0x19, 0x01, 0x40, 0x01, 0x3B,
	0xD0, 0x91, 0x90, 0x62, 0x9E, 0x89, 0xC2, 0x27,
	0x8E, 0xCF, 0xB6, 0xDB, 0xCE, 0x3F, 0x72, 0x10,
	0x50, 0x93, 0x8C, 0x23, 0x29, 0x83, 0x7B, 0x80,
	0x64, 0xA7, 0x59, 0xE8, 0x61, 0x67, 0x4C, 0xBC,
	0xD8, 0x58, 0xB8, 0xF1, 0xD4, 0xF8, 0x2C, 0x37,
	0x98, 0x16, 0x26, 0x0E, 0x4E, 0xF9, 0x4E, 0xEE,
	0x24, 0xDE, 0xCC, 0xD1, 0x4B, 0x4B, 0xC5, 0x06,
	0x7A, 0xFB, 0x49, 0x65, 0xE6, 0xC0, 0x00, 0x83,
	0x48, 0x1E, 0x8E, 0x42, 0x2A, 0x53, 0xA0, 0xF5,
	0x37, 0x29, 0x2B, 0x5A, 0xF9, 0x73, 0xC5, 0x9A,
	0xA1, 0xB5, 0xB5, 0x74, 0x7C, 0x06, 0xDC, 0x7B,
	0x7C, 0xDC, 0x6C, 0x6E, 0x82, 0x6B, 0x49, 0x88,
	0xD4, 0x1B, 0x25, 0xE0, 0xEE, 0xD1, 0x79, 0xBD,
	0x39, 0x85, 0xFA, 0x4F, 0x25, 0xEC, 0x70, 0x19,
	0x23, 0xC1, 0xB9, 0xA6, 0xD9, 0x7E, 0x3E, 0xDA,
	0x48, 0xA9, 0x58, 0xE3, 0x18, 0x14, 0x1E, 0x9F,
	0x30, 0x7F, 0x4C, 0xA8, 0xAE, 0x53, 0x22, 0x66,
	0x2B, 0xBE, 0x24, 0xCB, 0x47, 0x66, 0xFC, 0x83,
	0xCF, 0x5C, 0x2D, 0x1E, 0x3A, 0xAB, 0xAB, 0x06,
	0xBE, 0x05, 0xAA, 0x1A, 0x9B, 0x2D, 0xB7, 0xA6,
	0x54, 0xF3, 0x63, 0x2B, 0x97, 0xBF, 0x93, 0xBE,
	0xC1, 0xAF, 0x21, 0x39, 0x49, 0x0C, 0xE9, 0x31,
	0x90, 0xCC, 0xC2, 0xBB, 0x3C, 0x02, 0xC4, 0xE2,
	0xBD, 0xBD, 0x2F, 0x84, 0x63, 0x9B, 0xD2, 0xDD,
	0x78, 0x3E, 0x90, 0xC6, 0xC5, 0xAC, 0x16, 0x77,
	0x2E, 0x69, 0x6C, 0x77, 0xFD, 0xED, 0x8A, 0x4D,
	0x6A, 0x8C, 0xA3, 0xA9, 0x25, 0x6C, 0x21, 0xFD,
	0xB2, 0x94, 0x0C, 0x84, 0xAA, 0x07, 0x29, 0x26,
	0x46, 0xF7, 0x9B, 0x3A, 0x19, 0x87, 0xE0, 0x9F,
	0xEB, 0x30, 0xA8, 0xF5, 0x64, 0xEB, 0x07, 0xF1,
	0xE9, 0xDB, 0xF9, 0xAF, 0x2C, 0x8B, 0x69, 0x7E,
	0x2E, 0x67, 0x39, 0x3F, 0xF3, 0xA6, 0xE5, 0xCD,
	0xDA, 0x24, 0x9B, 0xA2, 0x78, 0x72, 0xF0, 0xA2,
	0x27, 0xC3, 0xE0, 0x25, 0xB4, 0xA1, 0x04, 0x6A,
	0x59, 0x80, 0x27, 0xB5, 0xDA, 0xB4, 0xB4, 0x53,
	0x97, 0x3B, 0x28, 0x99, 0xAC, 0xF4, 0x96, 0x27,
	0x0F, 0x7F, 0x30, 0x0C, 0x4A, 0xAF, 0xCB, 0x9E,
	0xD8, 0x71, 0x28, 0x24, 0x3E, 0xBC, 0x35, 0x15,
	0xBE, 0x13, 0xEB, 0xAF, 0x43, 0x01, 0xBD, 0x61,
	0x24, 0x54, 0x34, 0x9F, 0x73, 0x3E, 0xB5, 0x10,
	0x9F, 0xC9, 0xFC, 0x80, 0xE8, 0x4D, 0xE3, 0x32,
	0x96, 0x8F, 0x88, 0x10, 0x23, 0x25, 0xF3, 0xD3,
	0x3E, 0x6E, 0x6D, 0xBB, 0xDC, 0x29, 0x66, 0xEB,
	0x03
};

static const u8 hdcp2x_tx_test_kpubdpc[HDCP2X_TX_KPUB_DCP_LLC_N_SIZE
				 + HDCP2X_TX_KPUB_DCP_LLC_E_SIZE] = {
	0xa2, 0xc7, 0x55, 0x57, 0x54, 0xcb, 0xaa, 0xa7, 0x7a, 0x27, 0x92, 0xc3, 0x1a, 0x6d, 0xc2, 0x31,
	0xcf, 0x12, 0xc2, 0x24, 0xbf, 0x89, 0x72, 0x46, 0xa4, 0x8d, 0x20, 0x83, 0xb2, 0xdd, 0x04, 0xda,
	0x7e, 0x01, 0xa9, 0x19, 0xef, 0x7e, 0x8c, 0x47, 0x54, 0xc8, 0x59, 0x72, 0x5c, 0x89, 0x60, 0x62,
	0x9f, 0x39, 0xd0, 0xe4, 0x80, 0xca, 0xa8, 0xd4, 0x1e, 0x91, 0xe3, 0x0e, 0x2c, 0x77, 0x55, 0x6d,
	0x58, 0xa8, 0x9e, 0x3e, 0xf2, 0xda, 0x78, 0x3e, 0xba, 0xd1, 0x05, 0x37, 0x07, 0xf2, 0x88, 0x74,
	0x0c, 0xbc, 0xfb, 0x68, 0xa4, 0x7a, 0x27, 0xad, 0x63, 0xa5, 0x1f, 0x67, 0xf1, 0x45, 0x85, 0x16,
	0x49, 0x8a, 0xe6, 0x34, 0x1c, 0x6e, 0x80, 0xf5, 0xff, 0x13, 0x72, 0x85, 0x5d, 0xc1, 0xde, 0x5f,
	0x01, 0x86, 0x55, 0x86, 0x71, 0xe8, 0x10, 0x33, 0x14, 0x70, 0x2a, 0x5f, 0x15, 0x7b, 0x5c, 0x65,
	0x3c, 0x46, 0x3a, 0x17, 0x79, 0xed, 0x54, 0x6a, 0xa6, 0xc9, 0xdf, 0xeb, 0x2a, 0x81, 0x2a, 0x80,
	0x2a, 0x46, 0xa2, 0x06, 0xdb, 0xfd, 0xd5, 0xf3, 0xcf, 0x74, 0xbb, 0x66, 0x56, 0x48, 0xd7, 0x7c,
	0x6a, 0x03, 0x14, 0x1e, 0x55, 0x56, 0xe4, 0xb6, 0xfa, 0x38, 0x2b, 0x5d, 0xfb, 0x87, 0x9f, 0x9e,
	0x78, 0x21, 0x87, 0xc0, 0x0c, 0x63, 0x3e, 0x8d, 0x0f, 0xe2, 0xa7, 0x19, 0x10, 0x9b, 0x15, 0xe1,
	0x11, 0x87, 0x49, 0x33, 0x49, 0xb8, 0x66, 0x32, 0x28, 0x7c, 0x87, 0xf5, 0xd2, 0x2e, 0xc5, 0xf3,
	0x66, 0x2f, 0x79, 0xef, 0x40, 0x5a, 0xd4, 0x14, 0x85, 0x74, 0x5f, 0x06, 0x43, 0x50, 0xcd, 0xde,
	0x84, 0xe7, 0x3c, 0x7d, 0x8e, 0x8a, 0x49, 0xcc, 0x5a, 0xcf, 0x73, 0xa1, 0x8a, 0x13, 0xff, 0x37,
	0x13, 0x3d, 0xad, 0x57, 0xd8, 0x51, 0x22, 0xd6, 0x32, 0x1f, 0xc0, 0x68, 0x4c, 0xa0, 0x5b, 0xdd,
	0x5f, 0x78, 0xc8, 0x9f, 0x2d, 0x3a, 0xa2, 0xb8, 0x1e, 0x4a, 0xe4, 0x08, 0x55, 0x64, 0x05, 0xe6,
	0x94, 0xfb, 0xeb, 0x03, 0x6a, 0x0a, 0xbe, 0x83, 0x18, 0x94, 0xd4, 0xb6, 0xc3, 0xf2, 0x58, 0x9c,
	0x7a, 0x24, 0xdd, 0xd1, 0x3a, 0xb7, 0x3a, 0xb0, 0xbb, 0xe5, 0xd1, 0x28, 0xab, 0xad, 0x24, 0x54,
	0x72, 0x0e, 0x76, 0xd2, 0x89, 0x32, 0xea, 0x46, 0xd3, 0x78, 0xd0, 0xa9, 0x67, 0x78, 0xc1, 0x2d,
	0x18, 0xb0, 0x33, 0xde, 0xdb, 0x27, 0xcc, 0xb0, 0x7c, 0xc9, 0xa4, 0xbd, 0xdf, 0x2b, 0x64, 0x10,
	0x32, 0x44, 0x06, 0x81, 0x21, 0xb3, 0xba, 0xcf, 0x33, 0x85, 0x49, 0x1e, 0x86, 0x4c, 0xbd, 0xf2,
	0x3d, 0x34, 0xef, 0xd6, 0x23, 0x7a, 0x9f, 0x2c, 0xda, 0x84, 0xf0, 0x83, 0x83, 0x71, 0x7d, 0xda,
	0x6e, 0x44, 0x96, 0xcd, 0x1d, 0x05, 0xde, 0x30, 0xf6, 0x1e, 0x2f, 0x9c, 0x99, 0x9c, 0x60, 0x07,
	0x3
};

static inline struct sunxi_dp_hdcp *
sunxi_hdcp1_info_to_sunxi_hdcp(struct sunxi_dptx_hdcp1_info *info)
{
	return container_of(info, struct sunxi_dp_hdcp, hdcp1_info);
}

static u64 dptx_hdcp1_hw_get_an(struct sunxi_edp_hw_desc *edp_hw)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;
	u64 an = 0;

	if (ops == NULL)
		return 0;

	if (ops->hdcp1_get_an)
		an = ops->hdcp1_get_an(edp_hw);
	else
		an = get_random_u64();

	return an;
}

static u64 dptx_hdcp1_hw_get_aksv(struct sunxi_edp_hw_desc *edp_hw)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;

	if (ops == NULL)
		return 0;

	if (ops->hdcp1_get_aksv)
		return ops->hdcp1_get_aksv(edp_hw);
	else
		return 0;
}

static void dptx_hdcp1_hw_write_bksv(struct sunxi_edp_hw_desc *edp_hw, u64 bksv)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;

	if (ops == NULL)
		return;

	if (ops->hdcp1_write_bksv)
		return ops->hdcp1_write_bksv(edp_hw, bksv);
}

static u64 dptx_hdcp1_hw_calculate_km(struct sunxi_edp_hw_desc *edp_hw, u64 an, u64 bksv, bool repeater)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;

	if (ops == NULL)
		return 0;

	if (ops->hdcp1_cal_km)
		return ops->hdcp1_cal_km(edp_hw, an, bksv, repeater);
	else
		return 0; //TODO for software calculate
}

static u32 dptx_hdcp1_hw_calculate_r0(struct sunxi_edp_hw_desc *edp_hw, u64 an, u64 km, bool repeater)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;

	if (ops == NULL)
		return 0;

	if (ops->hdcp1_cal_r0)
		return ops->hdcp1_cal_r0(edp_hw, an, km, repeater);
	else
		return 0;
}

static u64 dptx_hdcp1_hw_get_m0(struct sunxi_edp_hw_desc *edp_hw)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;

	if (ops == NULL)
		return 0;

	if (ops->hdcp1_get_m0)
		return ops->hdcp1_get_m0(edp_hw);
	else
		return 0;
}

static void dptx_hdcp1_hw_encrypt_enable(struct sunxi_edp_hw_desc *edp_hw, bool enable)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;

	if (ops == NULL)
		return;

	if (ops->hdcp1_encrypt_enable)
		ops->hdcp1_encrypt_enable(edp_hw, enable);
}

static void dptx_hdcp_hw_enable(struct sunxi_edp_hw_desc *edp_hw, bool enable)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;

	if (ops == NULL)
		return;

	if (ops->hdcp_enable)
		ops->hdcp_enable(edp_hw, enable);
}

void dptx_hdcp_hw_set_mode(struct sunxi_edp_hw_desc *edp_hw, enum dp_hdcp_mode mode)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;

	if (ops == NULL)
		return;

	if (ops->hdcp_set_mode)
		ops->hdcp_set_mode(edp_hw, mode);
}

__maybe_unused static void dptx_hdcp2_hw_get_kd(struct sunxi_edp_hw_desc *edp_hw, char *rrx, char *rtx,
				 char *km, char *kd)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;

	if (ops == NULL)
		return;

	if (ops->hdcp2_get_kd)
		ops->hdcp2_get_kd(edp_hw, rrx, rtx, km, kd);
	else
		return;
}

__maybe_unused static void dptx_hdcp2_hw_get_edkey_ks(struct sunxi_edp_hw_desc *edp_hw, char *km, char *rn,
				 char *rrx, char *riv, char *ks, char *edkey_ks)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;

	if (ops == NULL)
		return;

	if (ops->hdcp2_get_edkey_ks)
		ops->hdcp2_get_edkey_ks(edp_hw, km, rn, rrx,
					       riv, ks, edkey_ks);
	else
		return;
}

static void dptx_hdcp2_hw_cipher_enable(struct sunxi_edp_hw_desc *edp_hw, char *riv, char *ks)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;

	if (ops == NULL)
		return;

	if (ops->hdcp2_cipher_enable)
		ops->hdcp2_cipher_enable(edp_hw, riv, ks);
	else
		return;
}

static void dptx_hdcp2_hw_cipher_disable(struct sunxi_edp_hw_desc *edp_hw)
{
	struct sunxi_edp_hw_hdcp_ops *ops = edp_hw->hdcp_ops;

	if (ops == NULL)
		return;

	if (ops->hdcp2_cipher_disable)
		ops->hdcp2_cipher_disable(edp_hw);
	else
		return;
}


static bool hdcp1_status_success(struct sunxi_dptx_hdcp1_info *info)
{
	return (info->status == DP_HDCP_STATUS_SUCCESS) ? true : false;
}

static bool hdcp1_status_fail(struct sunxi_dptx_hdcp1_info *info)
{
	return (info->status == DP_HDCP_STATUS_FAIL) ? true : false;
}

/* reserve delay for some unpreditable case in future */
static void sunxi_dp_hdcp1_set_state(struct sunxi_dptx_hdcp1_info *info,
				     enum sunxi_dp_hdcp1_state state, u32 delay_ms)
{
	if (delay_ms)
		msleep(delay_ms);
	info->state = state;
}

static void sunxi_dp_hdcp1_set_status(struct sunxi_dptx_hdcp1_info *info,
				       enum sunxi_dp_hdcp_status status, u32 delay_ms)
{
	if (delay_ms)
		msleep(delay_ms);
	info->status = status;
}

static s32 sunxi_dp_hdcp1_read_parse_bcaps(struct sunxi_dptx_hdcp1_info *info)
{
	char bcap_buf[16];
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp1_info_to_sunxi_hdcp(info);

	memset(bcap_buf, 0, sizeof(bcap_buf));

	if (edp_hw_aux_read(hdcp->edp_hw, HDCP1X_DPCD_BCAPS,
			      HDCP1X_BCAPS_SIZE, &bcap_buf[0]) < 0)
		return RET_FAIL;

	info->bcaps = bcap_buf[0];
	EDP_HDCP_DBG("[HDCP1X] bcaps:0x%x\n", bcap_buf[0]);

	if (info->bcaps & HDCP1X_CAPABLE)
		info->hdcp1_capable = true;

	if (info->bcaps & HDCP1X_REPEATER)
		info->rx_is_repeater = true;

	return RET_OK;
}

static s32 sunxi_dp_hdcp1_write_an_aksv(struct sunxi_dptx_hdcp1_info *info)
{
	char an_buf[16];
	char aksv_buf[16];
	s32 ret = RET_FAIL;
	u64 aksv = 0;
	u32 i = 0;
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp1_info_to_sunxi_hdcp(info);

	memset(an_buf, 0, sizeof(an_buf));
	memset(aksv_buf, 0, sizeof(aksv_buf));

	aksv = dptx_hdcp1_hw_get_aksv(hdcp->edp_hw);

	/* LSB first, DPCD value size is 8bit per reg */
	for (i = 0; i < HDCP1X_AN_SIZE; i++)
		an_buf[i] = (info->an >> (i * 8)) & 0xff;
	ret = edp_hw_aux_write(hdcp->edp_hw, HDCP1X_DPCD_AN,
				 HDCP1X_AN_SIZE, &an_buf[0]);
	if (ret != RET_OK)
		return ret;

	for (i = 0; i < HDCP1X_AKSV_SIZE; i++)
		aksv_buf[i] = (aksv >> (i * 8)) & 0xff;
	ret = edp_hw_aux_write(hdcp->edp_hw, HDCP1X_DPCD_AKSV,
				 HDCP1X_AKSV_SIZE, &aksv_buf[0]);
	if (ret != RET_OK)
		return ret;

	/* just for debug*/
	info->aksv = aksv;
	EDP_HDCP_DBG("[HDCP1X]: aksv:0x%llx", aksv);

	return RET_OK;
}

static s32 sunxi_dp_hdcp1_read_bksv(struct sunxi_dptx_hdcp1_info *info)
{
	char bksv_buf[16];
	u32 i = 0;
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp1_info_to_sunxi_hdcp(info);

	memset(bksv_buf, 0, sizeof(bksv_buf));

	if (edp_hw_aux_read(hdcp->edp_hw, HDCP1X_DPCD_BKSV,
			      HDCP1X_BKSV_SIZE, &bksv_buf[0]) < 0)
		return RET_FAIL;

	for (i = 0; i < HDCP1X_BKSV_SIZE; i++) {
		EDP_HDCP_DBG("[HDCP1X] bksv: data[%d]:0x%x\n", i, bksv_buf[i]);
		info->bksv |= ((u64)(bksv_buf[i]) << (i * 8));
	}

	return RET_OK;
}

static bool sunxi_dp_hdcp1_validate_srm(u64 bksv)
{
	//TODO: maybe some controller IP has its own srm version
	// return edp_hal_hdcp1_validate_srm();
	return true;
}

static bool sunxi_dp_hdcp1_validate_ksv_checksum(u64 bksv)
{
	int i, ones = 0;
	u64 val = 1;

	// test for bits above the KSV
	if (bksv & 0xffffff0000000000U)
		return false;

	// loop through the bits of the ksv
	// test each bit, calculate the number of one's.
	for (i = 0; i < 40; i++) {
		if (bksv & (val << i))
			ones += 1;
	}

	// return status of the ksv
	return (ones == 20) ? true : false;
}

static s32 sunxi_dp_hdcp1_validate_bksv(u64 bksv)
{
	bool pass;

	pass = sunxi_dp_hdcp1_validate_ksv_checksum(bksv);
	if (!pass)
		return RET_FAIL;

	pass = sunxi_dp_hdcp1_validate_srm(bksv);
	if (!pass)
		return RET_FAIL;

	return RET_OK;
}

static s32 sunxi_dp_hdcp1_set_ainfo(struct sunxi_dptx_hdcp1_info *info)
{
	char ainfo_buf[16];
	s32 ret = RET_FAIL;
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp1_info_to_sunxi_hdcp(info);

	memset(ainfo_buf, 0, sizeof(ainfo_buf));

	ainfo_buf[0] |= HDCP1X_REAUTH_ENABLE_IRQ_HPD;
	ret = edp_hw_aux_write(hdcp->edp_hw, HDCP1X_DPCD_KSV_AINFO,
				 HDCP1X_KSV_AINFO_SIZE, &ainfo_buf[0]);
	if (ret != RET_OK)
		return ret;

	return RET_OK;
}

static void sunxi_dp_hdcp1_cal_km(struct sunxi_dptx_hdcp1_info *info)
{
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp1_info_to_sunxi_hdcp(info);

	info->an = dptx_hdcp1_hw_get_an(hdcp->edp_hw);
	EDP_HDCP_DBG("[HDCP1X]: an:0x%llx", info->an);

	dptx_hdcp1_hw_write_bksv(hdcp->edp_hw, info->bksv);

	info->km = dptx_hdcp1_hw_calculate_km(hdcp->edp_hw, info->an, info->bksv, info->rx_is_repeater);
	EDP_HDCP_DBG("[HDCP1X]: km:0x%llx", info->km);
}

static void sunxi_dp_hdcp1_cal_r0(struct sunxi_dptx_hdcp1_info *info)
{
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp1_info_to_sunxi_hdcp(info);

	info->r0 = dptx_hdcp1_hw_calculate_r0(hdcp->edp_hw, info->an, info->km, info->rx_is_repeater);
	EDP_HDCP_DBG("[HDCP1X] r0= 0x%x\n", info->r0);
}

static s32 hdcp1_read_parse_r0_prime(struct sunxi_dptx_hdcp1_info *info)
{
	char bstatus_buf[16];
	char r0_buf[16];
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp1_info_to_sunxi_hdcp(info);

	memset(bstatus_buf, 0, sizeof(bstatus_buf));
	memset(r0_buf, 0, sizeof(r0_buf));

	if (edp_hw_aux_read(hdcp->edp_hw, HDCP1X_DPCD_BSTATUS,
			      HDCP1X_BSTATUS_SIZE, &bstatus_buf[0]) < 0)
		return RET_FAIL;

	if (bstatus_buf[0] & HDCP1X_R0_PRIME_AVAILABLE) {
		info->r0_prime_ready = true;

		if (edp_hw_aux_read(hdcp->edp_hw, HDCP1X_DPCD_R0_PRIME,
				      HDCP1X_R0_PRIME_SIZE, &r0_buf[0]) < 0)
			return RET_FAIL;

		info->r0_prime |= (u32)(r0_buf[0]);
		info->r0_prime |= (u32)(r0_buf[1]) << 8;
	}

	EDP_HDCP_DBG("[HDCP1X] r0'= 0x%x\n", info->r0_prime);

	return RET_OK;
}

static s32 sunxi_dp_hdcp_read_parse_binfo_ksvs_ready(struct sunxi_dptx_hdcp1_info *info)
{
	char bstatus_buf[16];
	char binfo_buf[16];
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp1_info_to_sunxi_hdcp(info);

	memset(bstatus_buf, 0, sizeof(bstatus_buf));
	memset(binfo_buf, 0, sizeof(binfo_buf));

	if (edp_hw_aux_read(hdcp->edp_hw, HDCP1X_DPCD_BSTATUS,
			      HDCP1X_BSTATUS_SIZE, &bstatus_buf[0]) < 0)
		return RET_FAIL;

	if (bstatus_buf[0] & HDCP1X_KSV_LIST_READY) {
		info->ksv_list_ready = true;
		if (edp_hw_aux_read(hdcp->edp_hw, HDCP1X_DPCD_BINFO,
				      HDCP1X_BINFO_SIZE, &binfo_buf[0]) < 0)
			return RET_FAIL;

		info->repeater_dev_cnt = binfo_buf[0] & HDCP1X_REPEATER_DEVICE_CNT_MASK;
		info->device_exceeded = (binfo_buf[0] & HDCP1X_REPEATER_DEVICE_EXCEED) ? true : false;
		info->repeater_dev_dep = binfo_buf[1] & HDCP1X_REPEATER_DEVICE_DEP_MASK;
		info->cascade_exceeded = binfo_buf[1] & HDCP1X_REPEATER_CASCADE_EXCEED;
		info->binfo[0] = binfo_buf[0];
		info->binfo[1] = binfo_buf[1];
		EDP_HDCP_DBG("[HDCP1X] dev_cnt:%d dev_exceed:%d dev_dep:%d cascade_exceed:%d binfo[0]:0x%x binfo[1]:0x%x\n",
			     info->repeater_dev_cnt, info->device_exceeded, info->repeater_dev_dep,
			     info->cascade_exceeded, info->binfo[0], info->binfo[1]);
	}

	return RET_OK;
}

static bool hdcp1_validate_r0_r0_prime(struct sunxi_dptx_hdcp1_info *info)
{
	return (info->r0 == info->r0_prime) ? true : false;
}

static s32 sunxi_dp_hdcp1_read_ksv_list(struct sunxi_dptx_hdcp1_info *info)
{
	char ksv_list_buf[16];
	u32 read_cnt = 0;
	u32 i = 0, j = 0;
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp1_info_to_sunxi_hdcp(info);

	// note:
	// HDCP1_KSV_LIST_BYTES_PER_DEV = 5
	// HDCP1_DEV_CNT_PER_KSV_LIST_FIFO = 3
	// HDCP1X_KSV_FIFO_BYTES = 15

	/* each dpcd read ksv list fifo contains 3 device's info */
	read_cnt = info->repeater_dev_cnt / HDCP1_DEV_CNT_PER_KSV_LIST_FIFO;
	read_cnt += (info->repeater_dev_cnt % HDCP1_DEV_CNT_PER_KSV_LIST_FIFO) ? 1 : 0;

	info->ksv_cnt = 0;
	memset(info->ksv_list, 0, HDCP1_MAX_REPEATER_DEV_CNT * HDCP1_KSV_LIST_BYTES_PER_DEV);
	for (i = 0; i < read_cnt; i++) {
		memset(ksv_list_buf, 0, sizeof(ksv_list_buf));
		if (edp_hw_aux_read(hdcp->edp_hw, HDCP1X_DPCD_KSV_FIFO,
			HDCP1X_KSV_FIFO_SIZE, &ksv_list_buf[0]) < 0) {
			return RET_FAIL;
		}

		for (j = 0; j < HDCP1X_KSV_FIFO_SIZE; j++)
			EDP_HDCP_DBG("[HDCP1X] ksv_list= 0x%x\n", ksv_list_buf[j]);

		for (j = 0; j < HDCP1_DEV_CNT_PER_KSV_LIST_FIFO; j++) {
			memcpy(info->ksv_list + (info->ksv_cnt * HDCP1_KSV_LIST_BYTES_PER_DEV),
			       ksv_list_buf + (j * HDCP1_KSV_LIST_BYTES_PER_DEV),
			       HDCP1_KSV_LIST_BYTES_PER_DEV);

			info->ksv_cnt++;

			/* break if device cnt reach */
			if (info->ksv_cnt == info->repeater_dev_cnt)
				break;
		}
	}

	return RET_OK;
}

static void sunxi_dp_hdcp1_get_m0(struct sunxi_dptx_hdcp1_info *info)
{
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp1_info_to_sunxi_hdcp(info);

	u64 m0 = dptx_hdcp1_hw_get_m0(hdcp->edp_hw);
	u32 i = 0;

	for (i = 0; i < 8; i++) {
		info->m0[i] = (u8)((m0 >> (8 * i)) & 0xff);
		EDP_HDCP_DBG("[HDCP1X] m0= 0x%x\n", info->m0[i]);
	}
}

static void data_endian_switch(char *data, u32 len)
{
	char tmp[256];
	u32 i;

	if (len > 256)
		EDP_ERR("%s len(%d) out of 256\n", __func__, len);

	for (i = 0; i < len; i++) {
		tmp[len - i - 1] = data[i];
	}

	memcpy(data, tmp, len);
}

static s32 sunxi_dp_hdcp1_read_v_prime(struct sunxi_dptx_hdcp1_info *info)
{
	char vprime_buf[HDCP1X_V_PRIME_SIZE];
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp1_info_to_sunxi_hdcp(info);
	u32 cal_cnt, i, size;

	memset(vprime_buf, 0, sizeof(vprime_buf));

	if (edp_hw_aux_read(hdcp->edp_hw, HDCP1X_DPCD_V_PRIME,
			      HDCP1X_V_PRIME_SIZE, &vprime_buf[0]) < 0)
		return RET_FAIL;

	for (i = 0; i < HDCP1X_V_PRIME_SIZE; i++)
		EDP_HDCP_DBG("[HDCP1X] vprime_buf[%d]= 0x%x\n", i, vprime_buf[i]);


	// raw data is LSB format, transfer to store and compare with local v
	size = sizeof(u32); // size = 4
	cal_cnt = HDCP1X_V_PRIME_SIZE / size; // cal_cnt = 5
	for (i = 0; i < cal_cnt; i++)
		data_endian_switch(&vprime_buf[i * size], size);

	for (i = 0; i < HDCP1X_V_PRIME_SIZE; i++)
		EDP_HDCP_DBG("[HDCP1X] vprime[%d]= 0x%x\n", i, vprime_buf[i]);

	memcpy(info->v_prime, vprime_buf, HDCP1X_V_PRIME_SIZE);

	return RET_OK;
}

static void sunxi_dp_hdcp1_calculate_v(struct sunxi_dptx_hdcp1_info *info)
{
	struct shash_desc *shash = kzalloc(sizeof (struct shash_desc), GFP_KERNEL);
	u8 sha1_output[HDCP1X_V_PRIME_SIZE];
	u32 i;

	if ((info->repeater_dev_cnt == 0) || (info->ksv_cnt == 0)) {
		EDP_ERR("%s return because of repeater_dev_cnt(%d) ksv_cnt(%d)\n",
			__func__, info->repeater_dev_cnt, info->ksv_cnt);
		return;
	}

	// note: HDCP1_KSV_LIST_BYTES_PER_DEV = 5
	sha1_base_init(shash);
	crypto_sha1_update(shash, info->ksv_list, info->ksv_cnt * HDCP1_KSV_LIST_BYTES_PER_DEV);
	crypto_sha1_finup(shash, info->binfo, (HDCP1X_BINFO_SIZE + HDCP1X_M0_SIZE), sha1_output);

	memcpy(info->v, sha1_output, HDCP1X_V_PRIME_SIZE);

	for (i = 0; i < HDCP1X_V_PRIME_SIZE; i++)
		EDP_HDCP_DBG("[HDCP1X] v= 0x%x\n", info->v[i]);
}

static bool sunxi_dp_hdcp1_v_validate(struct sunxi_dptx_hdcp1_info *info)
{
	u8 zero_char[HDCP1X_V_PRIME_SIZE] = {0};

	if (info->repeater_dev_cnt == 0) {
		EDP_HDCP_DBG("[HDCP1X] verify v and v' fail because of dev_cnt is 0\n");
		return false;
	}

	if (info->ksv_cnt == 0) {
		EDP_HDCP_DBG("[HDCP1X] verify v and v' fail because of ksv_list__cnt is 0\n");
		return false;
	}

	if (memcmp(info->v, zero_char, HDCP1X_V_PRIME_SIZE) == 0) {
		EDP_HDCP_DBG("[HDCP1X] verify v and v' fail because v is all 0\n");
		return false;
	}

	if (memcmp(info->v_prime, zero_char, HDCP1X_V_PRIME_SIZE) == 0) {
		EDP_HDCP_DBG("[HDCP1X] verify v and v' fail because v' is all 0\n");
		return false;
	}

	if (memcmp(info->v, info->v_prime, HDCP1X_V_PRIME_SIZE) == 0)
		return true;
	else
		return false;
}


static void sunxi_dp_hdcp_irq_work(struct work_struct *work)
{
	struct sunxi_dp_hdcp *hdcp = container_of(work, struct sunxi_dp_hdcp, cpirq_work);
	struct sunxi_dptx_hdcp1_info *hdcp1_info = &hdcp->hdcp1_info;
	struct sunxi_dptx_hdcp2_info *hdcp2_info = &hdcp->hdcp2_info;

	if (hdcp->hdcp1_capable) {
		hdcp1_read_parse_r0_prime(hdcp1_info);
		sunxi_dp_hdcp_read_parse_binfo_ksvs_ready(hdcp1_info);

		if (hdcp1_info->r0_prime_ready || hdcp1_info->ksv_list_ready)
			wake_up(&hdcp->auth_queue);

//TODO
//		if (ksv_list_available)
//			wake_up(&hdcp->auth_queue);

//TODO
//		if (link_integrity)
//			stop_and_re_auth();;
	}

	if (hdcp->hdcp2_capable) {
		hdcp2_read_parse_rxstatus(hdcp2_info);
	}
}

void sunxi_dp_hdcp_queue_cpirq_work(struct sunxi_dp_hdcp *hdcp)
{
	queue_work(hdcp->cpirq_workqueue, &hdcp->cpirq_work);
}

static void sunxi_dp_hdcp1_encrypt_enable(struct sunxi_dptx_hdcp1_info *info, bool enable)
{
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp1_info_to_sunxi_hdcp(info);

	dptx_hdcp1_hw_encrypt_enable(hdcp->edp_hw, enable);
}

s32 sunxi_dp_hdcp1_auth(struct sunxi_dp_hdcp *hdcp)
{
	struct sunxi_dptx_hdcp1_info *info = &hdcp->hdcp1_info;
	s32 ret = RET_OK;
	long timeout = 0;


	while (!hdcp1_status_success(info) && !hdcp1_status_fail(info)) {
		switch (info->state) {
		case HDCP1_DP_STATE_NONE:
			EDP_ERR("hdcp1 for dp not enable yet!\n");
			return RET_OK;
		case HDCP1_DP_STATE_START:
			sunxi_dp_hdcp1_set_state(info, HDCP1_A0_DETERMINE_RX_HDCP_CAPABLE, 0);
			break;
		case HDCP1_A0_DETERMINE_RX_HDCP_CAPABLE:
			ret = sunxi_dp_hdcp1_read_parse_bcaps(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp1_set_state(info, HDCP1_A0_READ_BCAPS_FAIL, 0);

			if (info->hdcp1_capable)
				sunxi_dp_hdcp1_set_state(info, HDCP1_A1_EXCHANGE_KSVS_READ_BKSV, 0);
			else
				sunxi_dp_hdcp1_set_state(info, HDCP1_A0_HDCP_NOT_CAPABLE, 0);
			break;
		case HDCP1_A1_EXCHANGE_KSVS_READ_BKSV:
			ret = sunxi_dp_hdcp1_read_bksv(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp1_set_state(info, HDCP1_A1_READ_BKSV_FAIL, 0);
			else
				sunxi_dp_hdcp1_set_state(info, HDCP1_A1_EXCHANGE_KSVS_VALIDATE_BKSV, 0);
			break;
		case HDCP1_A1_EXCHANGE_KSVS_VALIDATE_BKSV:
			ret = sunxi_dp_hdcp1_validate_bksv(info->bksv);
			if (ret != RET_OK)
				sunxi_dp_hdcp1_set_state(info, HDCP1_A1_VALIDATE_BKSV_FAIL, 0);
			else {
				if (info->rx_is_repeater) {
					if (sunxi_dp_hdcp1_set_ainfo(info) != RET_OK)
						sunxi_dp_hdcp1_set_state(info, HDCP1_A1_WRITE_AINFO_FAIL, 0);
				}
				sunxi_dp_hdcp1_set_state(info, HDCP1_A1_EXCHANGE_KSVS_CAL_KM, 0);
			}
			break;
		case HDCP1_A1_EXCHANGE_KSVS_CAL_KM:
			sunxi_dp_hdcp1_cal_km(info);
			sunxi_dp_hdcp1_set_state(info, HDCP1_A2_COMPUTATIONS, 0);
			break;
		case HDCP1_A2_COMPUTATIONS:
			sunxi_dp_hdcp1_cal_r0(info);
			sunxi_dp_hdcp1_set_state(info, HDCP1_A2_WRITE_AN_AKSV, 0);
			break;
		case HDCP1_A2_WRITE_AN_AKSV:
			ret = sunxi_dp_hdcp1_write_an_aksv(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp1_set_state(info, HDCP1_A2_WRITE_AN_AKSV_FAIL, 0);
			else
				sunxi_dp_hdcp1_set_state(info, HDCP1_A2_A3_WAIT_FOR_R0_PRIME, 0);
			break;
		case HDCP1_A2_A3_WAIT_FOR_R0_PRIME:
			/* refer from HDCP1X spec, we should not read r0' within 100ms*/
			timeout = wait_event_timeout(hdcp->auth_queue, info->r0_prime_ready,
						     msecs_to_jiffies(300));
			sunxi_dp_hdcp1_set_state(info, HDCP1_A2_A3_READ_R0_PRIME, 0);
			break;
		case HDCP1_A2_A3_READ_R0_PRIME:
			/*
			 * usually r0' has been read after CP_IRQ income
			 * but some system not support CP_IRQ, so read r0'
			 * manaualy after 100ms timeout
			 */
			if (!info->r0_prime_ready || !info->r0_prime) {
				EDP_HDCP_DBG("[HDCP1X]: wait r0_prime cpirq timeout, read it manaualy\n");
				if (hdcp1_read_parse_r0_prime(info) != RET_OK)
					sunxi_dp_hdcp1_set_state(info, HDCP1_A2_A3_READ_R0_PRIME_FAIL, 0);
			}

			if (!info->r0_prime_ready || !info->r0_prime)
				sunxi_dp_hdcp1_set_state(info, HDCP1_A2_A3_READ_R0_PRIME_FAIL, 0);
			else
				sunxi_dp_hdcp1_set_state(info, HDCP1_A3_VALIDATE_RX, 0);
			break;
		case HDCP1_A3_VALIDATE_RX:
			if (!hdcp1_validate_r0_r0_prime(info)) {
				if (info->r0_prime_retry >= 3)
					sunxi_dp_hdcp1_set_state(info, HDCP1_A3_VALIDATE_RX_FAIL, 0);
				else {
					/* clear r0' and try to re-read it 2 more times*/
					info->r0_prime = 0;
					sunxi_dp_hdcp1_set_state(info, HDCP1_A2_A3_READ_R0_PRIME, 0);
					info->r0_prime_retry++;
				}
			} else {
				sunxi_dp_hdcp1_set_state(info, HDCP1_A5_TEST_FOR_REPEATER, 0);
			}
			break;
		case HDCP1_A5_TEST_FOR_REPEATER:
			if (info->rx_is_repeater)
				sunxi_dp_hdcp1_set_state(info, HDCP1_A6_WAIT_FOR_READY, 0);
			else
				sunxi_dp_hdcp1_set_state(info, HDCP1_A4_AUTHENTICATED, 0);
			break;
		case HDCP1_A6_WAIT_FOR_READY:
			timeout = wait_event_timeout(hdcp->auth_queue, info->ksv_list_ready,
						     msecs_to_jiffies(5000));
			/*
			 * usually bstatus READY BIT has been read after CP_IRQ income but some
			 * system not support CP_IRQ, so read bstatus manaualy after 5s timeout
			 */
			if (!info->ksv_list_ready) {
				if (sunxi_dp_hdcp_read_parse_binfo_ksvs_ready(info) != RET_OK)
					sunxi_dp_hdcp1_set_state(info, HDCP1_A6_READ_BSTATUS_BINFO_FAIL, 0);
			}

			if (!info->ksv_list_ready || info->device_exceeded || info->cascade_exceeded)
				sunxi_dp_hdcp1_set_state(info, HDCP1_A6_WAIT_FOR_READY_FAIL, 0);
			else if (info->ksv_list_ready && (info->device_exceeded || info->cascade_exceeded))
				sunxi_dp_hdcp1_set_state(info, HDCP1_A6_REPEATER_DEV_DEP_EXCEED_FAIL, 0);
			else
				sunxi_dp_hdcp1_set_state(info, HDCP1_A7_READ_KSV_LIST, 0);
			break;
		case HDCP1_A7_READ_KSV_LIST:
			ret = sunxi_dp_hdcp1_read_ksv_list(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp1_set_state(info, HDCP1_A7_READ_KSV_LIST_FAIL, 0);
			else
				sunxi_dp_hdcp1_set_state(info, HDCP1_A7_CALCULATE_V, 0);
			break;
		case HDCP1_A7_CALCULATE_V:
			sunxi_dp_hdcp1_get_m0(info);
			ret = sunxi_dp_hdcp1_read_v_prime(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp1_set_state(info, HDCP1_A7_READ_V_PRIME_FAIL, 0);
			else {
				sunxi_dp_hdcp1_calculate_v(info);
				sunxi_dp_hdcp1_set_state(info, HDCP1_A7_VERIFY_V_V_PRIME, 0);
			}
			break;
		case HDCP1_A7_VERIFY_V_V_PRIME:
			if (!sunxi_dp_hdcp1_v_validate(info)) {
				if (info->v_read_retry >= 3)
					sunxi_dp_hdcp1_set_state(info, HDCP1_A7_VERIFY_V_V_PRIME_FAIL, 0);
				else {
					sunxi_dp_hdcp1_set_state(info, HDCP1_A7_READ_KSV_LIST, 0);
					info->v_read_retry++;
				}
			} else
				sunxi_dp_hdcp1_set_state(info, HDCP1_A4_AUTHENTICATED, 0);
			break;
		case HDCP1_A4_AUTHENTICATED:
			sunxi_dp_hdcp1_encrypt_enable(info, true);
			EDP_INFO("[HDCP1X]: hdcp1 authencitate success, start video transmit!\n");
			sunxi_dp_hdcp1_set_state(info, HDCP1_DP_STATE_END, 0);
			break;
		case HDCP1_DP_STATE_END:
			sunxi_dp_hdcp1_set_status(info, DP_HDCP_STATUS_SUCCESS, 0);
			//sunxi_dp_hdcp1_clear_r0_prime_ready(info);
			//sunxi_dp_hdcp1_clear_ksv_list_ready(info);
			ret = RET_OK;
			break;
		case HDCP1_A0_READ_BCAPS_FAIL:
		case HDCP1_A0_HDCP_NOT_CAPABLE:
		case HDCP1_A1_READ_BKSV_FAIL:
		case HDCP1_A1_VALIDATE_BKSV_FAIL:
		case HDCP1_A1_WRITE_AINFO_FAIL:
		case HDCP1_A2_WRITE_AN_AKSV_FAIL:
		case HDCP1_A2_A3_READ_R0_PRIME_FAIL:
		case HDCP1_A3_VALIDATE_RX_FAIL:
		case HDCP1_A6_READ_BSTATUS_BINFO_FAIL:
		case HDCP1_A6_WAIT_FOR_READY_FAIL:
		case HDCP1_A6_REPEATER_DEV_DEP_EXCEED_FAIL:
		case HDCP1_A7_READ_KSV_LIST_FAIL:
		case HDCP1_A7_READ_V_PRIME_FAIL:
		case HDCP1_A7_VERIFY_V_V_PRIME_FAIL:
		default:
			EDP_ERR("dp hdcp1 authentication fail, reason:%s\n", hdcp1_fail_reason[info->state]);
			sunxi_dp_hdcp1_set_status(info, DP_HDCP_STATUS_FAIL, 0);
			//sunxi_dp_hdcp1_clear_r0_prime_ready(info);
			//sunxi_dp_hdcp1_clear_ksv_list_ready(info);
			ret = RET_FAIL;
			break;
		}
	}

	return ret;
}

void sunxi_dp_hdcp1_clear_info(struct sunxi_dptx_hdcp1_info *info)
{
	memset(info, 0, sizeof(struct sunxi_dptx_hdcp1_info));
}

s32 sunxi_dp_hdcp1_disable(struct sunxi_dp_hdcp *hdcp)
{
	struct sunxi_dptx_hdcp1_info *info = &hdcp->hdcp1_info;

	sunxi_dp_hdcp1_encrypt_enable(info, false);
	sunxi_dp_hdcp1_clear_info(info);
	dptx_hdcp_hw_enable(hdcp->edp_hw, false);
	dptx_hdcp_hw_set_mode(hdcp->edp_hw, HDCP_NONE_MODE);

	return RET_OK;
}

s32 sunxi_dp_hdcp1_enable(struct sunxi_dp_hdcp *hdcp)
{
	struct sunxi_dptx_hdcp1_info *info = &hdcp->hdcp1_info;
	u32 i = 0;
	s32 ret = 0;

	// FIXME: not sure if need: clear all exit info,
	// begin authencation at the original procedure

	dptx_hdcp_hw_set_mode(hdcp->edp_hw, HDCP14_MODE);
	dptx_hdcp_hw_enable(hdcp->edp_hw, true);
	sunxi_dp_hdcp1_encrypt_enable(info, false);

	for (i = 0; i < HDCP1X_REAUTH_CNT; i++) {
		sunxi_dp_hdcp1_clear_info(info);
		sunxi_dp_hdcp1_set_state(info, HDCP1_DP_STATE_START, 0);
		ret = sunxi_dp_hdcp1_auth(hdcp);
		if (ret == RET_OK) {
			return ret;
		}
	}

	EDP_ERR("hdcp1 retry 3 times but still fail, authentication fail!\n");
	sunxi_dp_hdcp1_disable(hdcp);

	return RET_FAIL;
}

bool dprx_hdcp1_capable(struct sunxi_dp_hdcp *hdcp)
{
	char bcap_buf[16];

	memset(bcap_buf, 0, sizeof(bcap_buf));

	if (edp_hw_aux_read(hdcp->edp_hw, HDCP1X_DPCD_BCAPS, HDCP1X_BCAPS_SIZE, &bcap_buf[0]) < 0)
		return false;

	if (bcap_buf[0] & HDCP1X_CAPABLE)
		return true;
	else
		return false;

	return false;
}

bool dprx_hdcp2_capable(struct sunxi_dp_hdcp *hdcp)
{
	char rxcaps_buf[HDCP2X_RXCAPS_SIZE];
	u32 version = 0;
	u32 rxcaps = 0;
	u32 i = 0;

	memset(rxcaps_buf, 0, sizeof(rxcaps_buf));
	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_RXCAPS,
			      HDCP2X_RXCAPS_SIZE, &rxcaps_buf[0]) < 0)
		return RET_FAIL;

	for (i = 0; i < HDCP2X_RXCAPS_SIZE; i++) {
		/* data are store in big-endian */
		rxcaps |= rxcaps_buf[i] << ((HDCP2X_RXCAPS_SIZE - i - 1) * 8);
	}

	version = (rxcaps & RXCAPS_VERSION_MASK) >> RXCAPS_VERSION_OFFSET;

	if ((rxcaps & RXCAPS_CAPABLE_BIT) && (version == 0x2))
		return true;
	else
		return false;

	return false;
}

s32 sunxi_dp_hdcp_init(struct sunxi_dp_hdcp *hdcp,
				struct sunxi_edp_hw_desc *edp_hw)
{
	/* deliver edp_hw to hdcp, use for some lowlevel operation */
	hdcp->edp_hw = edp_hw;
	mutex_init(&hdcp->auth_lock);
	init_waitqueue_head(&hdcp->auth_queue);
	hdcp->cpirq_workqueue = alloc_workqueue("cp_irq", WQ_HIGHPRI, 0);
	INIT_WORK(&hdcp->cpirq_work, sunxi_dp_hdcp_irq_work);
	INIT_LIST_HEAD(&hdcp->stored_pairing_head);

	return RET_OK;
}

static inline struct sunxi_dp_hdcp *
sunxi_hdcp2_info_to_sunxi_hdcp(struct sunxi_dptx_hdcp2_info *info)
{
	return container_of(info, struct sunxi_dp_hdcp, hdcp2_info);
}

/* reserve delay for some unpreditable case in future */
static void sunxi_dp_hdcp2_set_state(struct sunxi_dptx_hdcp2_info *info,
				     enum sunxi_dp_hdcp2_state state, u32 delay_ms)
{
	if (delay_ms)
		msleep(delay_ms);
	info->state = state;
}

static void sunxi_dp_hdcp2_set_status(struct sunxi_dptx_hdcp2_info *info,
				       enum sunxi_dp_hdcp_status status, u32 delay_ms)
{
	if (delay_ms)
		msleep(delay_ms);
	info->status = status;
}

static bool hdcp2_status_success(struct sunxi_dptx_hdcp2_info *info)
{
	if (info->status == DP_HDCP_STATUS_SUCCESS)
		return true;
	else
		return false;
}

static bool hdcp2_status_fail(struct sunxi_dptx_hdcp2_info *info)
{
	if (info->status == DP_HDCP_STATUS_FAIL)
		return true;
	else
		return false;
}

void sunxi_dp_hdcp2_clear_info(struct sunxi_dptx_hdcp2_info *info)
{
	memset(info, 0, sizeof(struct sunxi_dptx_hdcp2_info));
}

static s32 hdcp2_read_parse_rxstatus(struct sunxi_dptx_hdcp2_info *info)
{
	char rxstatus_buf[HDCP2X_RXSTATUS_SIZE];
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);

	memset(rxstatus_buf, 0, sizeof(rxstatus_buf));

	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_RXSTATUS,
			      HDCP2X_RXSTATUS_SIZE, &rxstatus_buf[0]) < 0)
		return RET_FAIL;

	if (rxstatus_buf[0] & RXSTATUS_H_PRIME_AVA_BIT)
		info->h_prime_available = true;

	if (rxstatus_buf[0] & RXSTATUS_V_PRIME_READY_BIT)
		info->v_prime_ready = true;

	if (rxstatus_buf[0] & RXSTATUS_PAIRING_AVA_BIT)
		info->pairing_available = true;

	if (rxstatus_buf[0] & RXSTATUS_REQUTH_REQ_BIT)
		info->rx_need_reauth_req = true;

	if (rxstatus_buf[0] & RXSTATUS_LINK_INTEG_FAIL_BIT)
		info->link_integrity_failure = true;

	EDP_HDCP_DBG("[HDCP1X] <%s> rxstatus = 0x%x\n", __func__, rxstatus_buf[0]);

	return RET_OK;
}

static s32 sunxi_dp_hdcp2_read_parse_rxcaps(struct sunxi_dptx_hdcp2_info *info)
{
	char rxcaps_buf[HDCP2X_RXCAPS_SIZE];
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);
	int i = 0;
	u32 rxcaps = 0;

	memset(rxcaps_buf, 0, sizeof(rxcaps_buf));
	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_RXCAPS,
			      HDCP2X_RXCAPS_SIZE, &rxcaps_buf[0]) < 0)
		return RET_FAIL;

	/* fill rxcaps */
	memcpy(info->rxcaps, rxcaps_buf, sizeof(rxcaps_buf));

	for (i = 0; i < HDCP2X_RXCAPS_SIZE; i++) {
		/* data are store in big-endian */
		rxcaps |= rxcaps_buf[i] << ((HDCP2X_RXCAPS_SIZE - i - 1) * 8);
	}

	if (rxcaps & RXCAPS_CAPABLE_BIT)
		hdcp->hdcp2_capable = true;

	info->hdcp_version = (rxcaps & RXCAPS_VERSION_MASK) >> RXCAPS_VERSION_OFFSET;

	return RET_OK;
}

static s32 sunxi_dp_hdcp2_send_ake_init(struct sunxi_dptx_hdcp2_info *info)
{
	char ake_init_buf[HDCP2X_RTX_SIZE + HDCP2X_TXCAPS_SIZE];
	s32 ret = RET_FAIL;
	u32 i = 0;
	char rtx[HDCP2X_RTX_SIZE];
	char txcaps[] = {0x02, 0x00, 0x00};
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);

	memset(ake_init_buf, 0, sizeof(ake_init_buf));

	/* fill rtx */
	for (i = 0; i < HDCP2X_RTX_SIZE; i++)
		rtx[i] = get_random_u8();

	/* fill txcaps */
	memcpy(info->txcaps, txcaps, HDCP2X_TXCAPS_SIZE);

	/* big-endian, DPCD value size is 8bit per reg */
	for (i = 0; i < HDCP2X_RTX_SIZE; i++)
		ake_init_buf[i] = rtx[i];

	/* fill rtx */
	memcpy(info->rtx, ake_init_buf, HDCP2X_RTX_SIZE);

	for (i = 0; i < HDCP2X_TXCAPS_SIZE; i++) {
		ake_init_buf[HDCP2X_RTX_SIZE + i] = info->txcaps[i];
	}

	for (i = 0; i < sizeof(ake_init_buf); i++)
		EDP_HDCP_DBG("[HDCP2X] ake_init_buf[%d]:0x%x\n", i, ake_init_buf[i]);

	/* send ake init message */
	ret = edp_hw_aux_write(hdcp->edp_hw, HDCP2X_DPCD_RTX,
				 sizeof(ake_init_buf), &ake_init_buf[0]);
	return ret;
}

static s32 sunxi_dp_hdcp2_receive_ake_send_cert(struct sunxi_dptx_hdcp2_info *info)
{
	char certrx_buf[HDCP2X_CERTRX_SIZE];
	char rrx_buf[HDCP2X_RRX_SIZE];
	char rxcaps_buf[HDCP2X_RXCAPS_SIZE];
	u32 rxcaps = 0;
	u32 i = 0;
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);

	/* read certrx */
	memset(certrx_buf, 0, HDCP2X_CERTRX_SIZE);
	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_CERTRX,
			      HDCP2X_CERTRX_SIZE, &certrx_buf[0]) < 0)
		return RET_FAIL;
	memcpy(&info->certrx, certrx_buf, sizeof(certrx_buf));

	/* read rrx */
	memset(rrx_buf, 0, sizeof(rrx_buf));
	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_RRX,
			      HDCP2X_RRX_SIZE, &rrx_buf[0]) < 0)
		return RET_FAIL;
	memcpy(info->rrx, rrx_buf, sizeof(rrx_buf));

	/* read rxcaps */
	memset(rxcaps_buf, 0, sizeof(rxcaps_buf));
	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_RXCAPS,
			      HDCP2X_RXCAPS_SIZE, &rxcaps_buf[0]) < 0)
		return RET_FAIL;
	/* update rxcaps */
	memcpy(info->rxcaps, rxcaps_buf, sizeof(rxcaps_buf));

	for (i = 0; i < HDCP2X_RXCAPS_SIZE; i++) {
		/* data are store in big-endian */
		rxcaps |= rxcaps_buf[i] << ((HDCP2X_RXCAPS_SIZE - i - 1) * 8);
	}

	/* parse rxcaps */
	if (rxcaps & RXCAPS_REPEATER_BIT)
		info->rx_is_repeater = true;

	return RET_OK;
}

static bool sunxi_dp_hdcp2_verify_rcvid_checksum(char *rcvid)
{
	int i, j, ones = 0;

	for (i = 0; i < CERTRX_RCVID_SIZE; i++) {
		EDP_HDCP_DBG("[HDCP2X]: rcvid[%d]: 0x%x\n", i, rcvid[i]);
	}
	// loop through the bits of the rcvid
	// test each bit, calculate the number of one's.
	for (i = 0; i < CERTRX_RCVID_SIZE; i++) {
		for (j = 0; j < 8; j++) {
			if (rcvid[i] & (1 << j))
				ones += 1;
		}
	}

	// return status of the ksv
	return (ones == 20) ? true : false;
}

// TODO: complete it if need update revocatipn list in a way later
static bool sunxi_dp_hdcp2_verify_srm(struct sunxi_dptx_hdcp2_info *info)
{
	return true;
/*
	struct sunxi_dptx_hdcp2_revoclist *revoc_list = NULL;
	u32 result, i;

	revoc_list = &info->revoc_list;

	for (i = 0; i < revoc_list->num_of_devices; i++) {
		result = memcmp(info->certrx.rcvid, revoc_list->rcvid[i], CERTRX_RCVID_SIZE);
		if (!result)
			return false;
	}

	return true;
*/
}

static s32 sunxi_dp_hdcp2_verify_cert(struct sunxi_dptx_hdcp2_info *info)
{
	struct sunxi_dptx_hdcp2_certrx *certrx = &info->certrx;
	const u8 *kpubdpc = NULL;
	bool pass;


	pass = sunxi_dp_hdcp2_verify_rcvid_checksum(certrx->rcvid);
	if (!pass) {
		EDP_ERR("hdcp2 verify cert_rx rcvid fail\n");
		return RET_FAIL;
	}

	/* obtain public key for production use */
	kpubdpc = hdcp2x_tx_kpubdpc;

#ifdef HDCP2X_USE_FACSIMILE
	/*override public key if need, for facsimile test key */
	kpubdpc = hdcp2x_tx_test_kpubdpc;
#endif

	pass = sunxi_hdcp2x_tx_verify_certificate(&info->certrx, kpubdpc,
					       HDCP2X_TX_KPUB_DCP_LLC_N_SIZE,
					       &kpubdpc[HDCP2X_TX_KPUB_DCP_LLC_N_SIZE],
					       HDCP2X_TX_KPUB_DCP_LLC_E_SIZE);
	if (pass != 0) {
		EDP_ERR("hdcp2 verify cert_rx signature fail\n");
		return RET_FAIL;
	}

	pass = sunxi_dp_hdcp2_verify_srm(info);
	if (!pass) {
		EDP_ERR("hdcp2 verify srm fail\n");
		return RET_FAIL;
	}

	return RET_OK;
}


static bool sunxi_dp_hdcp2_get_pairing_info(struct sunxi_dptx_hdcp2_info *info)
{
	struct sunxi_dptx_hdcp2_pairing_info *pairing_info_ptr;
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);
	u8 illegal_rcvd[] = {0x0, 0x0, 0x0, 0x0, 0x0};
	bool info_exist = false;

	if (!memcmp(info->certrx.rcvid, illegal_rcvd, CERTRX_RCVID_SIZE))
		return NULL;

	list_for_each_entry(pairing_info_ptr, &hdcp->stored_pairing_head, list) {
		if (!memcmp(info->certrx.rcvid, pairing_info_ptr->rcvid, CERTRX_RCVID_SIZE) && pairing_info_ptr->ready) {
			info->pairing_info = pairing_info_ptr;
			info_exist = true;
			break;
		}
	}

	return info_exist;
}

static s32 sunxi_dp_hdcp2_ake_stored_km(struct sunxi_dptx_hdcp2_info *info)
{
	char ekh_km_buf[HDCP2X_EKH_KM_WR_SIZE];
	char m_buf[HDCP2X_M_SIZE];
	s32 ret = RET_FAIL;
	u32 i = 0;
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);

	/* fill ekh_km, big-endian, DPCD value size is 8bit per reg */
	memset(ekh_km_buf, 0, sizeof(ekh_km_buf));
	memcpy(ekh_km_buf, info->pairing_info->ekh_km, HDCP2X_EKH_KM_WR_SIZE);
	for (i = 0; i < HDCP2X_EKH_KM_WR_SIZE; i++) {
		EDP_HDCP_DBG("[HDCP2X] ekh_km_buf[%d]:0x%x\n", i, ekh_km_buf[i]);
	}

	/* send ekh_km message */
	ret = edp_hw_aux_write(hdcp->edp_hw, HDCP2X_DPCD_EKH_KM_WR,
				 sizeof(ekh_km_buf), &ekh_km_buf[0]);
	if (ret != RET_OK) {
		EDP_ERR("[HDCP2X] send ekh_km fail\n");
		return ret;
	}

	/* fill m, big-endian, DPCD value size is 8bit per reg */
	memset(m_buf, 0, sizeof(m_buf));
	memcpy(m_buf, info->pairing_info->m, HDCP2X_M_SIZE);
	for (i = 0; i < HDCP2X_M_SIZE; i++) {
		EDP_HDCP_DBG("[HDCP2X] m_buf[%d]:0x%x\n", i, m_buf[i]);
	}

	/* send ekh_km message */
	ret = edp_hw_aux_write(hdcp->edp_hw, HDCP2X_DPCD_M,
				 sizeof(m_buf), &m_buf[0]);
	if (ret != RET_OK) {
		EDP_ERR("[HDCP2X] send m fail\n");
		return ret;
	}

	return ret;
}

static void sunxi_dp_hdcp2_encrypt_km(struct sunxi_dptx_hdcp2_info *info, char *ekpub_km)
{
	u8 masking_seed[HDCP2X_TX_KM_MSK_SEED_SIZE];
	u32 i;

	for (i = 0; i < HDCP2X_TX_KM_MSK_SEED_SIZE; i++)
		masking_seed[i] = get_random_u8();

	sunxi_hdcp2x_tx_encryptedkm(&info->certrx,
				   info->km, masking_seed, ekpub_km);

	/* fill pairing_info: ekpub_km */
	memcpy(info->ekpub_km, ekpub_km, HDCP2X_EKPUB_KM_SIZE);
}

static s32 sunxi_dp_hdcp2_ake_no_stored_km(struct sunxi_dptx_hdcp2_info *info)
{
	char ekpub_km_buf[HDCP2X_EKPUB_KM_SIZE];
	s32 ret = RET_FAIL;
	u32 i = 0;
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);

	memset(ekpub_km_buf, 0, sizeof(ekpub_km_buf));

	/* create new pairing info */
	info->pairing_info = kzalloc(sizeof(struct sunxi_dptx_hdcp2_pairing_info), GFP_KERNEL);

	/* fill km */
	for (i = 0; i < HDCP2X_TX_KM_SIZE; i++)
		info->km[i] = get_random_u8();

	/* fill new pairing info */
	memcpy(info->pairing_info->rcvid, info->certrx.rcvid, sizeof(info->pairing_info->rcvid));
	memcpy(info->pairing_info->rxcaps, info->rxcaps, sizeof(info->rxcaps));
	memcpy(info->pairing_info->rtx, info->rtx, sizeof(info->rtx));
	memcpy(info->pairing_info->rrx, info->rrx, sizeof(info->rrx));
	memcpy(&info->pairing_info->m[0], info->rtx, sizeof(info->rtx));
	memcpy(&info->pairing_info->m[8], info->rrx, sizeof(info->rrx));
	memcpy(info->pairing_info->km, info->km, sizeof(info->km));

	/* fill ekpub_km */
	sunxi_dp_hdcp2_encrypt_km(info, ekpub_km_buf);

	/* fill ekh_km, big-endian, DPCD value size is 8bit per reg */
	for (i = 0; i < HDCP2X_EKPUB_KM_SIZE; i++) {
		EDP_HDCP_DBG("[HDCP2X] ekpub_km_buf[%d]:0x%x\n", i, ekpub_km_buf[i]);
	}

	/* send ekh_km message */
	ret = edp_hw_aux_write(hdcp->edp_hw, HDCP2X_DPCD_EKPUB_KM,
				 sizeof(ekpub_km_buf), &ekpub_km_buf[0]);
	if (ret != RET_OK) {
		EDP_ERR("[HDCP2X] send ekpub_km fail\n");
		return ret;
	}

	return ret;
}

static s32 sunxi_dp_hdcp2_receive_ake_send_h_prime(struct sunxi_dptx_hdcp2_info *info)
{
	char hprime_buf[HDCP2X_H_PRIME_SIZE];
	int i;
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);

	/* read hprime */
	memset(hprime_buf, 0, sizeof(hprime_buf));
	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_H_PRIME,
			      HDCP2X_H_PRIME_SIZE, &hprime_buf[0]) < 0)
		return RET_FAIL;
	memcpy(&info->h_prime, hprime_buf, sizeof(hprime_buf));

	for (i = 0; i < HDCP2X_H_PRIME_SIZE; i++)
		EDP_HDCP_DBG("[HDCP2X]: H_PRIME[%d] : 0x%x", i, info->h_prime[i]);

	return RET_OK;

}

void sunxi_dp_hdcp2_invalidate_pairing_info(struct sunxi_dptx_hdcp2_info *info)
{
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);
	struct sunxi_dptx_hdcp2_pairing_info *pairing_info_ptr;

	/* remove the invalid pairing info that support before */
	list_for_each_entry(pairing_info_ptr, &hdcp->stored_pairing_head, list) {
		if (!memcmp(info->pairing_info->rcvid, pairing_info_ptr->rcvid, CERTRX_RCVID_SIZE)) {
			list_del(&pairing_info_ptr->list);
			memset(pairing_info_ptr, 0, sizeof(struct sunxi_dptx_hdcp2_pairing_info));
			kfree(pairing_info_ptr);
			break;
		}
	}
}

static s32 sunxi_dp_hdcp2_compare_h_hprime(struct sunxi_dptx_hdcp2_info *info)
{
	char h_prime[HDCP2X_H_PRIME_SIZE];
	u32 i;

	/* compute h(tx hprime) by hmac-sha256 */
	sunxi_hdcp2x_tx_compute_hprime(info->rrx,
				      info->rxcaps,
				      info->rtx,
				      info->txcaps,
				      info->km, h_prime);

	memcpy(info->h, h_prime, HDCP2X_H_PRIME_SIZE);

	for (i = 0; i < HDCP2X_H_PRIME_SIZE; i++)
		EDP_HDCP_DBG("[HDCP2X]: H[%d] : 0x%x", i, info->h[i]);

	if (memcmp(info->h, info->h_prime, HDCP2X_H_PRIME_SIZE)) {
		EDP_ERR("[HDCP2X]: compare H and H' fail\n");
		sunxi_dp_hdcp2_invalidate_pairing_info(info);
		return RET_FAIL;
	}

	return RET_OK;
}

static s32 sunxi_dp_hdcp2_receive_pairing_info(struct sunxi_dptx_hdcp2_info *info)
{
	char ekh_km_buf[HDCP2X_EKH_KM_RD_SIZE];
	int i;
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);

	/* read pairing info */
	memset(ekh_km_buf, 0, sizeof(ekh_km_buf));
	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_EKH_KM_RD,
			      HDCP2X_EKH_KM_RD_SIZE, &ekh_km_buf[0]) < 0)
		return RET_FAIL;

	/*fill ekh_km */
	memcpy(info->pairing_info->ekh_km, ekh_km_buf, sizeof(ekh_km_buf));

	for (i = 0; i < HDCP2X_EKH_KM_RD_SIZE; i++)
		EDP_HDCP_DBG("[HDCP2X]: ekh_km[%d] : 0x%x", i, info->pairing_info->ekh_km[i]);

	/* add new pairing info into local database */
	info->pairing_info->ready = true;
	list_add(&info->pairing_info->list, &hdcp->stored_pairing_head);

	return RET_OK;
}

static s32 sunxi_dp_hdcp2_send_lc_init(struct sunxi_dptx_hdcp2_info *info)
{
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);
	char lc_init_buf[HDCP2X_RN_SIZE];
	s32 ret = RET_FAIL;
	u32 i = 0;

	memset(lc_init_buf, 0, sizeof(lc_init_buf));

	/* fill rn */
	for (i = 0; i < HDCP2X_RTX_SIZE; i++)
		lc_init_buf[i] = get_random_u8();

	memcpy(info->rn, lc_init_buf, HDCP2X_RN_SIZE);

	for (i = 0; i < sizeof(lc_init_buf); i++)
		EDP_HDCP_DBG("[HDCP2X] lc_init_buf[%d]:0x%x\n", i, lc_init_buf[i]);

	/* send ake init message */
	ret = edp_hw_aux_write(hdcp->edp_hw, HDCP2X_DPCD_RN,
				 sizeof(lc_init_buf), &lc_init_buf[0]);
	return ret;
}

static s32 sunxi_dp_hdcp2_receive_lc_send_lprime(struct sunxi_dptx_hdcp2_info *info)
{
	char lprime_buf[HDCP2X_L_PRIME_SIZE];
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);
	int i;

	/* read certrx */
	memset(lprime_buf, 0, sizeof(lprime_buf));
	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_L_PRIME,
			      HDCP2X_L_PRIME_SIZE, &lprime_buf[0]) < 0)
		return RET_FAIL;

	/* fill L' */
	memcpy(&info->l_prime, lprime_buf, sizeof(lprime_buf));

	for (i = 0; i < HDCP2X_L_PRIME_SIZE; i++)
		EDP_HDCP_DBG("[HDCP2X]: L'[%d] : 0x%x", i, info->l_prime[i]);

	return RET_OK;
}

static s32 sunxi_dp_hdcp2_compare_l_lprime(struct sunxi_dptx_hdcp2_info *info)
{
	char lprime[HDCP2X_L_PRIME_SIZE];
	int i;

	/* compute tx local L */
	sunxi_hdcp2x_tx_compute_lprime(info->rn, info->km, info->rrx, info->rtx, lprime);

	if (memcmp(info->l_prime, lprime, HDCP2X_L_PRIME_SIZE)) {
		EDP_ERR("[HDCP2X]: compare L and L' fail!\n");
		return RET_FAIL;
	}

	/* fill L */
	memcpy(info->l, lprime, HDCP2X_L_PRIME_SIZE);

	for (i = 0; i < HDCP2X_L_PRIME_SIZE; i++)
		EDP_HDCP_DBG("[HDCP2X]: L[%d] : 0x%x", i, info->l[i]);

	return RET_OK;
}

static s32 sunxi_dp_hdcp2_compute_edkey_ks(struct sunxi_dptx_hdcp2_info *info)
{
	char riv[HDCP2X_RIV_SIZE];
	char ks[HDCP2X_KS_SIZE];
	char edkey_ks[HDCP2X_EDKEY_KS_SIZE];
	int i;

	/* fill riv */
	for (i = 0; i < HDCP2X_RIV_SIZE; i++)
		riv[i] = get_random_u8();

	/* fill ks */
	for (i = 0; i < HDCP2X_KS_SIZE; i++)
		ks[i] = get_random_u8();

	/* fill riv */
	memcpy(info->riv, riv, HDCP2X_RIV_SIZE);

	/* fill ks */
	memcpy(info->ks, ks, HDCP2X_KS_SIZE);

	sunxi_hdcp2x_tx_compute_edkey_ks(info->rn, info->km,
					info->ks, info->rrx,
					info->rtx, edkey_ks);

	/* fill Edkey_ks */
	memcpy(info->edkey_ks, edkey_ks, HDCP2X_EDKEY_KS_SIZE);

	return RET_OK;
}

static void sunxi_dp_hdcp2_encrypt_enable(struct sunxi_dptx_hdcp2_info *info, bool enable)
{
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);

	if (enable)
		dptx_hdcp2_hw_cipher_enable(hdcp->edp_hw, info->riv, info->ks);
	else
		dptx_hdcp2_hw_cipher_disable(hdcp->edp_hw);
}

static s32 sunxi_dp_hdcp2_send_edkey_ks(struct sunxi_dptx_hdcp2_info *info)
{
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);
	char edkey_ks_buf[HDCP2X_EDKEY_KS_SIZE + HDCP2X_RIV_SIZE];
	s32 ret = RET_FAIL;
	u32 i = 0;

	memset(edkey_ks_buf, 0, sizeof(edkey_ks_buf));

	/* fill edkey_ks into edkey_ks message */
	memcpy(edkey_ks_buf, info->edkey_ks, HDCP2X_EDKEY_KS_SIZE);

	/* fill riv into edkey_ks message */
	memcpy(&edkey_ks_buf[HDCP2X_EDKEY_KS_SIZE], info->riv, HDCP2X_RIV_SIZE);

	for (i = 0; i < sizeof(edkey_ks_buf); i++)
		EDP_HDCP_DBG("[HDCP2X] edkey_ks_buf[%d]:0x%x\n", i, edkey_ks_buf[i]);

	/* send ake init message */
	ret = edp_hw_aux_write(hdcp->edp_hw, HDCP2X_DPCD_EDKEY_KS,
				 sizeof(edkey_ks_buf), &edkey_ks_buf[0]);
	return ret;
}

static s32 sunxi_dp_hdcp2_authenticated(struct sunxi_dptx_hdcp2_info *info)
{
	sunxi_dp_hdcp2_encrypt_enable(info, true);

	return RET_OK;
}

static s32 sunxi_dp_hdcp2_check_downstream(struct sunxi_dptx_hdcp2_info *info)
{
	char rxinfo_buf[HDCP2X_RXINFO_SIZE];
	char seq_v_buf[HDCP2X_SEQ_NUM_V_SIZE];
	char v_prime_buf[HDCP2X_V_PRIME_SIZE];
	char rcv_list_buf[HDCP2X_RCVID_LIST_SIZE];
	int i;

	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);

	/* read rxinfo */
	memset(rxinfo_buf, 0, sizeof(rxinfo_buf));
	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_RXINFO,
			      HDCP2X_RXINFO_SIZE, &rxinfo_buf[0]) < 0)
		return RET_FAIL;

	for (i = 0; i < HDCP2X_RXINFO_SIZE; i++) {
		EDP_HDCP_DBG("[HDCP2X]: RXINFO'[%d] : 0x%x", i, rxinfo_buf[i]);
		info->rxinfo[i] = rxinfo_buf[i];
	}

	/* big-endian stored */
	info->repeater_dev_cnt = ((rxinfo_buf[0] & BIT(0)) << 4) | ((rxinfo_buf[1] & GENMASK(7, 4)) >> 4);
	info->repeater_dev_dep = ((rxinfo_buf[0] & GENMASK(3, 1)) >> 1);
	info->device_exceeded = (rxinfo_buf[1] & BIT(3)) ? true : false;
	info->cascade_exceeded = (rxinfo_buf[1] & BIT(2)) ? true : false;
	EDP_HDCP_DBG("[HDCP2X]: rpt_dev_cnt:%d rpt_dev_dep:%d dev_exceed:%d cascade_exceed:%d\n",
		     info->repeater_dev_cnt, info->repeater_dev_dep,
		     info->device_exceeded, info->cascade_exceeded);

	if (info->device_exceeded || info->cascade_exceeded) {
		EDP_ERR("[HDCP2X]: check downstream fail because of dev_exceed:%d cacade_exceed:%d\n",
			info->device_exceeded, info->cascade_exceeded);
		return RET_FAIL;
	}

	/* read seq_num_v */
	memset(seq_v_buf, 0, sizeof(seq_v_buf));
	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_SEQ_NUM_V,
			      HDCP2X_SEQ_NUM_V_SIZE, &seq_v_buf[0]) < 0)
		return RET_FAIL;
	memcpy(info->seq_num_v, seq_v_buf, HDCP2X_SEQ_NUM_V_SIZE);
	for (i = 0; i < HDCP2X_SEQ_NUM_V_SIZE; i++)
		EDP_HDCP_DBG("[HDCP2X]: seq_v_buf[%d] = 0x%x\n", i, seq_v_buf[i]);

	/* read v' */
	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_V_PRIME,
			      HDCP2X_V_PRIME_SIZE, &v_prime_buf[0]) < 0)
		return RET_FAIL;
	memcpy(info->v_prime, v_prime_buf, HDCP2X_V_PRIME_SIZE);
	for (i = 0; i < HDCP2X_V_PRIME_SIZE; i++)
		EDP_HDCP_DBG("[HDCP2X]: v_prime[%d] = 0x%x\n", i, v_prime_buf[i]);

	/* read rcvid list */
	if (edp_hw_aux_read(hdcp->edp_hw, HDCP2X_DPCD_RCVID_LIST,
			      HDCP2X_RCVID_LIST_SIZE, &rcv_list_buf[0]) < 0)
		return RET_FAIL;
	memcpy(info->rcvid_list, rcv_list_buf, HDCP2X_RCVID_LIST_SIZE);
	for (i = 0; i < HDCP2X_RCVID_LIST_SIZE; i++)
		EDP_HDCP_DBG("[HDCP2X]: rcvid_list[%d] = 0x%x\n", i, rcv_list_buf[i]);

	return RET_OK;
}

static s32 sunxi_dp_hdcp2_verify_v_vprime(struct sunxi_dptx_hdcp2_info *info)
{
	char v[HDCP2X_TX_V_SIZE];
	u32 i;

	sunxi_hdcp2x_tx_compute_v(info->rn, info->rrx, info->rxinfo,
				 info->rtx, info->rcvid_list,
				 info->repeater_dev_cnt,
				 info->seq_num_v,
				 info->km, v);
	memcpy(info->v, v, HDCP2X_V_PRIME_SIZE);

	for (i = 0; i < HDCP2X_V_PRIME_SIZE; i++)
		EDP_HDCP_DBG("[HDCP2X]: v[%d] = 0x%x\n", i, v[i]);

	if (memcmp(info->v_prime, v,
		   HDCP2X_V_PRIME_SIZE)) {
		EDP_ERR("[HDCP2X]: v-prime compare fail\n");
		return RET_FAIL;
	}

	return RET_OK;
}

static s32 sunxi_dp_hdcp2_check_revoked(struct sunxi_dptx_hdcp2_info *info)
{
	/*
	 * TODO: update revoked list from HDCP organization and forbid device
	 * has been set in the list. Now we skip it.
	 */

	return RET_OK;
}

static s32 sunxi_dp_hdcp2_send_rcvid_list_ack(struct sunxi_dptx_hdcp2_info *info)
{
	struct sunxi_dp_hdcp *hdcp = sunxi_hdcp2_info_to_sunxi_hdcp(info);
	s32 ret;

	ret = edp_hw_aux_write(hdcp->edp_hw, HDCP2X_DPCD_V,
				 HDCP2X_V_SIZE, &info->v[0]);
	if (ret != RET_OK)
		return ret;

	return RET_OK;
}


s32 sunxi_dp_hdcp2_auth(struct sunxi_dp_hdcp *hdcp)
{
	struct sunxi_dptx_hdcp2_info *info = &hdcp->hdcp2_info;
	s32 ret = RET_OK;
	long timeout = 0;

	while (!hdcp2_status_success(info) && !hdcp2_status_fail(info)) {
		switch (info->state) {
		case HDCP2_DP_STATE_NONE:
			EDP_ERR("hdcp2 for dp not enable yet!\n");
			return RET_OK;
		case HDCP2_DP_STATE_START:
			sunxi_dp_hdcp2_set_state(info, HDCP2_A0_DETERMINE_RX_HDCP_CAPABLE, 0);
			break;
		case HDCP2_A0_DETERMINE_RX_HDCP_CAPABLE:
			ret = sunxi_dp_hdcp2_read_parse_rxcaps(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A0_READ_RXCAPS_FAIL, 0);

			if (hdcp->hdcp2_capable)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_EXCHANGE_KM_AKE_INIT, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A0_HDCP_NOT_CAPABLE, 0);
			break;
		case HDCP2_A1_EXCHANGE_KM_AKE_INIT:
			ret = sunxi_dp_hdcp2_send_ake_init(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_SEND_AKE_INIT_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_EXCHANGE_KM_AKE_SEND_CERT, 110); //100ms at least
			break;
		case HDCP2_A1_EXCHANGE_KM_AKE_SEND_CERT:
			ret = sunxi_dp_hdcp2_receive_ake_send_cert(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_READ_AKE_SEND_CERT_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_EXCHANGE_KM_VERIFY_CERTRX, 0);
			break;
		case HDCP2_A1_EXCHANGE_KM_VERIFY_CERTRX:
			ret = sunxi_dp_hdcp2_verify_cert(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_VERIFY_CERTRX_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_EXCHANGE_KM_GET_PAIRING_INFO, 0);
			break;
		case HDCP2_A1_EXCHANGE_KM_GET_PAIRING_INFO:
			if (sunxi_dp_hdcp2_get_pairing_info(info)) {
				info->no_stored_km = false;
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_EXCHANGE_KM_AKE_STORED_KM, 0);
			} else {
				info->no_stored_km = true;
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_EXCHANGE_KM_AKE_NO_STORED_KM, 0);
			}
			break;
		case HDCP2_A1_EXCHANGE_KM_AKE_STORED_KM:
			ret = sunxi_dp_hdcp2_ake_stored_km(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_AKE_STORED_KM_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_EXCHANGE_KM_AKE_WAIT_H_PRIME, 0);
			break;
		case HDCP2_A1_EXCHANGE_KM_AKE_NO_STORED_KM:
			ret = sunxi_dp_hdcp2_ake_no_stored_km(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_AKE_NO_STORED_KM_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_EXCHANGE_KM_AKE_WAIT_H_PRIME, 0);
			break;
		case HDCP2_A1_EXCHANGE_KM_AKE_WAIT_H_PRIME:
			/* refer from HDCP2X spec, we should not read h_prime_available  within 1s*/
			timeout = wait_event_timeout(hdcp->auth_queue, info->h_prime_available,
						     msecs_to_jiffies(1000));
			/*
			 * usually H PRIME AVAILABLE BIT has been set after CP_IRQ income but some
			 * system not support CP_IRQ, so read rxstatus manaualy after 1s timeout
			 */
			if (!info->h_prime_available) {
				if (hdcp2_read_parse_rxstatus(info) != RET_OK) {
					sunxi_dp_hdcp2_set_state(info, HDCP2_A1_READ_RXSTATUS_FAIL, 0);
					break;
				}

				if (!info->h_prime_available) {
					sunxi_dp_hdcp2_set_state(info, HDCP2_A1_WAIT_H_PRIME_FAIL, 0);
					break;
				}
			}

			ret = sunxi_dp_hdcp2_receive_ake_send_h_prime(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_AKE_SEND_H_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_EXCHANGE_KM_AKE_COMPARE_H_HPRIME, 0);
			break;
		case HDCP2_A1_EXCHANGE_KM_AKE_COMPARE_H_HPRIME:
			ret = sunxi_dp_hdcp2_compare_h_hprime(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_COMPARE_H_HPRIME_FAIL, 0);
			else {
				if (info->no_stored_km)
					sunxi_dp_hdcp2_set_state(info, HDCP2_A1_EXCHANGE_KM_PAIRING, 0);
				else
					sunxi_dp_hdcp2_set_state(info, HDCP2_A2_LOCALITY_CHECK, 0);
			}
			break;
		case HDCP2_A1_EXCHANGE_KM_PAIRING:
			/* refer from HDCP2X spec, we should not read pairing_available within 200ms */
			timeout = wait_event_timeout(hdcp->auth_queue, info->pairing_available,
						     msecs_to_jiffies(200));
			/*
			 * usually PAIRING AVAILABLE BIT has been set after CP_IRQ income but some
			 * system not support CP_IRQ, so read rxstatus manaualy after 1s timeout
			 */
			if (!info->pairing_available) {
				if (hdcp2_read_parse_rxstatus(info) != RET_OK) {
					sunxi_dp_hdcp2_set_state(info, HDCP2_A1_READ_RXSTATUS_FAIL, 0);
					break;
				}

				if (!info->pairing_available) {
					sunxi_dp_hdcp2_set_state(info, HDCP2_A1_WAIT_PAIRING_INFO_FAIL, 0);
					break;
				}
			}
			ret = sunxi_dp_hdcp2_receive_pairing_info(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A1_WAIT_PAIRING_INFO_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A2_LOCALITY_CHECK, 0);
			break;
		case HDCP2_A2_LOCALITY_CHECK:
			ret = sunxi_dp_hdcp2_send_lc_init(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A2_SEND_LC_INIT_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A2_LOCALITY_CHECK_WAIT_L_PRIME, 20); //within 7ms at least
			break;
		case HDCP2_A2_LOCALITY_CHECK_WAIT_L_PRIME:
			ret = sunxi_dp_hdcp2_receive_lc_send_lprime(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A2_RECEIVE_L_PRIME_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A2_LOCALITY_CHECK_VERIFY_L_PRIME, 0);
			break;
		case HDCP2_A2_LOCALITY_CHECK_VERIFY_L_PRIME:
			ret = sunxi_dp_hdcp2_compare_l_lprime(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A2_VERIFY_L_PRIME_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A3_EXCHANGE_KS, 0);
			break;
		case HDCP2_A3_EXCHANGE_KS:
			ret = sunxi_dp_hdcp2_compute_edkey_ks(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A3_COMPUTE_EDKEY_KS_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A3_EXCHANGE_KS_SEND_EKS, 0);
			break;
		case HDCP2_A3_EXCHANGE_KS_SEND_EKS:
			ret = sunxi_dp_hdcp2_send_edkey_ks(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A3_SEND_EDKEY_KS_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A4_REPEATER_CHECK, 0);
			break;
		case HDCP2_A4_REPEATER_CHECK:
			if (!info->rx_is_repeater) {
				/* 200ms is need between SEND_EKS and HDCP ENCRY */
				sunxi_dp_hdcp2_set_state(info, HDCP2_A5_AUTHENTICATED, 200);
			} else {
				sunxi_dp_hdcp2_set_state(info, HDCP2_A6_WAIT_FOR_RCVID, 0);
			}
			break;
		case HDCP2_A6_WAIT_FOR_RCVID:
			/* refer from HDCP2X spec, repeater's READY bit should be set within 3s */
			/* all of these rxstatus flag bit should be read when cpirq trigger*/
			if (hdcp2_read_parse_rxstatus(info) != RET_OK) {
				sunxi_dp_hdcp2_set_state(info, HDCP2_A6_READ_RXSTATUS_FAIL, 0);
				break;
			}
			timeout = wait_event_timeout(hdcp->auth_queue, info->v_prime_ready,
						     msecs_to_jiffies(3000));
			/*
			 * usually READY BIT has been set after CP_IRQ income but some
			 * system not support CP_IRQ, so read rxstatus manaualy after 1s timeout
			 */
			if (!info->v_prime_ready) {
				if (hdcp2_read_parse_rxstatus(info) != RET_OK) {
					sunxi_dp_hdcp2_set_state(info, HDCP2_A6_READ_RXSTATUS_FAIL, 0);
					break;
				}

				if (!info->v_prime_ready) {
					sunxi_dp_hdcp2_set_state(info, HDCP2_A6_WAIT_FOR_RCVID_FAIL, 0);
					break;
				}
			}

			ret = sunxi_dp_hdcp2_check_downstream(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A6_CHECK_DOWNSTREAM_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A7_VERIFY_RCVID_LIST, 0);
			break;
		case HDCP2_A7_VERIFY_RCVID_LIST:
			ret = sunxi_dp_hdcp2_verify_v_vprime(info);
			if (ret != RET_OK) {
				sunxi_dp_hdcp2_set_state(info, HDCP2_A7_VERIFY_VPRIME_FAIL, 0);
				break;
			}

			ret = sunxi_dp_hdcp2_check_revoked(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A7_CHECK_REVOKED_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_A8_SEND_RCVID_LIST_ACK, 0);
			break;
		case HDCP2_A8_SEND_RCVID_LIST_ACK:
			ret = sunxi_dp_hdcp2_send_rcvid_list_ack(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A8_SEND_RCVID_LIST_ACK_FAIL, 0);
			else
				//sunxi_dp_hdcp2_set_state(info, HDCP2_A9_CONTENT_STREAM_MANAGEMENT, 0);
				sunxi_dp_hdcp2_set_state(info, HDCP2_A5_AUTHENTICATED, 0);
			break;
		case HDCP2_A9_CONTENT_STREAM_MANAGEMENT:
			//TODO: the stage may be used for MST mode or muti content
			//stream mode, useless for traditional transmiter
			sunxi_dp_hdcp2_set_state(info, HDCP2_A5_AUTHENTICATED, 0);
			break;
		case HDCP2_A5_AUTHENTICATED:
			ret = sunxi_dp_hdcp2_authenticated(info);
			if (ret != RET_OK)
				sunxi_dp_hdcp2_set_state(info, HDCP2_A5_AUTHENTICATED_FAIL, 0);
			else
				sunxi_dp_hdcp2_set_state(info, HDCP2_DP_STATE_END, 0);
			break;
		case HDCP2_DP_STATE_END:
			sunxi_dp_hdcp2_set_status(info, DP_HDCP_STATUS_SUCCESS, 0);
			ret = RET_OK;
			break;
		case HDCP2_A0_READ_RXCAPS_FAIL:
		case HDCP2_A0_HDCP_NOT_CAPABLE:
		case HDCP2_A1_SEND_AKE_INIT_FAIL:
		case HDCP2_A1_READ_AKE_SEND_CERT_FAIL:
		case HDCP2_A1_VERIFY_CERTRX_FAIL:
		case HDCP2_A1_AKE_STORED_KM_FAIL:
		case HDCP2_A1_AKE_NO_STORED_KM_FAIL:
		case HDCP2_A1_READ_RXSTATUS_FAIL:
		case HDCP2_A1_WAIT_H_PRIME_FAIL:
		case HDCP2_A1_AKE_SEND_H_FAIL:
		case HDCP2_A1_COMPARE_H_HPRIME_FAIL:
		case HDCP2_A1_WAIT_PAIRING_INFO_FAIL:
		case HDCP2_A2_SEND_LC_INIT_FAIL:
		case HDCP2_A2_RECEIVE_L_PRIME_FAIL:
		case HDCP2_A2_VERIFY_L_PRIME_FAIL:
		case HDCP2_A3_COMPUTE_EDKEY_KS_FAIL:
		case HDCP2_A3_SEND_EDKEY_KS_FAIL:
		case HDCP2_A5_AUTHENTICATED_FAIL:
		default:
			EDP_ERR("dp hdcp2 authentication fail, reason:%s\n", hdcp2_fail_reason[info->state]);
			sunxi_dp_hdcp2_set_status(info, DP_HDCP_STATUS_FAIL, 0);
			//sunxi_dp_hdcp1_clear_r0_prime_ready(info);
			//sunxi_dp_hdcp1_clear_ksv_list_ready(info);
			return RET_FAIL;
		}
	}

	return ret;
}


s32 sunxi_dp_hdcp2_disable(struct sunxi_dp_hdcp *hdcp)
{
	struct sunxi_dptx_hdcp2_info *info = &hdcp->hdcp2_info;

	sunxi_dp_hdcp2_encrypt_enable(info, false);
	sunxi_dp_hdcp2_clear_info(info);
	dptx_hdcp_hw_enable(hdcp->edp_hw, false);
	dptx_hdcp_hw_set_mode(hdcp->edp_hw, HDCP_NONE_MODE);

	return RET_OK;
}

s32 sunxi_dp_hdcp2_enable(struct sunxi_dp_hdcp *hdcp)
{
	struct sunxi_dptx_hdcp2_info *info = &hdcp->hdcp2_info;
	u32 i = 0;
	s32 ret = 0;
	sunxi_dp_hdcp2_set_status(info, DP_HDCP_STATUS_NONE, 0);

	// FIXME: not sure if need: clear all exit info,
	// begin authencation at the original procedure

	dptx_hdcp_hw_set_mode(hdcp->edp_hw, HDCP23_MODE);
	dptx_hdcp_hw_enable(hdcp->edp_hw, true);
	sunxi_dp_hdcp2_set_state(info, HDCP2_DP_STATE_START, 0);

	for (i = 0; i < HDCP2X_REAUTH_CNT; i++) {
		ret = sunxi_dp_hdcp2_auth(hdcp);
		if (ret == RET_OK) {
			EDP_INFO("[HDCP2X]: hdcp2 authencitate success, start video transmit!\n");
			return ret;
		} else {
			sunxi_dp_hdcp2_clear_info(info);
			sunxi_dp_hdcp2_set_state(info, HDCP2_DP_STATE_START, 0);
		}
	}

	EDP_ERR("hdcp1 retry 3 times but still fail, authentication fail!\n");
	sunxi_dp_hdcp2_disable(hdcp);

	return RET_FAIL;

}
