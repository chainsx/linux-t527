/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * edp_core.h
 *
 * Copyright (c) 2007-2022 Allwinnertech Co., Ltd.
 * Author: huangyongxing <huangyongxing@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef __SUNXI_DPTX_HDCP_H__
#define __SUNXI_DPTX_HDCP_H__

#include "../../sunxi_edp.h"

/* DPCD For HDCP1.x*/
#define DPCD_68000H						(0x68000)
#define DPCD_68005H						(0x68005)
#define DPCD_68007H						(0x68007)
#define DPCD_6800CH						(0x6800C)
#define DPCD_68014H						(0x68014)
#define DPCD_68028H						(0x68028)
#define DPCD_68029H						(0x68029)
#define DPCD_6802AH						(0x6802A)
#define DPCD_6802CH						(0x6802C)
#define DPCD_6803BH						(0x6803B)
#define DPCD_6803CH						(0x6803C)
#define DPCD_680C0H						(0x680C0)

#define HDCP1X_DPCD_BKSV				DPCD_68000H
#define HDCP1X_BKSV_SIZE				(5)

#define HDCP1X_DPCD_R0_PRIME			DPCD_68005H
#define HDCP1X_R0_PRIME_SIZE			(2)

#define HDCP1X_DPCD_AKSV				DPCD_68007H
#define HDCP1X_AKSV_SIZE				(5)

#define HDCP1X_DPCD_AN					DPCD_6800CH
#define HDCP1X_AN_SIZE					(8)

#define HDCP1X_DPCD_V_PRIME				DPCD_68014H
#define HDCP1X_V_PRIME_SIZE				(20)

#define HDCP1X_DPCD_BCAPS				DPCD_68028H
#define HDCP1X_BCAPS_SIZE				(1)
#define HDCP1X_CAPABLE					BIT(0)
#define HDCP1X_REPEATER					BIT(1)

#define HDCP1X_DPCD_BSTATUS				DPCD_68029H
#define HDCP1X_BSTATUS_SIZE				(1)
#define HDCP1X_KSV_LIST_READY			BIT(0)
#define HDCP1X_R0_PRIME_AVAILABLE		BIT(1)
#define HDCP1X_REAUTHENTICATION_REQ		BIT(3)

#define HDCP1X_DPCD_BINFO				DPCD_6802AH
#define HDCP1X_BINFO_SIZE				(2)
#define HDCP1X_M0_SIZE					(8)
#define HDCP1X_REPEATER_DEVICE_CNT_MASK	0x7F
#define HDCP1X_REPEATER_DEVICE_EXCEED	BIT(7)
#define HDCP1X_REPEATER_DEVICE_DEP_MASK	0x7
#define HDCP1X_REPEATER_CASCADE_EXCEED	BIT(4)

#define HDCP1X_DPCD_KSV_FIFO			DPCD_6802CH
#define HDCP1X_KSV_FIFO_SIZE			(15)

#define HDCP1X_DPCD_KSV_AINFO			DPCD_6803BH
#define HDCP1X_KSV_AINFO_SIZE			(1)
#define HDCP1X_REAUTH_ENABLE_IRQ_HPD	BIT(0)

#define HDCP1X_DPCD_RESERVED			DPCD_6803CH
#define HDCP1X_RESERVED_SIZE			(132)

#define HDCP1X_DPCD_DEBUG				DPCD_680C0H
#define HDCP1X_DEBUG_SIZE				(64)


/* HDCP1 reference */
#define HDCP1_MAX_REPEATER_DEV_CNT			(127)
#define HDCP1_MAX_REPEATER_DEV_DEP			(7)
#define HDCP1_KSV_LIST_BYTES_PER_DEV		(5)
#define HDCP1_KSV_LIST_FIFO_BYTES			(15)
#define HDCP1_DEV_CNT_PER_KSV_LIST_FIFO		(3)


/* DPCD For HDCP2.x*/
#define DPCD_69000H						(0x69000)
#define DPCD_69008H						(0x69008)
#define DPCD_6900BH						(0x6900B)
#define DPCD_69215H						(0x69215)
#define DPCD_6921DH						(0x6921D)
#define DPCD_69220H						(0x69220)
#define DPCD_692A0H						(0x692A0)
#define DPCD_692B0H						(0x692B0)
#define DPCD_692C0H						(0x692C0)
#define DPCD_692E0H						(0x692E0)
#define DPCD_692F0H						(0x692F0)
#define DPCD_692F8H						(0x692F8)
#define DPCD_69318H						(0x69318)
#define DPCD_69328H						(0x69328)
#define DPCD_69330H						(0x69330)
#define DPCD_69332H						(0x69332)
#define DPCD_69335H						(0x69335)
#define DPCD_69345H						(0x69345)
#define DPCD_693E0H						(0x693E0)
#define DPCD_693F0H						(0x693F0)
#define DPCD_693F3H						(0x693F3)
#define DPCD_693F5H						(0x693F5)
#define DPCD_69473H						(0x69473)
#define DPCD_69493H						(0x69493)
#define DPCD_69494H						(0x69494)
#define DPCD_69518H						(0x69518)

/* HDCP2 reference */
#define HDCP2X_DPCD_RTX					DPCD_69000H
#define HDCP2X_RTX_SIZE					(8)

#define HDCP2X_DPCD_TXCAPS				DPCD_69008H
#define HDCP2X_TXCAPS_SIZE				(3)

#define HDCP2X_DPCD_CERTRX				DPCD_6900BH
#define HDCP2X_CERTRX_SIZE				(522)
/* big-endian store */
#define CERTRX_RCVID_SIZE				(5)
#define HDCP2X_RECEIVER_ID_SIZE			CERTRX_RCVID_SIZE
#define CERTRX_RCVID_OFFSET				(0)
#define CERTRX_PUBLIC_KEY_SIZE			(131)
#define CERTRX_PUBLIC_KEY_N_SIZE		(128)
#define CERTRX_PUBLIC_KEY_E_SIZE		(3)
#define CERTRX_PUBLIC_KEY_OFFSET		CERTRX_RCVID_SIZE
#define CERTRX_RESERVED_SIZE			(2)
#define CERTRX_SIGNATURE_SIZE			(384)
#define CERTRX_SIGNATURE_OFFSET			(CERTRX_PUBLIC_KEY_OFFSET +\
										 CERTRX_PUBLIC_KEY_SIZE + CERTRX_RESERVED_SIZE)
#define CERTRX_PADDING_BYTES		330
#define CERTRX_PADDING_END_DELIMITER	332
#define CERTRX_PADDING_TI_IDENTIFIER	333
#define CERTRX_PADDING_T_HASH		352
#define SRM_SIGNATURE_SIZE		384
#define HDCP2X_TX_CERT_RSA_PARAMETER_SIZE 384
#define CERTRX_TI_IDENTIFIER_SIZE 19
#define CERTRX_T_HASH_SIZE 19

#define HDCP2X_DPCD_RRX					DPCD_69215H
#define HDCP2X_RRX_SIZE					(8)

#define HDCP2X_DPCD_RXCAPS				DPCD_6921DH
#define HDCP2X_RXCAPS_SIZE				(3)
#define RXCAPS_REPEATER_BIT				BIT(0)
#define RXCAPS_CAPABLE_BIT				BIT(1)
#define RXCAPS_VERSION_MASK				GENMASK(23, 16)
#define RXCAPS_VERSION_OFFSET			(16)

#define HDCP2X_DPCD_EKPUB_KM			DPCD_69220H
#define HDCP2X_EKPUB_KM_SIZE			(128)

#define HDCP2X_DPCD_EKH_KM_WR			DPCD_692A0H
#define HDCP2X_EKH_KM_WR_SIZE			(16)

#define HDCP2X_DPCD_M					DPCD_692B0H
#define HDCP2X_M_SIZE					(16)

#define HDCP2X_DPCD_H_PRIME				DPCD_692C0H
#define HDCP2X_H_PRIME_SIZE				(32)
#define HDCP2X_H_SIZE					(32)

#define HDCP2X_DPCD_EKH_KM_RD			DPCD_692E0H
#define HDCP2X_EKH_KM_RD_SIZE			(16)

#define HDCP2X_DPCD_RN					DPCD_692F0H
#define HDCP2X_RN_SIZE					(8)

#define HDCP2X_DPCD_L_PRIME				DPCD_692F8H
#define HDCP2X_L_PRIME_SIZE				(32)

#define HDCP2X_DPCD_EDKEY_KS			DPCD_69318H
#define HDCP2X_EDKEY_KS_SIZE			(16)
#define HDCP2X_KS_SIZE					HDCP2X_EDKEY_KS_SIZE

#define HDCP2X_DPCD_RIV					DPCD_69328H
#define HDCP2X_RIV_SIZE					(8)

#define HDCP2X_DPCD_RXINFO				DPCD_69330H
#define HDCP2X_RXINFO_SIZE				(2)

#define HDCP2X_DPCD_SEQ_NUM_V			DPCD_69332H
#define HDCP2X_SEQ_NUM_V_SIZE			(3)

#define HDCP2X_DPCD_V_PRIME				DPCD_69335H
#define HDCP2X_V_PRIME_SIZE				(16)

#define HDCP2X_DPCD_RCVID_LIST			DPCD_69345H
#define HDCP2X_RCVID_LIST_SIZE			(155)

#define HDCP2X_DPCD_V					DPCD_693E0H
#define HDCP2X_V_SIZE					(16)

#define HDCP2X_DPCD_SEQ_NUM_M			DPCD_693F0H
#define HDCP2X_SEQ_NUM_M_SIZE			(3)

#define HDCP2X_DPCD_K					DPCD_693F3H
#define HDCP2X_K_SIZE					(2)

#define HDCP2X_DPCD_STREAMID_TYPE		DPCD_693F5H
#define HDCP2X_STREAMID_TYPE_SIZE		(126)

#define HDCP2X_DPCD_M_PRIME				DPCD_69473H
#define HDCP2X_M_PRIME_SIZE				(32)

#define HDCP2X_DPCD_RXSTATUS			DPCD_69493H
#define HDCP2X_RXSTATUS_SIZE			(1)
#define RXSTATUS_V_PRIME_READY_BIT		BIT(0)
#define RXSTATUS_H_PRIME_AVA_BIT		BIT(1)
#define RXSTATUS_PAIRING_AVA_BIT		BIT(2)
#define RXSTATUS_REQUTH_REQ_BIT			BIT(3)
#define RXSTATUS_LINK_INTEG_FAIL_BIT	BIT(4)

#define HDCP2X_DPCD_RESERVED			DPCD_69494H
#define HDCP2X_RESERVED_SIZE			(132)

#define HDCP2X_DPCD_DEBUG				DPCD_69518H
#define HDCP2X_DEBUG_SIZE				(64)

#define HDCP2X_TX_KPUB_DCP_LLC_N_SIZE		 	384
#define HDCP2X_TX_KPUB_DCP_LLC_E_SIZE			1
#define HDCP2X_TX_SHA256_HASH_SIZE				32
#define SUNXI_DPTX_HDCP2_STORED_PAIRINGINFO_MAX	16
#define HDCP2X_TX_V_SIZE			32
#define HDCP2X_TX_MAX_DEV_COUNT			31
#define HDCP2X_TX_K_SIZE			2
#define HDCP2X_TX_STREAMID_TYPE_SIZE		2
#define HDCP2X_TX_SHA256_HASH_SIZE		32
#define HDCP2X_TX_AES128_SIZE			16
#define HDCP2X_TX_KM_SIZE			HDCP2X_TX_AES128_SIZE
#define HDCP2X_TX_KM_MSK_SEED_SIZE		HDCP2X_TX_SHA256_HASH_SIZE
#define HDCP2X_TX_KS_SIZE			16
#define HDCP2X_TX_DKEY				15
#define HDCP2X_TX_DKEY_CTR1			1
#define HDCP2X_TX_DKEY_CTR2			2
#define HDCP2X_TX_DKEY_SIZE			2
#define BD_MAX_MOD_SIZE  (HDCP2X_TX_CERT_RSA_PARAMETER_SIZE / sizeof(u32))
#define HDCP2X_TX_CERT_PUBLIC_EXPONENT_E	4
#define HDCP2X_RCVID_LEN		CERTRX_RCVID_SIZE

enum sunxi_dp_hdcp_mode {
	DP_HDCP_DISABLE,
	DP_HDCP1X_ONLY,
	DP_HDCP1X_PREFER,
	DP_HDCP2X_ONLY,
	DP_HDCP2X_PREFER,
};

enum sunxi_dp_hdcp_status {
	DP_HDCP_STATUS_NONE = 0,
	DP_HDCP_STATUS_SUCCESS,
	DP_HDCP_STATUS_FAIL,
	DP_HDCP_STATUS_PENDING,
};

enum sunxi_dp_hdcp1_state {
	HDCP1_DP_STATE_NONE = 0,
	HDCP1_DP_STATE_START,
	HDCP1_A0_DETERMINE_RX_HDCP_CAPABLE,
	HDCP1_A1_EXCHANGE_KSVS_WRITE_AN_AKSV,
	HDCP1_A1_EXCHANGE_KSVS_READ_BKSV,
	HDCP1_A1_EXCHANGE_KSVS_VALIDATE_BKSV,
	HDCP1_A1_EXCHANGE_KSVS_CAL_KM,
	HDCP1_A2_COMPUTATIONS,
	HDCP1_A2_WRITE_AN_AKSV,
	HDCP1_A2_A3_WAIT_FOR_R0_PRIME,
	HDCP1_A2_A3_READ_R0_PRIME,
	HDCP1_A3_VALIDATE_RX,
	HDCP1_A5_TEST_FOR_REPEATER,
	HDCP1_A6_WAIT_FOR_READY,
	HDCP1_A7_READ_KSV_LIST,
	HDCP1_A7_CALCULATE_V,
	HDCP1_A7_VERIFY_V_V_PRIME,
	HDCP1_A4_AUTHENTICATED,
	HDCP1_DP_STATE_END,

	HDCP1_A0_READ_BCAPS_FAIL,
	HDCP1_A0_HDCP_NOT_CAPABLE,
	HDCP1_A1_READ_BKSV_FAIL,
	HDCP1_A1_VALIDATE_BKSV_FAIL,
	HDCP1_A1_WRITE_AINFO_FAIL,
	HDCP1_A2_WRITE_AN_AKSV_FAIL,
	HDCP1_A2_A3_READ_R0_PRIME_FAIL,
	HDCP1_A3_VALIDATE_RX_FAIL,
	HDCP1_A6_READ_BSTATUS_BINFO_FAIL,
	HDCP1_A6_WAIT_FOR_READY_FAIL,
	HDCP1_A6_REPEATER_DEV_DEP_EXCEED_FAIL,
	HDCP1_A7_READ_KSV_LIST_FAIL,
	HDCP1_A7_READ_V_PRIME_FAIL,
	HDCP1_A7_VERIFY_V_V_PRIME_FAIL,
};

static const char * const hdcp1_fail_reason[] = {
	[HDCP1_A0_READ_BCAPS_FAIL]				= "HDCP1_A0_READ_BCAPS_FAIL",
	[HDCP1_A0_HDCP_NOT_CAPABLE]				= "HDCP1_A0_HDCP_NOT_CAPABLE",
	[HDCP1_A1_READ_BKSV_FAIL]				= "HDCP1_A1_READ_BKSV_FAIL",
	[HDCP1_A1_VALIDATE_BKSV_FAIL]			= "HDCP1_A1_VALIDATE_BKSV_FAIL",
	[HDCP1_A1_WRITE_AINFO_FAIL]				= "HDCP1_A1_WRITE_AINFO_FAIL",
	[HDCP1_A2_WRITE_AN_AKSV_FAIL]			= "HDCP1_A2_WRITE_AN_AKSV_FAIL",
	[HDCP1_A2_A3_READ_R0_PRIME_FAIL]		= "HDCP1_A2_A3_READ_R0_PRIME_FAIL",
	[HDCP1_A3_VALIDATE_RX_FAIL]				= "HDCP1_A3_VALIDATE_RX_FAIL",
	[HDCP1_A6_READ_BSTATUS_BINFO_FAIL]		= "HDCP1_A6_READ_BSTATUS_BINFO_FAIL",
	[HDCP1_A6_WAIT_FOR_READY_FAIL]			= "HDCP1_A6_WAIT_FOR_READY_FAIL",
	[HDCP1_A6_REPEATER_DEV_DEP_EXCEED_FAIL] = "HDCP1_A6_REPEATER_DEV_DEP_EXCEED_FAIL",
	[HDCP1_A7_READ_KSV_LIST_FAIL]			= "HDCP1_A7_READ_KSV_LIST_FAIL",
	[HDCP1_A7_READ_V_PRIME_FAIL]			= "HDCP1_A7_READ_V_PRIME_FAIL",
	[HDCP1_A7_VERIFY_V_V_PRIME_FAIL]		= "HDCP1_A7_VERIFY_V_V_PRIME_FAIL",
};

/* HDCP1X: muti-data are stored in little-endian in DPCD */
/* HDCP1X: DCP_LLC ir SRM data are stored in big-endian */
struct sunxi_dptx_hdcp1_info {
	enum sunxi_dp_hdcp_status status;
	enum sunxi_dp_hdcp1_state state;

	/* retry count when read V prime fail */
	u32 v_read_retry;

	/* retry count when read r0 prime fail */
	u32 r0_prime_retry;

	/* status parse from HDCP's DPCD*/
	bool r0_prime_ready;
	bool ksv_list_ready;
	bool hdcp1_capable;
	bool rx_is_repeater;
	bool device_exceeded;
	bool cascade_exceeded;

	/* 8-bit data indicate rx capability */
	u8 bcaps;

	/* 40-bit data*/
	u64 aksv;

	/* 64-bit Pseudo random value */
	u64 an;

	/* 40-bit data*/
	u64 bksv;

	/* 64-bit after-calculate data */
	u64 km;

	/* 16-bit after-calculate data */
	u32 r0;
	u32 r0_prime;

	/* ksv list contain all device, count = 5 * dev_cnt*/
	u8 ksv_list[HDCP1_MAX_REPEATER_DEV_CNT * HDCP1_KSV_LIST_BYTES_PER_DEV];
	u32 ksv_cnt;

	/* 16-bit data from DPCD */
	u8 binfo[HDCP1X_BINFO_SIZE];

	/* 64-bit data generate by hdcp tx */
	u8 m0[HDCP1X_M0_SIZE];

	/* 20-bytes data after-calculate from hdcp rx */
	u8 v_prime[HDCP1X_V_PRIME_SIZE];
	u8 v[HDCP1X_V_PRIME_SIZE];

	/* information parse from binfo */
	u32 repeater_dev_cnt;
	u32 repeater_dev_dep;
};

enum sunxi_dp_hdcp2_state {
	HDCP2_DP_STATE_NONE = 0,
	HDCP2_DP_STATE_START,
	HDCP2_A0_DETERMINE_RX_HDCP_CAPABLE,
	HDCP2_A1_EXCHANGE_KM_AKE_INIT,
	HDCP2_A1_EXCHANGE_KM_AKE_SEND_CERT,
	HDCP2_A1_EXCHANGE_KM_VERIFY_CERTRX,
	HDCP2_A1_EXCHANGE_KM_GET_PAIRING_INFO,
	HDCP2_A1_EXCHANGE_KM_AKE_NO_STORED_KM,
	HDCP2_A1_EXCHANGE_KM_AKE_STORED_KM,
	HDCP2_A1_EXCHANGE_KM_AKE_WAIT_H_PRIME,
	HDCP2_A1_EXCHANGE_KM_AKE_COMPARE_H_HPRIME,
	HDCP2_A1_EXCHANGE_KM_AKE_INVALIDATE_PAIRING,
	HDCP2_A1_EXCHANGE_KM_PAIRING,
	HDCP2_A2_LOCALITY_CHECK,
	HDCP2_A2_LOCALITY_CHECK_WAIT_L_PRIME,
	HDCP2_A2_LOCALITY_CHECK_VERIFY_L_PRIME,
	HDCP2_A3_EXCHANGE_KS,
	HDCP2_A3_EXCHANGE_KS_SEND_EKS,
	HDCP2_A4_REPEATER_CHECK,
	HDCP2_A5_AUTHENTICATED,
	HDCP2_A6_WAIT_FOR_RCVID,
	HDCP2_A7_VERIFY_RCVID_LIST,
	HDCP2_A8_SEND_RCVID_LIST_ACK,
	HDCP2_A9_CONTENT_STREAM_MANAGEMENT,
	HDCP2_DP_STATE_END,


	HDCP2_A0_READ_RXCAPS_FAIL,
	HDCP2_A0_HDCP_NOT_CAPABLE,
	HDCP2_A1_SEND_AKE_INIT_FAIL,
	HDCP2_A1_READ_AKE_SEND_CERT_FAIL,
	HDCP2_A1_VERIFY_CERTRX_FAIL,
	HDCP2_A1_AKE_STORED_KM_FAIL,
	HDCP2_A1_AKE_NO_STORED_KM_FAIL,
	HDCP2_A1_READ_RXSTATUS_FAIL,
	HDCP2_A1_WAIT_H_PRIME_FAIL,
	HDCP2_A1_AKE_SEND_H_FAIL,
	HDCP2_A1_COMPARE_H_HPRIME_FAIL,
	HDCP2_A1_WAIT_PAIRING_INFO_FAIL,
	HDCP2_A2_SEND_LC_INIT_FAIL,
	HDCP2_A2_RECEIVE_L_PRIME_FAIL,
	HDCP2_A2_VERIFY_L_PRIME_FAIL,
	HDCP2_A3_COMPUTE_EDKEY_KS_FAIL,
	HDCP2_A3_SEND_EDKEY_KS_FAIL,
	HDCP2_A5_AUTHENTICATED_FAIL,
	HDCP2_A6_READ_RXSTATUS_FAIL,
	HDCP2_A6_WAIT_FOR_RCVID_FAIL,
	HDCP2_A6_CHECK_DOWNSTREAM_FAIL,
	HDCP2_A7_VERIFY_VPRIME_FAIL,
	HDCP2_A7_CHECK_REVOKED_FAIL,
	HDCP2_A8_SEND_RCVID_LIST_ACK_FAIL,
};


static const char * const hdcp2_fail_reason[] = {
	[HDCP2_A0_READ_RXCAPS_FAIL]			= "A0_READ_RXCAPS_FAIL",
	[HDCP2_A0_HDCP_NOT_CAPABLE]			= "A0_HDCP_NOT_CAPABLE",
	[HDCP2_A1_SEND_AKE_INIT_FAIL]		= "A1_SEND_AKE_INIT_FAIL",
	[HDCP2_A1_READ_AKE_SEND_CERT_FAIL]	= "A1_READ_AKE_SEND_CERT_FAIL",
	[HDCP2_A1_VERIFY_CERTRX_FAIL]		= "A1_VERIFY_CERTRX_FAIL",
	[HDCP2_A1_AKE_STORED_KM_FAIL]		= "A1_AKE_STORED_KM_FAIL",
	[HDCP2_A1_AKE_NO_STORED_KM_FAIL]	= "A1_AKE_NO_STORED_KM_FAIL",
	[HDCP2_A1_READ_RXSTATUS_FAIL]		= "A1_READ_RXSTATUS_FAIL",
	[HDCP2_A1_WAIT_H_PRIME_FAIL]		= "A1_WAIT_H_PRIME_FAIL",
	[HDCP2_A1_AKE_SEND_H_FAIL]			= "A1_AKE_SEND_H_FAIL",
	[HDCP2_A1_COMPARE_H_HPRIME_FAIL]	= "A1_COMPARE_H_HPRIME_FAIL",
	[HDCP2_A1_WAIT_PAIRING_INFO_FAIL]	= "A1_WAIT_PAIRING_INFO_FAIL",
	[HDCP2_A2_SEND_LC_INIT_FAIL]		= "A2_SEND_LC_INIT_FAIL",
	[HDCP2_A2_RECEIVE_L_PRIME_FAIL]		= "A2_RECEIVE_L_PRIME_FAIL",
	[HDCP2_A2_VERIFY_L_PRIME_FAIL]		= "A2_VERIFY_L_PRIME_FAIL",
	[HDCP2_A3_COMPUTE_EDKEY_KS_FAIL]	= "A3_COMPUTE_EDKEY_KS_FAIL",
	[HDCP2_A3_SEND_EDKEY_KS_FAIL]		= "A3_SEND_EDKEY_KS_FAIL",
	[HDCP2_A5_AUTHENTICATED_FAIL]		= "A5_AUTHENTICATED_FAIL",
	[HDCP2_A6_READ_RXSTATUS_FAIL]		= "A6_READ_RXSTATUS_FAIL",
	[HDCP2_A6_WAIT_FOR_RCVID_FAIL]		= "HDCP2_A6_WAIT_FOR_RCVID_FAIL",
	[HDCP2_A6_CHECK_DOWNSTREAM_FAIL]	= "HDCP2_A6_CHECK_DOWNSTREAM_FAIL",
	[HDCP2_A7_VERIFY_VPRIME_FAIL]		= "HDCP2_A7_VERIFY_VPRIME_FAIL",
	[HDCP2_A7_CHECK_REVOKED_FAIL]		= "HDCP2_A7_CHECK_REVOKED_FAIL",
	[HDCP2_A8_SEND_RCVID_LIST_ACK_FAIL]	= "HDCP2_A8_SEND_RCVID_LIST_ACK_FAIL",
};

struct sunxi_dptx_hdcp2_certrx {
	char rcvid[CERTRX_RCVID_SIZE]; //size: 5
//	char public_key[CERTRX_PUBLIC_KEY_SIZE]; //size: 131
	char public_key_n[CERTRX_PUBLIC_KEY_N_SIZE]; //size: 128
	char public_key_e[CERTRX_PUBLIC_KEY_E_SIZE]; //size: 3
	char reserved[CERTRX_RESERVED_SIZE]; // size: 2
	char signature[CERTRX_SIGNATURE_SIZE]; //size: 384
};

//struct sunxi_dptx_hdcp2_revoclist {
//	u8  rcvid[HDCP2X_TX_REVOCATION_LIST_MAX_DEVICES][HDCP_2_2_RECEIVER_ID_LEN];//[944][5]
//	u32 devices_num;
//};

struct sunxi_dptx_hdcp2_pairing_info {
	struct list_head list;
	char rcvid[CERTRX_RCVID_SIZE];
	char rxcaps[HDCP2X_RXCAPS_SIZE];
	char rtx[HDCP2X_RTX_SIZE];
	char rrx[HDCP2X_RRX_SIZE];
	char km[HDCP2X_EKH_KM_WR_SIZE];
	char m[HDCP2X_M_SIZE]; // m = rtx || rrx
	char ekh_km[HDCP2X_EKH_KM_WR_SIZE];
	bool ready;
};

//struct sunxi_dptx_hdcp2_stored_pairing_info {
//	struct sunxi_dptx_hdcp2_pairing_info pairing_info[SUNXI_DPTX_HDCP2_STORED_PAIRINGINFO_MAX],
//	u32 info_cnt;
//}

/* HDCP2X: muti-data are stored in big-endian in DPCD */
/* HDCP2X: DCP_LLC ir SRM data are stored in big-endian */
struct sunxi_dptx_hdcp2_info {
	enum sunxi_dp_hdcp_status status;
	enum sunxi_dp_hdcp2_state state;

	bool no_stored_km;

	/* status parse from HDCP2's DPCD rxcaps */
	u32 hdcp_version;
	bool rx_is_repeater;

	/* status parse from HDCP2's DPCD rxstatus */
	bool v_prime_ready;
	bool h_prime_available;
	bool pairing_available;
	bool rx_need_reauth_req;
	bool link_integrity_failure;

	/* info parse from HDCP2's DPCD rxinfo */
	bool device_exceeded;
	bool cascade_exceeded;
	bool downstream_is_hdcp2_repeater;
	bool downstream_is_hdcp1_dev;
	u32 repeater_dev_cnt;
	u32 repeater_dev_dep;


	/* info parse from HDCP2's DPCD certrx */
	struct sunxi_dptx_hdcp2_certrx certrx;

	struct sunxi_dptx_hdcp2_pairing_info *pairing_info;

	/* TODO: srm revocation list*/
	// struct sunxi_dptx_hdcp2_revoclist revoc_list;

	/* muti-bytes data are stored in big-endian format */
	/* 3 bytes(24-bit) data rxcaps */
	char rxcaps[HDCP2X_RXCAPS_SIZE];

	/* 8 bytes(64 bit) pseudo-random value */
	char rtx[HDCP2X_RTX_SIZE];

	/* 24 bit data, must be 0x02 0x00 0x00 */
	char txcaps[HDCP2X_TXCAPS_SIZE];

	/* 8 bytes data: rrx */
	char rrx[HDCP2X_RRX_SIZE];

	/* 16 bytes(128 bits) pseudo random data: km */
	char km[HDCP2X_TX_KM_SIZE];

	/* 128 byte data: encrypted km: ekpub_km*/
	char ekpub_km[HDCP2X_EKPUB_KM_SIZE];

	/* 32 bytes data: H' from DPCD:692c0 */
	char h_prime[HDCP2X_H_PRIME_SIZE];

	/* 32 bytes data: H, calculate by hmax_sha256 ourself */
	char h[HDCP2X_H_SIZE];

	/* 32 bytes data: kd, generate by ourself */
	char kd[HDCP2X_TX_DKEY_SIZE * HDCP2X_TX_AES128_SIZE];

	/* 8 bytes data: pseudo random data: rn */
	char rn[HDCP2X_RN_SIZE];

	/* 32 bytes data: L' from DPCD:692F8 */
	char l_prime[HDCP2X_L_PRIME_SIZE];

	/* 32 bytes data: L from DPCD:692F8 */
	char l[HDCP2X_L_PRIME_SIZE];

	/* 16 bytes data: ks */
	char ks[HDCP2X_KS_SIZE];

	/* 8 bytes data: ks */
	char riv[HDCP2X_RIV_SIZE];

	/* 16 bytes data: Edkey_ks */
	char edkey_ks[HDCP2X_EDKEY_KS_SIZE];

	/* 2 bytes data: RXINFO */
	char rxinfo[HDCP2X_RXINFO_SIZE];

	/* 3 bytes data: seq_num_v */
	char seq_num_v[HDCP2X_SEQ_NUM_V_SIZE];

	/* 16 bytes data: v_prime */
	char v_prime[HDCP2X_V_PRIME_SIZE];

	/* 155 bytes data: rcvid_list */
	char rcvid_list[HDCP2X_RCVID_LIST_SIZE];

	/* 16 bytes data: v */
	char v[HDCP2X_V_SIZE];
};


struct sunxi_dp_hdcp {
	wait_queue_head_t auth_queue;
	struct workqueue_struct *cpirq_workqueue;
	struct work_struct cpirq_work;
	struct mutex auth_lock;
	struct sunxi_edp_hw_desc *edp_hw;

	bool hdcp1_capable;
	bool hdcp2_capable;

	struct sunxi_dptx_hdcp1_info hdcp1_info;
	struct sunxi_dptx_hdcp2_info hdcp2_info;

	/* stored auth device info, for HDCP2X */
//	struct sunxi_dptx_hdcp2_stored_pairing_info stored_pairing_info;
	struct list_head stored_pairing_head;
};

#if IS_ENABLED(CONFIG_AW_DRM_DP_HDCP)
s32 sunxi_dp_hdcp_init(struct sunxi_dp_hdcp *hdcp, struct sunxi_edp_hw_desc *edp_hw);
s32 sunxi_dp_hdcp1_enable(struct sunxi_dp_hdcp *hdcp);
s32 sunxi_dp_hdcp2_enable(struct sunxi_dp_hdcp *hdcp);
s32 sunxi_dp_hdcp1_disable(struct sunxi_dp_hdcp *hdcp);
s32 sunxi_dp_hdcp2_disable(struct sunxi_dp_hdcp *hdcp);
bool dprx_hdcp1_capable(struct sunxi_dp_hdcp *hdcp);
bool dprx_hdcp2_capable(struct sunxi_dp_hdcp *hdcp);
#else
static inline s32 sunxi_dp_hdcp_init(struct sunxi_dp_hdcp *hdcp, struct sunxi_edp_hw_desc *edp_hw)
{
	return 0;
}

static inline s32 sunxi_dp_hdcp1_enable(struct sunxi_dp_hdcp *hdcp)
{
	return 0;
}

static inline s32 sunxi_dp_hdcp2_enable(struct sunxi_dp_hdcp *hdcp)
{
	return 0;
}

static inline s32 sunxi_dp_hdcp1_disable(struct sunxi_dp_hdcp *hdcp)
{
	return 0;
}

static inline s32 sunxi_dp_hdcp2_disable(struct sunxi_dp_hdcp *hdcp)
{
	return 0;
}

static inline bool dprx_hdcp1_capable(struct sunxi_dp_hdcp *hdcp)
{
	return false;
}

static inline bool dprx_hdcp2_capable(struct sunxi_dp_hdcp *hdcp)
{
	return false;
}
#endif

#endif /*End of file*/
