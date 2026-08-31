/**
******************************************************************************
*
* @file rwnx_mod_params.c
*
* @brief Set configuration according to modules parameters
*
* Copyright (C) RivieraWaves 2012-2021
*
******************************************************************************
*/
#include <linux/module.h>
#include <linux/rtnetlink.h>

#include "rwnx_defs.h"
#include "rwnx_tx.h"
#include "hal_desc.h"
#include "rwnx_cfgfile.h"
#include "rwnx_dini.h"
#include "reg_access.h"
#include "rwnx_compat.h"

#ifdef CONFIG_RWNX_FULLMAC
#define COMMON_PARAM(name, default_softmac, default_fullmac)    \
	.name = default_fullmac,
#define SOFTMAC_PARAM(name, default)
#define FULLMAC_PARAM(name, default) .name = default,
#endif /* CONFIG_RWNX_FULLMAC */

extern int reg_regdb_size;

struct rwnx_mod_params rwnx_mod_params = {
	/* common parameters */
	COMMON_PARAM(ht_on, true, true)
	COMMON_PARAM(vht_on, true, true)
	COMMON_PARAM(he_on, true, true)
	COMMON_PARAM(mcs_map, IEEE80211_VHT_MCS_SUPPORT_0_9, IEEE80211_VHT_MCS_SUPPORT_0_9)
	COMMON_PARAM(he_mcs_map, IEEE80211_HE_MCS_SUPPORT_0_11, IEEE80211_HE_MCS_SUPPORT_0_11)
	COMMON_PARAM(he_ul_on, false, false)
	COMMON_PARAM(ldpc_on, true, true)
	COMMON_PARAM(stbc_on, true, true)
	COMMON_PARAM(gf_rx_on, false, false)
	COMMON_PARAM(phy_cfg, 2, 2)
	COMMON_PARAM(uapsd_timeout, 300, 300)
	COMMON_PARAM(ap_uapsd_on, true, true)
	COMMON_PARAM(sgi, true, true)
	COMMON_PARAM(sgi80, true, true)
	COMMON_PARAM(use_2040, 1, 1)
	COMMON_PARAM(nss, 2, 2)
	COMMON_PARAM(amsdu_rx_max, 1, 1)
	COMMON_PARAM(bfmee, true, true)
	COMMON_PARAM(bfmer, false, false)
	COMMON_PARAM(mesh, true, true)
	COMMON_PARAM(murx, true, true)
	COMMON_PARAM(mutx, true, true)
	COMMON_PARAM(mutx_on, true, true)
	COMMON_PARAM(use_80, true, true)
	/* false: use crda(iw reg set CN); true: drive self-management(wifi_test wlan0 country_set CN) */
	COMMON_PARAM(custregd, true, true)
	COMMON_PARAM(custchan, false, false)
	COMMON_PARAM(roc_dur_max, 500, 500)
	COMMON_PARAM(listen_itv, 0, 0)
	COMMON_PARAM(listen_bcmc, true, true)
	COMMON_PARAM(lp_clk_ppm, 20, 20)
	COMMON_PARAM(ps_on, true, true)
	COMMON_PARAM(tx_lft, RWNX_TX_LIFETIME_MS, RWNX_TX_LIFETIME_MS)
	COMMON_PARAM(amsdu_maxnb, 4, 4)//NX_TX_PAYLOAD_MAX, NX_TX_PAYLOAD_MAX)
	// By default, only enable UAPSD for Voice queue (see IEEE80211_DEFAULT_UAPSD_QUEUE comment)
	COMMON_PARAM(uapsd_queues, IEEE80211_WMM_IE_STA_QOSINFO_AC_VO, IEEE80211_WMM_IE_STA_QOSINFO_AC_VO)
	COMMON_PARAM(tdls, false, false)
	COMMON_PARAM(uf, false, false)
	COMMON_PARAM(ftl, "", "")
	COMMON_PARAM(dpsm, false, false)
	COMMON_PARAM(tx_to_bk, 0, 0)
	COMMON_PARAM(tx_to_be, 0, 0)
	COMMON_PARAM(tx_to_vi, 0, 0)
	COMMON_PARAM(tx_to_vo, 0, 0)
	COMMON_PARAM(amsdu_tx, 0, 0)

	/* SOFTMAC only parameters */
	SOFTMAC_PARAM(mfp_on, false)
	SOFTMAC_PARAM(gf_on, false)
	SOFTMAC_PARAM(bwsig_on, true)
	SOFTMAC_PARAM(dynbw_on, true)
	SOFTMAC_PARAM(agg_tx, true)
	SOFTMAC_PARAM(amsdu_force, 2)
	SOFTMAC_PARAM(rc_probes_on, false)
	SOFTMAC_PARAM(cmon, true)
	SOFTMAC_PARAM(hwscan, true)
	SOFTMAC_PARAM(autobcn, true)
	SOFTMAC_PARAM(dpsm, true)

	/* FULLMAC only parameters */
	FULLMAC_PARAM(ant_div, true)
};

#ifdef CONFIG_RWNX_FULLMAC
/* FULLMAC specific parameters*/
module_param_named(ant_div, rwnx_mod_params.ant_div, bool, S_IRUGO);
MODULE_PARM_DESC(ant_div, "Enable Antenna Diversity (Default: 1)");
#endif /* CONFIG_RWNX_FULLMAC */

module_param_named(amsdu_tx, rwnx_mod_params.amsdu_tx, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(amsdu_tx, "Use A-MSDU in TX: 0-if advertised, 1-yes, 2-no (Default: 0)");

module_param_named(ht_on, rwnx_mod_params.ht_on, bool, S_IRUGO);
MODULE_PARM_DESC(ht_on, "Enable HT (Default: 1)");

module_param_named(vht_on, rwnx_mod_params.vht_on, bool, S_IRUGO);
MODULE_PARM_DESC(vht_on, "Enable VHT (Default: 1)");

module_param_named(he_on, rwnx_mod_params.he_on, bool, S_IRUGO);
MODULE_PARM_DESC(he_on, "Enable HE (Default: 1)");

module_param_named(mcs_map, rwnx_mod_params.mcs_map, int, S_IRUGO);
MODULE_PARM_DESC(mcs_map,  "VHT MCS map value  0: MCS0_7, 1: MCS0_8, 2: MCS0_9"
				 " (Default: 2)");

module_param_named(he_mcs_map, rwnx_mod_params.he_mcs_map, int, S_IRUGO);
MODULE_PARM_DESC(he_mcs_map,  "HE MCS map value  0: MCS0_7, 1: MCS0_9, 2: MCS0_11"
				 " (Default: 2)");

module_param_named(he_ul_on, rwnx_mod_params.he_ul_on, bool, S_IRUGO);
MODULE_PARM_DESC(he_ul_on, "Enable HE OFDMA UL (Default: 0)");

module_param_named(amsdu_maxnb, rwnx_mod_params.amsdu_maxnb, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(amsdu_maxnb, "Maximum number of MSDUs inside an A-MSDU in TX: (Default: NX_TX_PAYLOAD_MAX)");

module_param_named(ps_on, rwnx_mod_params.ps_on, bool, S_IRUGO);
MODULE_PARM_DESC(ps_on, "Enable PowerSaving (Default: 1-Enabled)");

module_param_named(tx_lft, rwnx_mod_params.tx_lft, int, 0644);
MODULE_PARM_DESC(tx_lft, "Tx lifetime (ms) - setting it to 0 disables retries "
				 "(Default: "__stringify(RWNX_TX_LIFETIME_MS)")");

module_param_named(ldpc_on, rwnx_mod_params.ldpc_on, bool, S_IRUGO);
MODULE_PARM_DESC(ldpc_on, "Enable LDPC (Default: 1)");

module_param_named(stbc_on, rwnx_mod_params.stbc_on, bool, S_IRUGO);
MODULE_PARM_DESC(stbc_on, "Enable STBC in RX (Default: 1)");

module_param_named(gf_rx_on, rwnx_mod_params.gf_rx_on, bool, S_IRUGO);
MODULE_PARM_DESC(gf_rx_on, "Enable HT greenfield in reception (Default: 1)");

module_param_named(phycfg, rwnx_mod_params.phy_cfg, int, S_IRUGO);
MODULE_PARM_DESC(phycfg,
				 "0 <= phycfg <= 5 : RF Channel Conf (Default: 2(C0-A1-B2))");

module_param_named(uapsd_timeout, rwnx_mod_params.uapsd_timeout, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(uapsd_timeout,
				 "UAPSD Timer timeout, in ms (Default: 300). If 0, UAPSD is disabled");

module_param_named(uapsd_queues, rwnx_mod_params.uapsd_queues, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(uapsd_queues, "UAPSD Queues, integer value, must be seen as a bitfield\n"
				 "        Bit 0 = VO\n"
				 "        Bit 1 = VI\n"
				 "        Bit 2 = BK\n"
				 "        Bit 3 = BE\n"
				 "     -> uapsd_queues=7 will enable uapsd for VO, VI and BK queues");

module_param_named(ap_uapsd_on, rwnx_mod_params.ap_uapsd_on, bool, S_IRUGO);
MODULE_PARM_DESC(ap_uapsd_on, "Enable UAPSD in AP mode (Default: 1)");

module_param_named(sgi, rwnx_mod_params.sgi, bool, S_IRUGO);
MODULE_PARM_DESC(sgi, "Advertise Short Guard Interval support (Default: 1)");

module_param_named(sgi80, rwnx_mod_params.sgi80, bool, S_IRUGO);
MODULE_PARM_DESC(sgi80, "Advertise Short Guard Interval support for 80MHz (Default: 1)");

module_param_named(use_2040, rwnx_mod_params.use_2040, bool, S_IRUGO);
MODULE_PARM_DESC(use_2040, "Use tweaked 20-40MHz mode (Default: 1)");

module_param_named(use_80, rwnx_mod_params.use_80, bool, S_IRUGO);
MODULE_PARM_DESC(use_80, "Enable 80MHz (Default: 1)");

module_param_named(custregd, rwnx_mod_params.custregd, bool, S_IRUGO);
MODULE_PARM_DESC(custregd,
				 "Use permissive custom regulatory rules (for testing ONLY) (Default: 0)");

module_param_named(custchan, rwnx_mod_params.custchan, bool, S_IRUGO);
MODULE_PARM_DESC(custchan,
				 "Extend channel set to non-standard channels (for testing ONLY) (Default: 0)");

module_param_named(nss, rwnx_mod_params.nss, int, S_IRUGO);
MODULE_PARM_DESC(nss, "1 <= nss <= 2 : Supported number of Spatial Streams (Default: 1)");

module_param_named(amsdu_rx_max, rwnx_mod_params.amsdu_rx_max, int, S_IRUGO);
MODULE_PARM_DESC(amsdu_rx_max, "0 <= amsdu_rx_max <= 2 : Maximum A-MSDU size supported in RX\n"
				 "        0: 3895 bytes\n"
				 "        1: 7991 bytes\n"
				 "        2: 11454 bytes\n"
				 "        This value might be reduced according to the FW capabilities.\n"
				 "        Default: 2");

module_param_named(bfmee, rwnx_mod_params.bfmee, bool, S_IRUGO);
MODULE_PARM_DESC(bfmee, "Enable Beamformee Capability (Default: 1-Enabled)");

module_param_named(bfmer, rwnx_mod_params.bfmer, bool, S_IRUGO);
MODULE_PARM_DESC(bfmer, "Enable Beamformer Capability (Default: 0-Disabled)");

module_param_named(mesh, rwnx_mod_params.mesh, bool, S_IRUGO);
MODULE_PARM_DESC(mesh, "Enable Meshing Capability (Default: 1-Enabled)");

module_param_named(murx, rwnx_mod_params.murx, bool, S_IRUGO);
MODULE_PARM_DESC(murx, "Enable MU-MIMO RX Capability (Default: 1-Enabled)");

module_param_named(mutx, rwnx_mod_params.mutx, bool, S_IRUGO);
MODULE_PARM_DESC(mutx, "Enable MU-MIMO TX Capability (Default: 1-Enabled)");

module_param_named(mutx_on, rwnx_mod_params.mutx_on, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(mutx_on, "Enable MU-MIMO transmissions (Default: 1-Enabled)");

module_param_named(roc_dur_max, rwnx_mod_params.roc_dur_max, int, S_IRUGO);
MODULE_PARM_DESC(roc_dur_max, "Maximum Remain on Channel duration");

module_param_named(listen_itv, rwnx_mod_params.listen_itv, int, S_IRUGO);
MODULE_PARM_DESC(listen_itv, "Maximum listen interval");

module_param_named(listen_bcmc, rwnx_mod_params.listen_bcmc, bool, S_IRUGO);
MODULE_PARM_DESC(listen_bcmc, "Wait for BC/MC traffic following DTIM beacon");

module_param_named(lp_clk_ppm, rwnx_mod_params.lp_clk_ppm, int, S_IRUGO);
MODULE_PARM_DESC(lp_clk_ppm, "Low Power Clock accuracy of the local device");

module_param_named(tdls, rwnx_mod_params.tdls, bool, S_IRUGO);
MODULE_PARM_DESC(tdls, "Enable TDLS (Default: 1-Enabled)");

module_param_named(uf, rwnx_mod_params.uf, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(uf, "Enable Unsupported HT Frame Logging (Default: 0-Disabled)");

module_param_named(ftl, rwnx_mod_params.ftl, charp, S_IRUGO);
MODULE_PARM_DESC(ftl, "Firmware trace level  (Default: \"\")");

module_param_named(dpsm, rwnx_mod_params.dpsm, bool, S_IRUGO);
MODULE_PARM_DESC(dpsm, "Enable Dynamic PowerSaving (Default: 1-Enabled)");

module_param_named(tx_to_bk, rwnx_mod_params.tx_to_bk, int, S_IRUGO);
MODULE_PARM_DESC(tx_to_bk,
	 "TX timeout for BK, in ms (Default: 0, Max: 65535). If 0, default value is applied");

module_param_named(tx_to_be, rwnx_mod_params.tx_to_be, int, S_IRUGO);
MODULE_PARM_DESC(tx_to_be,
	 "TX timeout for BE, in ms (Default: 0, Max: 65535). If 0, default value is applied");

module_param_named(tx_to_vi, rwnx_mod_params.tx_to_vi, int, S_IRUGO);
MODULE_PARM_DESC(tx_to_vi,
	 "TX timeout for VI, in ms (Default: 0, Max: 65535). If 0, default value is applied");

module_param_named(tx_to_vo, rwnx_mod_params.tx_to_vo, int, S_IRUGO);
MODULE_PARM_DESC(tx_to_vo,
	 "TX timeout for VO, in ms (Default: 0, Max: 65535). If 0, default value is applied");

#ifdef DEFAULT_COUNTRY_CODE
char default_ccode[4] = DEFAULT_COUNTRY_CODE;
#else
char default_ccode[4] = "00";
#endif

char country_code[4];
module_param_string(country_code, country_code, 4, 0600);

static const int mcs_map_to_rate[4][3] = {
	[PHY_CHNL_BW_20][IEEE80211_VHT_MCS_SUPPORT_0_7] = 65,
	[PHY_CHNL_BW_20][IEEE80211_VHT_MCS_SUPPORT_0_8] = 78,
	[PHY_CHNL_BW_20][IEEE80211_VHT_MCS_SUPPORT_0_9] = 78,
	[PHY_CHNL_BW_40][IEEE80211_VHT_MCS_SUPPORT_0_7] = 135,
	[PHY_CHNL_BW_40][IEEE80211_VHT_MCS_SUPPORT_0_8] = 162,
	[PHY_CHNL_BW_40][IEEE80211_VHT_MCS_SUPPORT_0_9] = 180,
	[PHY_CHNL_BW_80][IEEE80211_VHT_MCS_SUPPORT_0_7] = 292,
	[PHY_CHNL_BW_80][IEEE80211_VHT_MCS_SUPPORT_0_8] = 351,
	[PHY_CHNL_BW_80][IEEE80211_VHT_MCS_SUPPORT_0_9] = 390,
	[PHY_CHNL_BW_160][IEEE80211_VHT_MCS_SUPPORT_0_7] = 585,
	[PHY_CHNL_BW_160][IEEE80211_VHT_MCS_SUPPORT_0_8] = 702,
	[PHY_CHNL_BW_160][IEEE80211_VHT_MCS_SUPPORT_0_9] = 780,
};

#define MAX_VHT_RATE(map, nss, bw) (mcs_map_to_rate[bw][map] * (nss))

extern struct ieee80211_regdomain *reg_regdb[];

char ccode_channels[200];
int index_for_channel_list = 0;
module_param_string(ccode_channels, ccode_channels, 200, 0600);

void rwnx_get_countrycode_channels(struct wiphy *wiphy,
		struct ieee80211_regdomain *regdomain)
{
	enum nl80211_band band;
	struct ieee80211_supported_band *sband;
	int channel_index;
	int rule_index;
	int band_num = 0;
	int rule_num = regdomain->n_reg_rules;
	int start_freq = 0;
	int end_freq = 0;
	int center_freq = 0;
	char channel[4];

	band_num = NUM_NL80211_BANDS;

	memset(ccode_channels, 0, 200);
	index_for_channel_list = 0;

	for (band = 0; band < band_num; band++) {
		sband = wiphy->bands[band];// bands: 0:2.4G 1:5G 2:60G
		if (!sband)
			continue;

		for (channel_index = 0; channel_index < sband->n_channels; channel_index++) {
			for (rule_index = 0; rule_index < rule_num; rule_index++) {
				start_freq = regdomain->reg_rules[rule_index].freq_range.start_freq_khz/1000;
				end_freq = regdomain->reg_rules[rule_index].freq_range.end_freq_khz/1000;
				center_freq = sband->channels[channel_index].center_freq;
				if ((center_freq - 10) >= start_freq && (center_freq + 10) <= end_freq) {
					sprintf(channel, "%d", ieee80211_frequency_to_channel(center_freq));
					memcpy(ccode_channels + index_for_channel_list, channel, strlen(channel));
					index_for_channel_list += strlen(channel);
					memcpy(ccode_channels + index_for_channel_list, " ", 1);
					index_for_channel_list += 1;
					break;
				}
			}
		}
	}
	AICWFDBG(LOGINFO, "%s support channel:%s\r\n", __func__, ccode_channels);
}


struct ieee80211_regdomain *getRegdomainFromRwnxDBIndex(struct wiphy *wiphy,
	int index)
{
	u8 idx;

	idx = index;

	memset(country_code, 0, 4);
	country_code[0] = reg_regdb[idx]->alpha2[0];
	country_code[1] = reg_regdb[idx]->alpha2[1];

	AICWFDBG(LOGINFO, "%s set ccode:%s\r\n", __func__, country_code);

	rwnx_get_countrycode_channels(wiphy, reg_regdb[idx]);

	return reg_regdb[idx];
}


struct ieee80211_regdomain *getRegdomainFromRwnxDB(struct wiphy *wiphy,
	char *alpha2)
{
	u8 idx;

	memset(country_code, 0, 4);

	AICWFDBG(LOGINFO, "%s set ccode:%s\r\n", __func__, alpha2);
	idx = 0;

	while (reg_regdb[idx] && idx < reg_regdb_size) {
		if ((reg_regdb[idx]->alpha2[0] == alpha2[0]) &&
			(reg_regdb[idx]->alpha2[1] == alpha2[1])) {
			memcpy(country_code, alpha2, 2);
			rwnx_get_countrycode_channels(wiphy, reg_regdb[idx]);
			return reg_regdb[idx];
		}
		idx++;
	}

	AICWFDBG(LOGERROR, "%s(): Error, wrong country = %s\n",
			__func__, alpha2);
	AICWFDBG(LOGINFO, "Set as default 00\n");
	memcpy(country_code, default_ccode, sizeof(default_ccode));
	rwnx_get_countrycode_channels(wiphy, reg_regdb[0]);

	return reg_regdb[0];
}

static void rwnx_set_vht_capa(struct rwnx_hw *rwnx_hw, struct wiphy *wiphy)
{
#ifdef CONFIG_VHT_FOR_OLD_KERNEL
	#ifdef USE_5G
	struct ieee80211_supported_band *band_5GHz = wiphy->bands[NL80211_BAND_5GHZ];
	#endif
	struct ieee80211_supported_band *band_2GHz = wiphy->bands[NL80211_BAND_2GHZ];

	int i;
	int nss = rwnx_hw->mod_params->nss;
	int mcs_map;
	int mcs_map_max;
	int bw_max;

	if (!rwnx_hw->mod_params->vht_on) {
		return;
	}

	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80)
		nss = 1;

	rwnx_hw->vht_cap_2G.vht_supported = true;
		if (rwnx_hw->mod_params->sgi80)
			rwnx_hw->vht_cap_2G.cap |= IEEE80211_VHT_CAP_SHORT_GI_80;
		if (rwnx_hw->mod_params->stbc_on)
			rwnx_hw->vht_cap_2G.cap |= IEEE80211_VHT_CAP_RXSTBC_1;
		if (rwnx_hw->mod_params->ldpc_on)
			rwnx_hw->vht_cap_2G.cap |= IEEE80211_VHT_CAP_RXLDPC;
		if (rwnx_hw->mod_params->bfmee) {
			rwnx_hw->vht_cap_2G.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
			rwnx_hw->vht_cap_2G.cap |= 3 << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT;
		#else
			rwnx_hw->vht_cap_2G.cap |= 3 << 13;
		#endif
		}
		if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2 && rwnx_hw->mod_params->stbc_on == true)
			rwnx_hw->vht_cap_2G.cap |= IEEE80211_VHT_CAP_TXSTBC;

		// Update the AMSDU max RX size (not shifted as located at offset 0 of the VHT cap)
		rwnx_hw->vht_cap_2G.cap |= rwnx_hw->mod_params->amsdu_rx_max;

		if (rwnx_hw->mod_params->bfmer) {
			rwnx_hw->vht_cap_2G.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
			/* Set number of sounding dimensions */
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
			rwnx_hw->vht_cap_2G.cap |= (nss - 1) << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT;
		#else
			rwnx_hw->vht_cap_2G.cap |= (nss - 1) << 16;
		#endif
		}
		if (rwnx_hw->mod_params->murx)
			rwnx_hw->vht_cap_2G.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;
		if (rwnx_hw->mod_params->mutx)
			rwnx_hw->vht_cap_2G.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE;

		/*
		 * MCS map:
		 * This capabilities are filled according to the mcs_map module parameter.
		 * However currently we have some limitations due to FPGA clock constraints
		 * that prevent always using the range of MCS that is defined by the
		 * parameter:
		 *	 - in RX, 2SS, we support up to MCS7
		 *	 - in TX, 2SS, we support up to MCS8
		 */
		// Get max supported BW
		if (rwnx_hw->mod_params->use_80)
			bw_max = PHY_CHNL_BW_80;
		else if (rwnx_hw->mod_params->use_2040)
			bw_max = PHY_CHNL_BW_40;
		else
			bw_max = PHY_CHNL_BW_20;

		// Check if MCS map should be limited to MCS0_8 due to the standard. Indeed in BW20,
		// MCS9 is not supported in 1 and 2 SS
		if (rwnx_hw->mod_params->use_2040)
			mcs_map_max = IEEE80211_VHT_MCS_SUPPORT_0_9;
		else
			mcs_map_max = IEEE80211_VHT_MCS_SUPPORT_0_8;

		mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map, mcs_map_max);
		rwnx_hw->vht_cap_2G.vht_mcs.rx_mcs_map = cpu_to_le16(0);
		for (i = 0; i < nss; i++) {
			rwnx_hw->vht_cap_2G.vht_mcs.rx_mcs_map |= cpu_to_le16(mcs_map << (i*2));
			rwnx_hw->vht_cap_2G.vht_mcs.rx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
			//mcs_map = IEEE80211_VHT_MCS_SUPPORT_0_7;
			//printk("lemon map=%x\n", rwnx_hw->vht_cap_2G.vht_mcs.rx_mcs_map);
		}
		for (; i < 8; i++) {
			rwnx_hw->vht_cap_2G.vht_mcs.rx_mcs_map |= cpu_to_le16(
				IEEE80211_VHT_MCS_NOT_SUPPORTED << (i*2));
		}

		mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map, mcs_map_max);
		rwnx_hw->vht_cap_2G.vht_mcs.tx_mcs_map = cpu_to_le16(0);
		for (i = 0; i < nss; i++) {
			rwnx_hw->vht_cap_2G.vht_mcs.tx_mcs_map |= cpu_to_le16(mcs_map << (i*2));
			rwnx_hw->vht_cap_2G.vht_mcs.tx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
			//mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map,
			//				IEEE80211_VHT_MCS_SUPPORT_0_8);
		}
		for (; i < 8; i++) {
			rwnx_hw->vht_cap_2G.vht_mcs.tx_mcs_map |= cpu_to_le16(
				IEEE80211_VHT_MCS_NOT_SUPPORTED << (i*2));
		}

		if (!rwnx_hw->mod_params->use_80) {
#ifdef CONFIG_VENDOR_RWNX_VHT_NO80
			rwnx_hw->vht_cap_2G.cap |= IEEE80211_VHT_CAP_NOT_SUP_WIDTH_80;
#endif//CONFIG_VENDOR_RWNX_VHT_NO80
			rwnx_hw->vht_cap_2G.cap &= ~IEEE80211_VHT_CAP_SHORT_GI_80;
		}

			rwnx_hw->vht_cap_2G.cap |= IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK;
		AICWFDBG(LOGINFO, "%s, vht_capa_info=0x%x\n", __func__, rwnx_hw->vht_cap_2G.cap);
#ifdef USE_5G
	if (rwnx_hw->band_5g_support) {
		rwnx_hw->vht_cap_5G.vht_supported = true;
		if (rwnx_hw->mod_params->sgi80)
			rwnx_hw->vht_cap_5G.cap |= IEEE80211_VHT_CAP_SHORT_GI_80;
		if (rwnx_hw->mod_params->stbc_on)
			rwnx_hw->vht_cap_5G.cap |= IEEE80211_VHT_CAP_RXSTBC_1;
		if (rwnx_hw->mod_params->ldpc_on)
			rwnx_hw->vht_cap_5G.cap |= IEEE80211_VHT_CAP_RXLDPC;
		if (rwnx_hw->mod_params->bfmee) {
			rwnx_hw->vht_cap_5G.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
			#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
			rwnx_hw->vht_cap_5G.cap |= 3 << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT;
			#else
			rwnx_hw->vht_cap_5G.cap |= 3 << 13;
			#endif
		}
		if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2 && rwnx_hw->mod_params->stbc_on == true)
			rwnx_hw->vht_cap_5G.cap |= IEEE80211_VHT_CAP_TXSTBC;

		// Update the AMSDU max RX size (not shifted as located at offset 0 of the VHT cap)
		rwnx_hw->vht_cap_5G.cap |= rwnx_hw->mod_params->amsdu_rx_max;

		if (rwnx_hw->mod_params->bfmer) {
			rwnx_hw->vht_cap_5G.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
			/* Set number of sounding dimensions */
			#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
			rwnx_hw->vht_cap_5G.cap |= (nss - 1) << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT;
			#else
			rwnx_hw->vht_cap_5G.cap |= (nss - 1) << 16;
			#endif
		}
		if (rwnx_hw->mod_params->murx)
			rwnx_hw->vht_cap_5G.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;
		if (rwnx_hw->mod_params->mutx)
			rwnx_hw->vht_cap_5G.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE;

		/*
		 * MCS map:
		 * This capabilities are filled according to the mcs_map module parameter.
		 * However currently we have some limitations due to FPGA clock constraints
		 * that prevent always using the range of MCS that is defined by the
		 * parameter:
		 *   - in RX, 2SS, we support up to MCS7
		 *   - in TX, 2SS, we support up to MCS8
		 */
		// Get max supported BW
		if (rwnx_hw->mod_params->use_80)
			bw_max = PHY_CHNL_BW_80;
		else if (rwnx_hw->mod_params->use_2040)
			bw_max = PHY_CHNL_BW_40;
		else
			bw_max = PHY_CHNL_BW_20;

		// Check if MCS map should be limited to MCS0_8 due to the standard. Indeed in BW20,
		// MCS9 is not supported in 1 and 2 SS
		if (rwnx_hw->mod_params->use_2040)
			mcs_map_max = IEEE80211_VHT_MCS_SUPPORT_0_9;
		else
			mcs_map_max = IEEE80211_VHT_MCS_SUPPORT_0_8;

		mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map, mcs_map_max);
		rwnx_hw->vht_cap_5G.vht_mcs.rx_mcs_map = cpu_to_le16(0);
		for (i = 0; i < nss; i++) {
			rwnx_hw->vht_cap_5G.vht_mcs.rx_mcs_map |= cpu_to_le16(mcs_map << (i*2));
			rwnx_hw->vht_cap_5G.vht_mcs.rx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
			//mcs_map = IEEE80211_VHT_MCS_SUPPORT_0_7;
		}
		for (; i < 8; i++) {
			rwnx_hw->vht_cap_5G.vht_mcs.rx_mcs_map |= cpu_to_le16(
				IEEE80211_VHT_MCS_NOT_SUPPORTED << (i*2));
		}

		mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map, mcs_map_max);
		rwnx_hw->vht_cap_5G.vht_mcs.tx_mcs_map = cpu_to_le16(0);
		for (i = 0; i < nss; i++) {
			rwnx_hw->vht_cap_5G.vht_mcs.tx_mcs_map |= cpu_to_le16(mcs_map << (i*2));
			rwnx_hw->vht_cap_5G.vht_mcs.tx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
			//mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map,
			//                IEEE80211_VHT_MCS_SUPPORT_0_8);
		}
		for (; i < 8; i++) {
			rwnx_hw->vht_cap_5G.vht_mcs.tx_mcs_map |= cpu_to_le16(
				IEEE80211_VHT_MCS_NOT_SUPPORTED << (i*2));
		}

		if (!rwnx_hw->mod_params->use_80) {
#ifdef CONFIG_VENDOR_RWNX_VHT_NO80
			rwnx_hw->vht_cap_5G.cap |= IEEE80211_VHT_CAP_NOT_SUP_WIDTH_80;
#endif//CONFIG_VENDOR_RWNX_VHT_NO80
			rwnx_hw->vht_cap_5G.cap &= ~IEEE80211_VHT_CAP_SHORT_GI_80;
		}

		rwnx_hw->vht_cap_5G.cap |= IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK;

	}
#endif//USE_5G
	return;
#else
	struct ieee80211_supported_band *band_5GHz = wiphy->bands[NL80211_BAND_5GHZ];
	struct ieee80211_supported_band *band_2GHz = wiphy->bands[NL80211_BAND_2GHZ];

	int i;
	int nss = rwnx_hw->mod_params->nss;
	int mcs_map;
	int mcs_map_max;
	int bw_max;
#endif//CONFIG_VHT_FOR_OLD_KERNEL

	if (!rwnx_hw->mod_params->vht_on) {
		return;
	}

	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80)
		nss = 1;

	band_2GHz->vht_cap.vht_supported = true;
	if (rwnx_hw->mod_params->sgi80)
		band_2GHz->vht_cap.cap |= IEEE80211_VHT_CAP_SHORT_GI_80;
	if (rwnx_hw->mod_params->stbc_on)
		band_2GHz->vht_cap.cap |= IEEE80211_VHT_CAP_RXSTBC_1;
	if (rwnx_hw->mod_params->ldpc_on)
		band_2GHz->vht_cap.cap |= IEEE80211_VHT_CAP_RXLDPC;
	if (rwnx_hw->mod_params->bfmee) {
		band_2GHz->vht_cap.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
		band_2GHz->vht_cap.cap |= 3 << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT;
	#else
		band_2GHz->vht_cap.cap |= 3 << 13;
	#endif
	}
	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2 && rwnx_hw->mod_params->stbc_on == true)
		band_2GHz->vht_cap.cap |= IEEE80211_VHT_CAP_TXSTBC;

	// Update the AMSDU max RX size (not shifted as located at offset 0 of the VHT cap)
	band_2GHz->vht_cap.cap |= rwnx_hw->mod_params->amsdu_rx_max;

	if (rwnx_hw->mod_params->bfmer) {
		band_2GHz->vht_cap.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
		/* Set number of sounding dimensions */
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
		band_2GHz->vht_cap.cap |= (nss - 1) << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT;
	#else
		band_2GHz->vht_cap.cap |= (nss - 1) << 16;
	#endif
	}
	if (rwnx_hw->mod_params->murx)
		band_2GHz->vht_cap.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;
	if (rwnx_hw->mod_params->mutx)
		band_2GHz->vht_cap.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE;

	/*
	 * MCS map:
	 * This capabilities are filled according to the mcs_map module parameter.
	 * However currently we have some limitations due to FPGA clock constraints
	 * that prevent always using the range of MCS that is defined by the
	 * parameter:
	 *	 - in RX, 2SS, we support up to MCS7
	 *	 - in TX, 2SS, we support up to MCS8
	 */
	// Get max supported BW
	if (rwnx_hw->mod_params->use_80)
		bw_max = PHY_CHNL_BW_80;
	else if (rwnx_hw->mod_params->use_2040)
		bw_max = PHY_CHNL_BW_40;
	else
		bw_max = PHY_CHNL_BW_20;

	// Check if MCS map should be limited to MCS0_8 due to the standard. Indeed in BW20,
	// MCS9 is not supported in 1 and 2 SS
	if (rwnx_hw->mod_params->use_2040)
		mcs_map_max = IEEE80211_VHT_MCS_SUPPORT_0_9;
	else
		mcs_map_max = IEEE80211_VHT_MCS_SUPPORT_0_8;

	mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map, mcs_map_max);
	band_2GHz->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(0);
	for (i = 0; i < nss; i++) {
		band_2GHz->vht_cap.vht_mcs.rx_mcs_map |= cpu_to_le16(mcs_map << (i*2));
		band_2GHz->vht_cap.vht_mcs.rx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
		//mcs_map = IEEE80211_VHT_MCS_SUPPORT_0_7;
	}
	for (; i < 8; i++) {
		band_2GHz->vht_cap.vht_mcs.rx_mcs_map |= cpu_to_le16(
			IEEE80211_VHT_MCS_NOT_SUPPORTED << (i*2));
	}

	mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map, mcs_map_max);
	band_2GHz->vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(0);
	for (i = 0; i < nss; i++) {
		band_2GHz->vht_cap.vht_mcs.tx_mcs_map |= cpu_to_le16(mcs_map << (i*2));
		band_2GHz->vht_cap.vht_mcs.tx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
		//mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map,
		//				IEEE80211_VHT_MCS_SUPPORT_0_8);
	}
	for (; i < 8; i++) {
		band_2GHz->vht_cap.vht_mcs.tx_mcs_map |= cpu_to_le16(
			IEEE80211_VHT_MCS_NOT_SUPPORTED << (i*2));
	}

	if (!rwnx_hw->mod_params->use_80) {
#ifdef CONFIG_VENDOR_RWNX_VHT_NO80
		band_2GHz->vht_cap.cap |= IEEE80211_VHT_CAP_NOT_SUP_WIDTH_80;
#endif
		band_2GHz->vht_cap.cap &= ~IEEE80211_VHT_CAP_SHORT_GI_80;
	}

	if (rwnx_hw->band_5g_support) {
		band_5GHz->vht_cap.vht_supported = true;
		if (rwnx_hw->mod_params->sgi80)
			band_5GHz->vht_cap.cap |= IEEE80211_VHT_CAP_SHORT_GI_80;
		if (rwnx_hw->mod_params->stbc_on)
			band_5GHz->vht_cap.cap |= IEEE80211_VHT_CAP_RXSTBC_1;
		if (rwnx_hw->mod_params->ldpc_on)
			band_5GHz->vht_cap.cap |= IEEE80211_VHT_CAP_RXLDPC;
		if (rwnx_hw->mod_params->bfmee) {
			band_5GHz->vht_cap.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
			#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
			band_5GHz->vht_cap.cap |= 3 << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT;
			#else
			band_5GHz->vht_cap.cap |= 3 << 13;
			#endif
		}
		if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2 && rwnx_hw->mod_params->stbc_on == true)
			band_5GHz->vht_cap.cap |= IEEE80211_VHT_CAP_TXSTBC;

		// Update the AMSDU max RX size (not shifted as located at offset 0 of the VHT cap)
		band_5GHz->vht_cap.cap |= rwnx_hw->mod_params->amsdu_rx_max;

		if (rwnx_hw->mod_params->bfmer) {
			band_5GHz->vht_cap.cap |= IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
			/* Set number of sounding dimensions */
			#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
			band_5GHz->vht_cap.cap |= (nss - 1) << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT;
			#else
			band_5GHz->vht_cap.cap |= (nss - 1) << 16;
			#endif
		}
		if (rwnx_hw->mod_params->murx)
			band_5GHz->vht_cap.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;
		if (rwnx_hw->mod_params->mutx)
			band_5GHz->vht_cap.cap |= IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE;

		/*
		 * MCS map:
		 * This capabilities are filled according to the mcs_map module parameter.
		 * However currently we have some limitations due to FPGA clock constraints
		 * that prevent always using the range of MCS that is defined by the
		 * parameter:
		 *   - in RX, 2SS, we support up to MCS7
		 *   - in TX, 2SS, we support up to MCS8
		 */
		// Get max supported BW
		if (rwnx_hw->mod_params->use_80)
			bw_max = PHY_CHNL_BW_80;
		else if (rwnx_hw->mod_params->use_2040)
			bw_max = PHY_CHNL_BW_40;
		else
			bw_max = PHY_CHNL_BW_20;

		// Check if MCS map should be limited to MCS0_8 due to the standard. Indeed in BW20,
		// MCS9 is not supported in 1 and 2 SS
		if (rwnx_hw->mod_params->use_2040)
			mcs_map_max = IEEE80211_VHT_MCS_SUPPORT_0_9;
		else
			mcs_map_max = IEEE80211_VHT_MCS_SUPPORT_0_8;

		mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map, mcs_map_max);
		band_5GHz->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(0);
		for (i = 0; i < nss; i++) {
			band_5GHz->vht_cap.vht_mcs.rx_mcs_map |= cpu_to_le16(mcs_map << (i*2));
			band_5GHz->vht_cap.vht_mcs.rx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
			//mcs_map = IEEE80211_VHT_MCS_SUPPORT_0_7;
		}
		for (; i < 8; i++) {
			band_5GHz->vht_cap.vht_mcs.rx_mcs_map |= cpu_to_le16(
				IEEE80211_VHT_MCS_NOT_SUPPORTED << (i*2));
		}

		mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map, mcs_map_max);
		band_5GHz->vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(0);
		for (i = 0; i < nss; i++) {
			band_5GHz->vht_cap.vht_mcs.tx_mcs_map |= cpu_to_le16(mcs_map << (i*2));
			band_5GHz->vht_cap.vht_mcs.tx_highest = MAX_VHT_RATE(mcs_map, nss, bw_max);
			//mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map,
			//				IEEE80211_VHT_MCS_SUPPORT_0_8);
		}
		for (; i < 8; i++) {
			band_5GHz->vht_cap.vht_mcs.tx_mcs_map |= cpu_to_le16(
				IEEE80211_VHT_MCS_NOT_SUPPORTED << (i*2));
		}

		if (!rwnx_hw->mod_params->use_80) {
#ifdef CONFIG_VENDOR_RWNX_VHT_NO80
			band_5GHz->vht_cap.cap |= IEEE80211_VHT_CAP_NOT_SUP_WIDTH_80;
#endif
			band_5GHz->vht_cap.cap &= ~IEEE80211_VHT_CAP_SHORT_GI_80;
		}
	}
}

static void rwnx_set_ht_capa(struct rwnx_hw *rwnx_hw, struct wiphy *wiphy)
{
	struct ieee80211_supported_band *band_5GHz = wiphy->bands[NL80211_BAND_5GHZ];
	struct ieee80211_supported_band *band_2GHz = wiphy->bands[NL80211_BAND_2GHZ];
	int i;
	int nss = rwnx_hw->mod_params->nss;

	if (!rwnx_hw->mod_params->ht_on) {
		band_2GHz->ht_cap.ht_supported = false;
		if (rwnx_hw->band_5g_support)
			band_5GHz->ht_cap.ht_supported = false;
		return;
	}

	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80)
		nss = 1;

	if (rwnx_hw->mod_params->stbc_on)
		band_2GHz->ht_cap.cap |= 1 << IEEE80211_HT_CAP_RX_STBC_SHIFT;
	if (rwnx_hw->mod_params->ldpc_on)
		band_2GHz->ht_cap.cap |= IEEE80211_HT_CAP_LDPC_CODING;
	if (rwnx_hw->mod_params->use_2040) {
		band_2GHz->ht_cap.mcs.rx_mask[4] = 0x1; /* MCS32 */
		band_2GHz->ht_cap.cap |= IEEE80211_HT_CAP_SUP_WIDTH_20_40;
		band_2GHz->ht_cap.mcs.rx_highest = cpu_to_le16(135 * nss);
	} else {
		band_2GHz->ht_cap.mcs.rx_highest = cpu_to_le16(65 * nss);
	}
	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2 && rwnx_hw->mod_params->stbc_on == true)
		band_2GHz->ht_cap.cap |= IEEE80211_HT_CAP_TX_STBC;

	// Update the AMSDU max RX size
	if (rwnx_hw->mod_params->amsdu_rx_max)
		band_2GHz->ht_cap.cap |= IEEE80211_HT_CAP_MAX_AMSDU;

	if (rwnx_hw->mod_params->sgi) {
		band_2GHz->ht_cap.cap |= IEEE80211_HT_CAP_SGI_20;
		if (rwnx_hw->mod_params->use_2040) {
			band_2GHz->ht_cap.cap |= IEEE80211_HT_CAP_SGI_40;
			band_2GHz->ht_cap.mcs.rx_highest = cpu_to_le16(150 * nss);
		} else
			band_2GHz->ht_cap.mcs.rx_highest = cpu_to_le16(72 * nss);
	}
	if (rwnx_hw->mod_params->gf_rx_on)
		band_2GHz->ht_cap.cap |= IEEE80211_HT_CAP_GRN_FLD;

	for (i = 0; i < nss; i++) {
		band_2GHz->ht_cap.mcs.rx_mask[i] = 0xFF;
	}

	if (rwnx_hw->band_5g_support)
		band_5GHz->ht_cap = band_2GHz->ht_cap;
}

#ifdef CONFIG_HE_FOR_OLD_KERNEL
extern struct ieee80211_sband_iftype_data rwnx_he_capa;
#endif
static void rwnx_set_he_capa(struct rwnx_hw *rwnx_hw, struct wiphy *wiphy)
{
#ifdef CONFIG_HE_FOR_OLD_KERNEL
	struct ieee80211_sta_he_cap *he_cap;
	int i;
	int nss = rwnx_hw->mod_params->nss;
	int mcs_map;

	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80)
		nss  = 1;

	he_cap = (struct ieee80211_sta_he_cap *) &rwnx_he_capa.he_cap;
	he_cap->has_he = true;
	he_cap->he_cap_elem.mac_cap_info[2] |= IEEE80211_HE_MAC_CAP2_ALL_ACK;
	if (rwnx_hw->mod_params->use_2040) {
		he_cap->he_cap_elem.phy_cap_info[0] |=
						IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_IN_2G;
		he_cap->ppe_thres[0] |= 0x10;
	}
	if (rwnx_hw->mod_params->use_80) {
		he_cap->ppe_thres[0] |= 0x20;
		he_cap->ppe_thres[2] |= 0xc0;
		he_cap->ppe_thres[3] |= 0x01;
	}
	if (rwnx_hw->mod_params->nss == 2) {
		he_cap->ppe_thres[0] |= 0x01;
		he_cap->ppe_thres[3] |= 0x70;
		he_cap->ppe_thres[4] |= 0x1c;
		he_cap->ppe_thres[5] |= 0x07;
	}

	//if (rwnx_hw->mod_params->use_80)
	{
		he_cap->he_cap_elem.phy_cap_info[0] |=
						IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G;
	}
	if (rwnx_hw->mod_params->ldpc_on) {
		he_cap->he_cap_elem.phy_cap_info[1] |= IEEE80211_HE_PHY_CAP1_LDPC_CODING_IN_PAYLOAD;
	} else {
		// If no LDPC is supported, we have to limit to MCS0_9, as LDPC is mandatory
		// for MCS 10 and 11
		rwnx_hw->mod_params->he_mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map,
												IEEE80211_HE_MCS_SUPPORT_0_9);
	}
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
	he_cap->he_cap_elem.phy_cap_info[1] |= IEEE80211_HE_PHY_CAP1_HE_LTF_AND_GI_FOR_HE_PPDUS_0_8US
											| IEEE80211_HE_PHY_CAP1_MIDAMBLE_RX_TX_MAX_NSTS;

	he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_MIDAMBLE_RX_TX_MAX_NSTS |
										   IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US |
										   IEEE80211_HE_PHY_CAP2_DOPPLER_RX;
	#else
	he_cap->he_cap_elem.phy_cap_info[1] |= IEEE80211_HE_PHY_CAP1_HE_LTF_AND_GI_FOR_HE_PPDUS_0_8US;
	he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US |
										   IEEE80211_HE_PHY_CAP2_DOPPLER_RX;
	#endif
	if (rwnx_hw->mod_params->stbc_on)
		he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_STBC_RX_UNDER_80MHZ;
	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2 && rwnx_hw->mod_params->stbc_on == true) {
		he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_STBC_TX_UNDER_80MHZ;
	}
	he_cap->he_cap_elem.phy_cap_info[3] |=
											#if 0
											IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_16_QAM |
											IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_16_QAM |
											#else
											IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_BPSK |
											IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_BPSK |
											#endif
											IEEE80211_HE_PHY_CAP3_DCM_MAX_RX_NSS_1 |
											IEEE80211_HE_PHY_CAP3_DCM_MAX_TX_NSS_1 |
											IEEE80211_HE_PHY_CAP3_RX_PARTIAL_BW_SU_IN_20MHZ_MU
	if (rwnx_hw->mod_params->bfmee) {
		he_cap->he_cap_elem.phy_cap_info[4] |= IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE;
		he_cap->he_cap_elem.phy_cap_info[4] |=
					 IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_4;
	}
	he_cap->he_cap_elem.phy_cap_info[5] |= IEEE80211_HE_PHY_CAP5_NG16_SU_FEEDBACK |
										   IEEE80211_HE_PHY_CAP5_NG16_MU_FEEDBACK;
	he_cap->he_cap_elem.phy_cap_info[6] |= IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_42_SU |
										   IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_75_MU |
										   IEEE80211_HE_PHY_CAP6_TRIG_SU_BEAMFORMER_FB |
										   IEEE80211_HE_PHY_CAP6_TRIG_MU_BEAMFORMER_FB |
										   IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT |
										   IEEE80211_HE_PHY_CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO |
										   IEEE80211_HE_PHY_CAP6_PARTIAL_BW_EXT_RANGE;
	//he_cap->he_cap_elem.phy_cap_info[7] |= IEEE80211_HE_PHY_CAP7_HE_SU_MU_PPDU_4XLTF_AND_08_US_GI;
	he_cap->he_cap_elem.phy_cap_info[8] |= IEEE80211_HE_PHY_CAP8_20MHZ_IN_40MHZ_HE_PPDU_IN_2G;
	he_cap->he_cap_elem.phy_cap_info[9] |= IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_COMP_SIGB |
										   IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_NON_COMP_SIGB;

	mcs_map = rwnx_hw->mod_params->he_mcs_map;
	memset(&he_cap->he_mcs_nss_supp, 0, sizeof(he_cap->he_mcs_nss_supp));
	for (i = 0; i < nss; i++) {
		__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
		he_cap->he_mcs_nss_supp.rx_mcs_80 |= cpu_to_le16(mcs_map << (i*2));
		he_cap->he_mcs_nss_supp.rx_mcs_160 |= unsup_for_ss;
		he_cap->he_mcs_nss_supp.rx_mcs_80p80 |= unsup_for_ss;
		//mcs_map = IEEE80211_HE_MCS_SUPPORT_0_7;
		}
	for (; i < 8; i++) {
		__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
		he_cap->he_mcs_nss_supp.rx_mcs_80 |= unsup_for_ss;
		he_cap->he_mcs_nss_supp.rx_mcs_160 |= unsup_for_ss;
		he_cap->he_mcs_nss_supp.rx_mcs_80p80 |= unsup_for_ss;
		}
	mcs_map = rwnx_hw->mod_params->he_mcs_map;
	for (i = 0; i < nss; i++) {
		__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
		he_cap->he_mcs_nss_supp.tx_mcs_80 |= cpu_to_le16(mcs_map << (i*2));
		he_cap->he_mcs_nss_supp.tx_mcs_160 |= unsup_for_ss;
		he_cap->he_mcs_nss_supp.tx_mcs_80p80 |= unsup_for_ss;
		//mcs_map = min_t(int, rwnx_hw->mod_params->he_mcs_map,
		//				IEEE80211_HE_MCS_SUPPORT_0_7);
	}
	for (; i < 8; i++) {
		__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
		he_cap->he_mcs_nss_supp.tx_mcs_80 |= unsup_for_ss;
		he_cap->he_mcs_nss_supp.tx_mcs_160 |= unsup_for_ss;
		he_cap->he_mcs_nss_supp.tx_mcs_80p80 |= unsup_for_ss;
	}

	return ;
	#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	struct ieee80211_supported_band *band_5GHz = wiphy->bands[NL80211_BAND_5GHZ];
	struct ieee80211_supported_band *band_2GHz = wiphy->bands[NL80211_BAND_2GHZ];
	int i;
	int nss = rwnx_hw->mod_params->nss;
	struct ieee80211_sta_he_cap *he_cap;
	int mcs_map;

	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80)
		nss  = 1;

	if (!rwnx_hw->mod_params->he_on) {
		band_2GHz->iftype_data = NULL;
		band_2GHz->n_iftype_data = 0;
		if (rwnx_hw->band_5g_support) {
			band_5GHz->iftype_data = NULL;
			band_5GHz->n_iftype_data = 0;
		}
		return;
	}
	he_cap = (struct ieee80211_sta_he_cap *) &band_2GHz->iftype_data->he_cap;
	he_cap->has_he = true;
	he_cap->he_cap_elem.mac_cap_info[2] |= IEEE80211_HE_MAC_CAP2_ALL_ACK;
	if (rwnx_hw->mod_params->use_2040) {
		he_cap->he_cap_elem.phy_cap_info[0] |=
						IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_IN_2G;
		he_cap->ppe_thres[0] |= 0x10;
	}
	if (rwnx_hw->mod_params->use_80) {
		he_cap->ppe_thres[0] |= 0x20;
		he_cap->ppe_thres[2] |= 0xc0;
		he_cap->ppe_thres[3] |= 0x01;
	}
	if (rwnx_hw->mod_params->nss == 2) {
		he_cap->ppe_thres[0] |= 0x01;
		he_cap->ppe_thres[3] |= 0x70;
		he_cap->ppe_thres[4] |= 0x1c;
		he_cap->ppe_thres[5] |= 0x07;
	}
	//if (rwnx_hw->mod_params->use_80)
	{
		he_cap->he_cap_elem.phy_cap_info[0] |=
						IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G;
	}
	if (rwnx_hw->mod_params->ldpc_on) {
		he_cap->he_cap_elem.phy_cap_info[1] |= IEEE80211_HE_PHY_CAP1_LDPC_CODING_IN_PAYLOAD;
	} else {
		// If no LDPC is supported, we have to limit to MCS0_9, as LDPC is mandatory
		// for MCS 10 and 11
		rwnx_hw->mod_params->he_mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map,
												IEEE80211_HE_MCS_SUPPORT_0_9);
	}
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
	he_cap->he_cap_elem.phy_cap_info[1] |= IEEE80211_HE_PHY_CAP1_HE_LTF_AND_GI_FOR_HE_PPDUS_0_8US
											| IEEE80211_HE_PHY_CAP1_MIDAMBLE_RX_TX_MAX_NSTS;

	he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_MIDAMBLE_RX_TX_MAX_NSTS |
										   IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US |
										   IEEE80211_HE_PHY_CAP2_DOPPLER_RX;
	#else
	he_cap->he_cap_elem.phy_cap_info[1] |= IEEE80211_HE_PHY_CAP1_HE_LTF_AND_GI_FOR_HE_PPDUS_0_8US;
	he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US |
										   IEEE80211_HE_PHY_CAP2_DOPPLER_RX;
	#endif
	if (rwnx_hw->mod_params->stbc_on)
		he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_STBC_RX_UNDER_80MHZ;

	if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2 && rwnx_hw->mod_params->stbc_on == true) {
		he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_STBC_TX_UNDER_80MHZ;
	}

	#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
	he_cap->he_cap_elem.phy_cap_info[3] |=
		#if 0
		IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_16_QAM |
		IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_16_QAM |
		#else
		IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_BPSK |
		IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_BPSK |
		#endif
		IEEE80211_HE_PHY_CAP3_DCM_MAX_RX_NSS_1 |
		IEEE80211_HE_PHY_CAP3_DCM_MAX_TX_NSS_1 |
		IEEE80211_HE_PHY_CAP3_RX_PARTIAL_BW_SU_IN_20MHZ_MU;
	#else
	he_cap->he_cap_elem.phy_cap_info[3] |=
		#if 0
		IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_16_QAM |
		IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_16_QAM |
		#else
		IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_BPSK |
		IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_BPSK |
		#endif
		IEEE80211_HE_PHY_CAP3_DCM_MAX_RX_NSS_1 |
		IEEE80211_HE_PHY_CAP3_DCM_MAX_TX_NSS_1 |
		IEEE80211_HE_PHY_CAP3_RX_PARTIAL_BW_SU_IN_20MHZ_MU;
	#endif


	if (rwnx_hw->mod_params->bfmee) {
		he_cap->he_cap_elem.phy_cap_info[4] |= IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE;
		he_cap->he_cap_elem.phy_cap_info[4] |=
					 IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_4;
	}
	he_cap->he_cap_elem.phy_cap_info[5] |= IEEE80211_HE_PHY_CAP5_NG16_SU_FEEDBACK |
										   IEEE80211_HE_PHY_CAP5_NG16_MU_FEEDBACK;
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
	he_cap->he_cap_elem.phy_cap_info[6] |= IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_42_SU |
					IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_75_MU |
					IEEE80211_HE_PHY_CAP6_TRIG_SU_BEAMFORMING_FB |
					IEEE80211_HE_PHY_CAP6_TRIG_MU_BEAMFORMING_PARTIAL_BW_FB |
					IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT |
					IEEE80211_HE_PHY_CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO |
					IEEE80211_HE_PHY_CAP6_PARTIAL_BW_EXT_RANGE;
	#else
	he_cap->he_cap_elem.phy_cap_info[6] |= IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_42_SU |
					IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_75_MU |
					IEEE80211_HE_PHY_CAP6_TRIG_SU_BEAMFORMER_FB |
					IEEE80211_HE_PHY_CAP6_TRIG_MU_BEAMFORMER_FB |
					IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT |
					IEEE80211_HE_PHY_CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO |
					IEEE80211_HE_PHY_CAP6_PARTIAL_BW_EXT_RANGE;
	#endif

	//he_cap->he_cap_elem.phy_cap_info[7] |= IEEE80211_HE_PHY_CAP7_HE_SU_MU_PPDU_4XLTF_AND_08_US_GI;
	he_cap->he_cap_elem.phy_cap_info[8] |= IEEE80211_HE_PHY_CAP8_20MHZ_IN_40MHZ_HE_PPDU_IN_2G;
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
	he_cap->he_cap_elem.phy_cap_info[9] |= IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_COMP_SIGB |
					IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_NON_COMP_SIGB;
	#endif
	mcs_map = rwnx_hw->mod_params->he_mcs_map;
	memset(&he_cap->he_mcs_nss_supp, 0, sizeof(he_cap->he_mcs_nss_supp));
	for (i = 0; i < nss; i++) {
		__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
		he_cap->he_mcs_nss_supp.rx_mcs_80 |= cpu_to_le16(mcs_map << (i*2));
		he_cap->he_mcs_nss_supp.rx_mcs_160 |= unsup_for_ss;
		he_cap->he_mcs_nss_supp.rx_mcs_80p80 |= unsup_for_ss;
		//mcs_map = IEEE80211_HE_MCS_SUPPORT_0_7;
		}
	for (; i < 8; i++) {
		__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
		he_cap->he_mcs_nss_supp.rx_mcs_80 |= unsup_for_ss;
		he_cap->he_mcs_nss_supp.rx_mcs_160 |= unsup_for_ss;
		he_cap->he_mcs_nss_supp.rx_mcs_80p80 |= unsup_for_ss;
		}
	mcs_map = rwnx_hw->mod_params->he_mcs_map;
	for (i = 0; i < nss; i++) {
		__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
		he_cap->he_mcs_nss_supp.tx_mcs_80 |= cpu_to_le16(mcs_map << (i*2));
		he_cap->he_mcs_nss_supp.tx_mcs_160 |= unsup_for_ss;
		he_cap->he_mcs_nss_supp.tx_mcs_80p80 |= unsup_for_ss;
		//mcs_map = min_t(int, rwnx_hw->mod_params->he_mcs_map,
		//				IEEE80211_HE_MCS_SUPPORT_0_7);
	}
	for (; i < 8; i++) {
		__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
		he_cap->he_mcs_nss_supp.tx_mcs_80 |= unsup_for_ss;
		he_cap->he_mcs_nss_supp.tx_mcs_160 |= unsup_for_ss;
		he_cap->he_mcs_nss_supp.tx_mcs_80p80 |= unsup_for_ss;
	}

	if (rwnx_hw->band_5g_support) {
		he_cap = (struct ieee80211_sta_he_cap *) &band_5GHz->iftype_data->he_cap;
		he_cap->has_he = true;
		he_cap->he_cap_elem.mac_cap_info[2] |= IEEE80211_HE_MAC_CAP2_ALL_ACK;
		if (rwnx_hw->mod_params->use_2040) {
			he_cap->he_cap_elem.phy_cap_info[0] |=
							IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_IN_2G;
			he_cap->ppe_thres[0] |= 0x10;
		}
		if (rwnx_hw->mod_params->use_80) {
			he_cap->ppe_thres[0] |= 0x20;
			he_cap->ppe_thres[2] |= 0xc0;
			he_cap->ppe_thres[3] |= 0x01;
		}
		if (rwnx_hw->mod_params->nss == 2) {
			he_cap->ppe_thres[0] |= 0x01;
			he_cap->ppe_thres[3] |= 0x70;
			he_cap->ppe_thres[4] |= 0x1c;
			he_cap->ppe_thres[5] |= 0x07;
		}

		//if (rwnx_hw->mod_params->use_80)
		{
			he_cap->he_cap_elem.phy_cap_info[0] |=
							IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G;
		}
		if (rwnx_hw->mod_params->ldpc_on) {
			he_cap->he_cap_elem.phy_cap_info[1] |= IEEE80211_HE_PHY_CAP1_LDPC_CODING_IN_PAYLOAD;
		} else {
			// If no LDPC is supported, we have to limit to MCS0_9, as LDPC is mandatory
			// for MCS 10 and 11
			rwnx_hw->mod_params->he_mcs_map = min_t(int, rwnx_hw->mod_params->mcs_map,
													IEEE80211_HE_MCS_SUPPORT_0_9);
		}
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
			he_cap->he_cap_elem.phy_cap_info[1] |= IEEE80211_HE_PHY_CAP1_HE_LTF_AND_GI_FOR_HE_PPDUS_0_8US |
				IEEE80211_HE_PHY_CAP1_MIDAMBLE_RX_TX_MAX_NSTS;
			he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_MIDAMBLE_RX_TX_MAX_NSTS |
				IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US |
				IEEE80211_HE_PHY_CAP2_DOPPLER_RX;
		#else
			he_cap->he_cap_elem.phy_cap_info[1] |= IEEE80211_HE_PHY_CAP1_HE_LTF_AND_GI_FOR_HE_PPDUS_0_8US;
			he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US |
				IEEE80211_HE_PHY_CAP2_DOPPLER_RX;
		#endif
		if (rwnx_hw->mod_params->stbc_on)
			he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_STBC_RX_UNDER_80MHZ;

		if (rwnx_hw->pcidev->chip_id == PRODUCT_ID_AIC8800D80X2 && rwnx_hw->mod_params->stbc_on == true) {
			he_cap->he_cap_elem.phy_cap_info[2] |= IEEE80211_HE_PHY_CAP2_STBC_TX_UNDER_80MHZ;
		}
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
			he_cap->he_cap_elem.phy_cap_info[3] |=
													#if 0
													IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_16_QAM |
													IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_16_QAM |
													#else
													IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_BPSK |
													IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_BPSK |
													#endif
													IEEE80211_HE_PHY_CAP3_DCM_MAX_RX_NSS_1 |
													IEEE80211_HE_PHY_CAP3_DCM_MAX_TX_NSS_1 |
													IEEE80211_HE_PHY_CAP3_RX_PARTIAL_BW_SU_IN_20MHZ_MU;
		#else
			he_cap->he_cap_elem.phy_cap_info[3] |=
													#if 0
													IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_16_QAM |
													IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_16_QAM |
													#else
													IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_BPSK |
													IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_BPSK |
													#endif
													IEEE80211_HE_PHY_CAP3_DCM_MAX_RX_NSS_1 |
													IEEE80211_HE_PHY_CAP3_DCM_MAX_TX_NSS_1 |
													IEEE80211_HE_PHY_CAP3_RX_PARTIAL_BW_SU_IN_20MHZ_MU;
		#endif

		if (rwnx_hw->mod_params->bfmee) {
			he_cap->he_cap_elem.phy_cap_info[4] |= IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE;
			he_cap->he_cap_elem.phy_cap_info[4] |=
						 IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_4;
		}
		he_cap->he_cap_elem.phy_cap_info[5] |= IEEE80211_HE_PHY_CAP5_NG16_SU_FEEDBACK |
											   IEEE80211_HE_PHY_CAP5_NG16_MU_FEEDBACK;
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0)
			he_cap->he_cap_elem.phy_cap_info[6] |= IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_42_SU |
				IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_75_MU |
				IEEE80211_HE_PHY_CAP6_TRIG_SU_BEAMFORMING_FB |
				IEEE80211_HE_PHY_CAP6_TRIG_MU_BEAMFORMING_PARTIAL_BW_FB |
				IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT |
				IEEE80211_HE_PHY_CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO |
				IEEE80211_HE_PHY_CAP6_PARTIAL_BW_EXT_RANGE;
		#else
			he_cap->he_cap_elem.phy_cap_info[6] |= IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_42_SU |
				IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_75_MU |
				IEEE80211_HE_PHY_CAP6_TRIG_SU_BEAMFORMER_FB |
				IEEE80211_HE_PHY_CAP6_TRIG_MU_BEAMFORMER_FB |
				IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT |
				IEEE80211_HE_PHY_CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO |
				IEEE80211_HE_PHY_CAP6_PARTIAL_BW_EXT_RANGE;
		#endif

		//he_cap->he_cap_elem.phy_cap_info[7] |= IEEE80211_HE_PHY_CAP7_HE_SU_MU_PPDU_4XLTF_AND_08_US_GI;
		he_cap->he_cap_elem.phy_cap_info[8] |= IEEE80211_HE_PHY_CAP8_20MHZ_IN_40MHZ_HE_PPDU_IN_2G;
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
		he_cap->he_cap_elem.phy_cap_info[9] |= IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_COMP_SIGB |
											   IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_NON_COMP_SIGB;
		#endif
		mcs_map = rwnx_hw->mod_params->he_mcs_map;
		memset(&he_cap->he_mcs_nss_supp, 0, sizeof(he_cap->he_mcs_nss_supp));
		for (i = 0; i < nss; i++) {
			__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
			he_cap->he_mcs_nss_supp.rx_mcs_80 |= cpu_to_le16(mcs_map << (i*2));
			he_cap->he_mcs_nss_supp.rx_mcs_160 |= unsup_for_ss;
			he_cap->he_mcs_nss_supp.rx_mcs_80p80 |= unsup_for_ss;
			//mcs_map = IEEE80211_HE_MCS_SUPPORT_0_7;
		}
		for (; i < 8; i++) {
			__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
			he_cap->he_mcs_nss_supp.rx_mcs_80 |= unsup_for_ss;
			he_cap->he_mcs_nss_supp.rx_mcs_160 |= unsup_for_ss;
			he_cap->he_mcs_nss_supp.rx_mcs_80p80 |= unsup_for_ss;
		}
		mcs_map = rwnx_hw->mod_params->he_mcs_map;
		for (i = 0; i < nss; i++) {
			__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
			he_cap->he_mcs_nss_supp.tx_mcs_80 |= cpu_to_le16(mcs_map << (i*2));
			he_cap->he_mcs_nss_supp.tx_mcs_160 |= unsup_for_ss;
			he_cap->he_mcs_nss_supp.tx_mcs_80p80 |= unsup_for_ss;
			//mcs_map = min_t(int, rwnx_hw->mod_params->he_mcs_map,
			//				IEEE80211_HE_MCS_SUPPORT_0_7);
		}
		for (; i < 8; i++) {
			__le16 unsup_for_ss = cpu_to_le16(IEEE80211_HE_MCS_NOT_SUPPORTED << (i*2));
			he_cap->he_mcs_nss_supp.tx_mcs_80 |= unsup_for_ss;
			he_cap->he_mcs_nss_supp.tx_mcs_160 |= unsup_for_ss;
			he_cap->he_mcs_nss_supp.tx_mcs_80p80 |= unsup_for_ss;
		}
	}
#endif
}

static void rwnx_set_wiphy_params(struct rwnx_hw *rwnx_hw, struct wiphy *wiphy)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
		struct ieee80211_regdomain *regdomain;
#endif

#ifdef CONFIG_RWNX_FULLMAC
	/* FULLMAC specific parameters */
	wiphy->flags |= WIPHY_FLAG_REPORTS_OBSS;
	wiphy->max_scan_ssids = SCAN_SSID_MAX;
	wiphy->max_scan_ie_len = SCANU_MAX_IE_LEN;
#endif /* CONFIG_RWNX_FULLMAC */

	if (rwnx_hw->mod_params->tdls) {
		/* TDLS support */
		wiphy->flags |= WIPHY_FLAG_SUPPORTS_TDLS;
#ifdef CONFIG_RWNX_FULLMAC
		/* TDLS external setup support */
		wiphy->flags |= WIPHY_FLAG_TDLS_EXTERNAL_SETUP;
#endif
	}

	if (rwnx_hw->mod_params->ap_uapsd_on)
		wiphy->flags |= WIPHY_FLAG_AP_UAPSD;

#ifdef CONFIG_RWNX_FULLMAC
	if (rwnx_hw->mod_params->ps_on)
		wiphy->flags |= WIPHY_FLAG_PS_ON_BY_DEFAULT;
	else
		wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;
#endif

if (rwnx_hw->mod_params->custregd) {

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
		// Apply custom regulatory. Note that for recent kernel versions we use instead the
		// REGULATORY_WIPHY_SELF_MANAGED flag, along with the regulatory_set_wiphy_regd()
		// function, that needs to be called after wiphy registration
		memcpy(country_code, default_ccode, sizeof(default_ccode));
		regdomain = getRegdomainFromRwnxDB(wiphy, default_ccode);
		AICWFDBG(LOGINFO, KERN_CRIT
			   "\n\n%s: CAUTION: USING PERMISSIVE CUSTOM REGULATORY RULES\n\n",
			   __func__);
		wiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;
		wiphy->regulatory_flags |= REGULATORY_IGNORE_STALE_KICKOFF;
		wiphy_apply_custom_regulatory(wiphy, regdomain);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
		memcpy(country_code, default_ccode, sizeof(default_ccode));
		regdomain = getRegdomainFromRwnxDB(wiphy, default_ccode);
		AICWFDBG(LOGINFO, KERN_CRIT"%s: Registering custom regulatory\n", __func__);
		wiphy->flags |= WIPHY_FLAG_CUSTOM_REGULATORY;
		wiphy_apply_custom_regulatory(wiphy, regdomain);
#endif
		// Check if custom channel set shall be enabled. In such case only monitor mode is
		// supported
		if (rwnx_hw->mod_params->custchan) {
			wiphy->interface_modes = BIT(NL80211_IFTYPE_MONITOR);

			// Enable "extra" channels
			wiphy->bands[NL80211_BAND_2GHZ]->n_channels += 13;
			//#ifdef USE_5G
			if (rwnx_hw->band_5g_support) {
				wiphy->bands[NL80211_BAND_5GHZ]->n_channels += 59;
			}
			//#endif
		}
	}
}

int rwnx_handle_dynparams(struct rwnx_hw *rwnx_hw, struct wiphy *wiphy)
{
	/* Check compatibility between requested parameters and HW/SW features */
	int ret;
	/* Allocate the RX buffers according to the maximum AMSDU RX size */
	#ifdef CONFIG_RX_SKBLIST
	ret = rwnx_ipc_rxbuf_init(rwnx_hw, 2048);
	#else
	ret = rwnx_ipc_rxbuf_init(rwnx_hw, (rwnx_hw, (4 * (rwnx_hw->mod_params->amsdu_rx_max + 1) + 1) * 1024));
	#endif
	if (ret) {
		wiphy_err(wiphy, "Cannot allocate the RX buffers\n");
		return ret;
	}

	rwnx_hw->mod_params->sgi80 = true;
	rwnx_hw->mod_params->use_80 = true;

	/* Set wiphy parameters */
	rwnx_set_wiphy_params(rwnx_hw, wiphy);
	/* Set VHT capabilities */
	rwnx_set_vht_capa(rwnx_hw, wiphy);
	/* Set HE capabilities */
	rwnx_set_he_capa(rwnx_hw, wiphy);
	/* Set HT capabilities */
	rwnx_set_ht_capa(rwnx_hw, wiphy);
	/* Set RF specific parameters (shall be done last as it might change some
	   capabilities previously set) */
#if 0
	rwnx_set_rf_params(rwnx_hw, wiphy);
#endif
	return 0;
}

void rwnx_custregd(struct rwnx_hw *rwnx_hw, struct wiphy *wiphy)
{
// For older kernel version, the custom regulatory is applied before the wiphy
// registration (in rwnx_set_wiphy_params()), so nothing has to be done here

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
	wiphy->regulatory_flags |= REGULATORY_IGNORE_STALE_KICKOFF;
#endif
	if (!rwnx_hw->mod_params->custregd)
		return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
	wiphy->regulatory_flags |= REGULATORY_WIPHY_SELF_MANAGED;

	rtnl_lock();
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
		if (regulatory_set_wiphy_regd_sync(wiphy, getRegdomainFromRwnxDB(wiphy, default_ccode))) {
			wiphy_err(wiphy, "Failed to set custom regdomain\n");
		}
	#else
		if (regulatory_set_wiphy_regd_sync_rtnl(wiphy, getRegdomainFromRwnxDB(wiphy, default_ccode))) {
			wiphy_err(wiphy, "Failed to set custom regdomain\n");
		}
	#endif

	else {
		wiphy_err(wiphy, "\n"
				  "*******************************************************\n"
				  "** CAUTION: USING PERMISSIVE CUSTOM REGULATORY RULES **\n"
				  "*******************************************************\n");
	}
	 rtnl_unlock();
#endif
}
