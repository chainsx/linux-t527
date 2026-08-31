/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Xilinx Specific HDCP2X driver
 *
 * Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * Author: Lakshmi Prasanna Eachuri <lakshmi.prasanna.eachuri@amd.com>
 */

#ifndef _XLNX_HDCP2X_TX_H_
#define _XLNX_HDCP2X_TX_H_

#include <linux/bits.h>
#include <linux/device.h>
#include "sunxi_dptx_hdcp.h"

int sunxi_hdcp2x_tx_verify_certificate(const struct sunxi_dptx_hdcp2_certrx *rx_certificate,
				      const u8 *dcp_cert_nvalue, int dcp_cert_nsize,
				      const u8 *dcp_cert_evalue, int dcp_cert_esize);
int sunxi_hdcp2x_tx_encryptedkm(const struct sunxi_dptx_hdcp2_certrx *rx_certificate,
			       const u8 *km_ptr, u8 *masking_seed, u8 *encrypted_km);
void sunxi_hdcp2x_tx_compute_edkey_ks(const u8 *rn, const u8 *km, const u8 *ks, const u8 *r_rx,
				     const u8 *r_tx, u8 *encrypted_ks);
void sunxi_hdcp2x_tx_compute_lprime(const u8 *rn, const u8 *km, const u8 *r_rx, const u8 *r_tx,
				   u8 *lprime);
void sunxi_hdcp2x_tx_compute_v(const u8 *rn, const u8 *r_rx, const u8 *rx_info,
			      const u8 *r_tx, const u8 *rcvid_list, const u8 rcvid_count,
			      const u8 *seq_num_v, const u8 *km, u8 *hash_v);
void sunxi_hdcp2x_tx_compute_m(const u8 *rn, const u8 *r_rx, const u8 *r_tx,
			      const u8 *stream_id_type, const u8 *k,
			      const u8 *seq_num_m, const u8 *km, u8 *m_hash);
void sunxi_hdcp2x_tx_compute_hprime(const u8 *r_rx, const u8 *rxcaps,
				   const u8 *r_tx, const u8 *txcaps,
				   const u8 *km, u8 *hprime);

#endif
