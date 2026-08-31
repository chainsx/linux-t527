#include "aic_vendor.h"
#include "rwnx_defs.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/rtnetlink.h>
#include <linux/types.h>
#include <linux/module.h>
#include <net/netlink.h>
#include "rwnx_version_gen.h"
#include "rwnx_msg_tx.h"
#include "aicwf_nan.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)

#ifdef CONFIG_APF
#define AIC_APF_MEM_SIZE      4096
#define AIC_APF_VERSION       6100

static char apfProgram[AIC_APF_MEM_SIZE];
#endif

static struct wifi_ring_buffer_status ring_buffer[] = {
	{
		.name            = "aicwf_ring_buffer0",
		.flags           = 0,
		.ring_id         = 0,
		.verbose_level   = 0,
		.written_bytes   = 0,
		.read_bytes      = 0,
		.written_records = 0,
	},
};

static struct wlan_driver_wake_reason_cnt_t wake_reason_cnt = {
	.total_cmd_event_wake = 10,
};

int aic_dev_start_mkeep_alive(struct rwnx_hw *rwnx_hw, struct rwnx_vif *rwnx_vif,
			u8 mkeep_alive_id, u8 *ip_pkt, u16 ip_pkt_len, u8 *src_mac, u8 *dst_mac, u32 period_msec)
{
	u8 *data, *pos;

	data = kzalloc(ip_pkt_len + 14, GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	pos = data;
	memcpy(pos, dst_mac, 6);
	pos += 6;
	memcpy(pos, src_mac, 6);
	pos += 6;
	/* Mapping Ethernet type (ETHERTYPE_IP: 0x0800) */
	*(pos++) = 0x08;
	*(pos++) = 0x00;

	/* Mapping IP pkt */
	memcpy(pos, ip_pkt, ip_pkt_len);
	pos += ip_pkt_len;

	//add send 802.3 pkt(raw data)
	kfree(data);

	return 0;
}

int aic_dev_stop_mkeep_alive(struct rwnx_hw *rwnx_hw, struct rwnx_vif *rwnx_vif, u8 mkeep_alive_id)
{
	int  res = -1;

	/*
	 * The mkeep_alive packet is for STA interface only; if the bss is configured as AP,
	 * dongle shall reject a mkeep_alive request.
	 */
	if (rwnx_vif->wdev.iftype != NL80211_IFTYPE_STATION)
		return res;

	printk("%s execution\n", __func__);

	//add send stop keep alive
	res = 0;
	return res;
}

static int aicwf_vendor_start_mkeep_alive(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	/* max size of IP packet for keep alive */
	const int MKEEP_ALIVE_IP_PKT_MAX = 256;

	int ret = 0, rem, type;
	u8 mkeep_alive_id = 0;
	u8 *ip_pkt = NULL;
	u16 ip_pkt_len = 0;
	u8 src_mac[6];
	u8 dst_mac[6];
	u32 period_msec = 0;
	const struct nlattr *iter;
	struct rwnx_hw *rwnx_hw = wiphy_priv(wiphy);
	struct rwnx_vif *rwnx_vif = container_of(wdev, struct rwnx_vif, wdev);
	gfp_t kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
	printk("%s\n", __func__);

	nla_for_each_attr(iter, data, len, rem) {
		type = nla_type(iter);
		switch (type) {
		case MKEEP_ALIVE_ATTRIBUTE_ID:
			mkeep_alive_id = nla_get_u8(iter);
			break;
		case MKEEP_ALIVE_ATTRIBUTE_IP_PKT_LEN:
			ip_pkt_len = nla_get_u16(iter);
			if (ip_pkt_len > MKEEP_ALIVE_IP_PKT_MAX) {
				ret = -EINVAL;
				goto exit;
			}
			break;
		case MKEEP_ALIVE_ATTRIBUTE_IP_PKT:
			if (!ip_pkt_len) {
				ret = -EINVAL;
				printk("ip packet length is 0\n");
				goto exit;
			}
			ip_pkt = (u8 *)kzalloc(ip_pkt_len, kflags);
			if (ip_pkt == NULL) {
				ret = -ENOMEM;
				printk("Failed to allocate mem for ip packet\n");
				goto exit;
			}
			memcpy(ip_pkt, (u8 *)nla_data(iter), ip_pkt_len);
			break;
		case MKEEP_ALIVE_ATTRIBUTE_SRC_MAC_ADDR:
			memcpy(src_mac, nla_data(iter), 6);
			break;
		case MKEEP_ALIVE_ATTRIBUTE_DST_MAC_ADDR:
			memcpy(dst_mac, nla_data(iter), 6);
			break;
		case MKEEP_ALIVE_ATTRIBUTE_PERIOD_MSEC:
			period_msec = nla_get_u32(iter);
			break;
		default:
			pr_err("%s(%d), Unknown type: %d\n", __func__, __LINE__, type);
			ret = -EINVAL;
			goto exit;
		}
	}

	if (ip_pkt == NULL) {
		ret = -EINVAL;
		printk("ip packet is NULL\n");
		goto exit;
	}

	ret = aic_dev_start_mkeep_alive(rwnx_hw, rwnx_vif, mkeep_alive_id, ip_pkt, ip_pkt_len, src_mac,
		dst_mac, period_msec);
	if (ret < 0) {
		printk("start_mkeep_alive is failed ret: %d\n", ret);
	}

exit:
	if (ip_pkt) {
		kfree(ip_pkt);
	}

	return ret;
}

static int aicwf_vendor_stop_mkeep_alive(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret = 0, rem, type;
	u8 mkeep_alive_id = 0;
	const struct nlattr *iter;
	struct rwnx_hw *rwnx_hw = wiphy_priv(wiphy);
	struct rwnx_vif *rwnx_vif = container_of(wdev, struct rwnx_vif, wdev);

	printk("%s\n", __func__);
	nla_for_each_attr(iter, data, len, rem) {
		type = nla_type(iter);
		switch (type) {
		case MKEEP_ALIVE_ATTRIBUTE_ID:
			mkeep_alive_id = nla_get_u8(iter);
			break;
		default:
			pr_err("%s(%d), Unknown type: %d\n", __func__, __LINE__, type);
			ret = -EINVAL;
			break;
		}
	}

	ret = aic_dev_stop_mkeep_alive(rwnx_hw, rwnx_vif, mkeep_alive_id);
	if (ret < 0) {
		printk("stop_mkeep_alive is failed ret: %d\n", ret);
	}

	return ret;
}

static int aicwf_vendor_get_ver(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret = 0, rem, type;
	const struct nlattr *iter;
	int payload = 0;
	char version[128];
	int  attr = -1;
	struct sk_buff *reply;

	nla_for_each_attr(iter, data, len, rem) {
		type = nla_type(iter);
		switch (type) {
		case LOGGER_ATTRIBUTE_DRIVER_VER:
			memcpy(version, RWNX_VERS_BANNER, sizeof(RWNX_VERS_BANNER));
			payload = strlen(version);
			attr = LOGGER_ATTRIBUTE_DRIVER_VER;
			break;
		case LOGGER_ATTRIBUTE_FW_VER:
			memcpy(version, wiphy->fw_version, sizeof(wiphy->fw_version));
			payload = strlen(version);
			attr = LOGGER_ATTRIBUTE_FW_VER;
			break;
		default:
			pr_err("%s(%d), Unknown type: %d\n", __func__, __LINE__, type);
			return -EINVAL;
		}
	}

	if (attr < 0)
		return -EINVAL;

	reply = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload);

	if (!reply)
		return -ENOMEM;

	if (nla_put(reply, attr,
			payload, version)) {
		wiphy_err(wiphy, "%s put version error\n", __func__);
		goto out_put_fail;
	}

	ret = cfg80211_vendor_cmd_reply(reply);
	if (ret)
		wiphy_err(wiphy, "%s reply cmd error\n", __func__);
	return ret;

out_put_fail:
	kfree_skb(reply);
	return -EMSGSIZE;
}

static int aicwf_vendor_subcmd_get_channel_list(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret = 0, rem, type;
	const struct nlattr *iter;
	struct sk_buff *reply;
	int num_channels = 0;
	int *channel_list = NULL;
	int payload;
	int i = 0;
	struct rwnx_hw *rwnx_hw = wiphy_priv(wiphy);
	struct ieee80211_supported_band *rwnx_band_2GHz = rwnx_hw->wiphy->bands[NL80211_BAND_2GHZ];
	struct ieee80211_supported_band *rwnx_band_5GHz = rwnx_hw->wiphy->bands[NL80211_BAND_5GHZ];

	num_channels += rwnx_band_2GHz->n_channels;
	num_channels += (rwnx_hw->band_5g_support) ? rwnx_band_5GHz->n_channels : 0;

	channel_list = (int *)kzalloc(sizeof(int) * num_channels, GFP_KERNEL);
	if (!channel_list)
		return -ENOMEM;

	for (i = 0; i < rwnx_band_2GHz->n_channels; i++)
		channel_list[i] = rwnx_band_2GHz->channels[i].center_freq;

	for (; rwnx_hw->band_5g_support && i < num_channels; i++)
		channel_list[i] = rwnx_band_5GHz->channels[i].center_freq;

	payload = sizeof(num_channels) + sizeof(int) * num_channels + 4;

	nla_for_each_attr(iter, data, len, rem) {
		type = nla_type(iter);
		switch (type) {
		case GSCAN_ATTRIBUTE_BAND:
			reply = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload);

			if (!reply)
				return -ENOMEM;

			if (nla_put_u32(reply, GSCAN_ATTRIBUTE_NUM_CHANNELS, num_channels))
				goto out_put_fail;

			if (nla_put(reply, GSCAN_ATTRIBUTE_CHANNEL_LIST, sizeof(int) * num_channels, channel_list))
				goto out_put_fail;

			ret = cfg80211_vendor_cmd_reply(reply);
			if (ret)
				wiphy_err(wiphy, "%s reply cmd error\n", __func__);
			break;
		default:
			pr_err("%s(%d), Unknown type: %d\n", __func__, __LINE__, type);
			return -EINVAL;
		}
	}

	kfree(channel_list);
	return ret;

out_put_fail:
	kfree(channel_list);
	kfree_skb(reply);
	return -EMSGSIZE;
}

static int aicwf_vendor_subcmd_set_country_code(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret = 0, rem, type;
	const struct nlattr *iter;

	nla_for_each_attr(iter, data, len, rem) {
		type = nla_type(iter);
		switch (type) {
		case ANDR_WIFI_ATTRIBUTE_COUNTRY:
			printk("%s(%d), ANDR_WIFI_ATTRIBUTE_COUNTRY: %s\n", __func__, __LINE__, (char *)nla_data(iter));
			break;
		default:
			pr_err("%s(%d), Unknown type: %d\n", __func__, __LINE__, type);
			return -EINVAL;
		}
	}

	/* TODO
	 * Add handle in the future!
	 */

	return ret;
}

static int aicwf_vendor_logger_trigger_memory_dump(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	/* TODO
	 * Add handle in the future!
	 */
	return 0;
}

static int aicwf_vendor_subcmd_get_feature_set(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret;
	struct sk_buff *reply;
	uint32_t feature = 0, payload;
	struct rwnx_hw *rwnx_hw = wiphy_priv(wiphy);

	payload = sizeof(feature);
	reply = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload);

	if (!reply)
		return -ENOMEM;

	/* TODO
	 * Add handle in the future!
	 */
	/*bit 1:Basic infrastructure mode*/
	if (wiphy->interface_modes & BIT(NL80211_IFTYPE_STATION))
		feature |= WIFI_FEATURE_INFRA;

	/*bit 2:Support for 5 GHz Band*/
	if (rwnx_hw->band_5g_support)
		feature |= WIFI_FEATURE_INFRA_5G;

	/*bit3:HOTSPOT is a supplicant feature, enable it by default*/
	feature |= WIFI_FEATURE_HOTSPOT;

	/*bit 4:P2P*/
	if ((wiphy->interface_modes & BIT(NL80211_IFTYPE_P2P_CLIENT)) &&
		(wiphy->interface_modes & BIT(NL80211_IFTYPE_P2P_GO)))
		feature |= WIFI_FEATURE_P2P;

	/*bit 5:soft AP feature supported*/
	if (wiphy->interface_modes & BIT(NL80211_IFTYPE_AP))
		feature |= WIFI_FEATURE_SOFT_AP;

#ifdef CONFIG_AIC_NAN
	/*bit 7:NAN*/
	if (wiphy->interface_modes & BIT(NL80211_IFTYPE_NAN))
		feature |= WIFI_FEATURE_NAN;
#endif

	/*bit 18:WiFi Logger*/
	feature |= WIFI_FEATURE_LOGGER;

	/*bit 21:WiFi mkeep_alive*/
	feature |= WIFI_FEATURE_MKEEP_ALIVE;

	feature |= WIFI_FEATURE_SET_LATENCY_MODE;

	if (nla_put_u32(reply, ANDR_WIFI_ATTRIBUTE_NUM_FEATURE_SET, feature)) {
		wiphy_err(wiphy, "%s put u32 error\n", __func__);
		goto out_put_fail;
	}

	ret = cfg80211_vendor_cmd_reply(reply);
	if (ret)
		wiphy_err(wiphy, "%s reply cmd error\n", __func__);

	return ret;

out_put_fail:
	kfree_skb(reply);
	return -EMSGSIZE;
}

static int aicwf_vendor_logger_get_feature(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret;
	struct sk_buff *reply;
	uint32_t feature = 0, payload;

	payload = sizeof(feature);
	reply = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload);

	if (!reply)
		return -ENOMEM;

	feature |= WIFI_LOGGER_MEMORY_DUMP_SUPPORTED;
	feature |= WIFI_LOGGER_CONNECT_EVENT_SUPPORTED;

	/*vts will test wake reason state function*/
	feature |= WIFI_LOGGER_WAKE_LOCK_SUPPORTED;

	if (nla_put_u32(reply, ANDR_WIFI_ATTRIBUTE_NUM_FEATURE_SET, feature)) {
		wiphy_err(wiphy, "put skb u32 failed\n");
		goto out_put_fail;
	}

	ret = cfg80211_vendor_cmd_reply(reply);
	if (ret)
		wiphy_err(wiphy, "reply cmd error\n");

	return ret;

out_put_fail:
	kfree_skb(reply);
	return -EMSGSIZE;
}

static int aicwf_vendor_logger_get_ring_status(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret;
	struct sk_buff *reply;
	uint32_t payload;
	uint32_t ring_buffer_nums = sizeof(ring_buffer) / sizeof(ring_buffer[0]);

	payload = sizeof(ring_buffer_nums) + sizeof(ring_buffer);
	reply = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload);

	if (!reply)
		return -ENOMEM;

	if (nla_put_u32(reply, LOGGER_ATTRIBUTE_RING_NUM, ring_buffer_nums)) {
		wiphy_err(wiphy, "put skb u32 failed\n");
		goto out_put_fail;
	}

	if (nla_put(reply, LOGGER_ATTRIBUTE_RING_STATUS, sizeof(ring_buffer), ring_buffer)) {
		wiphy_err(wiphy, "put skb failed\n");
		goto out_put_fail;
	}

	ret = cfg80211_vendor_cmd_reply(reply);
	if (ret)
		wiphy_err(wiphy, "reply cmd error\n");

	return ret;

out_put_fail:
	kfree_skb(reply);
	return -EMSGSIZE;
}

static int aicwf_vendor_logger_start_logging(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret = 0, rem, type, intval, size, i;
	const struct nlattr *iter;
	struct wifi_ring_buffer_status rb;

	nla_for_each_attr(iter, data, len, rem) {
		type = nla_type(iter);
		switch (type) {
		case LOGGER_ATTRIBUTE_LOG_LEVEL:
			rb.verbose_level = nla_get_u32(iter);
			break;
		case LOGGER_ATTRIBUTE_RING_FLAGS:
			rb.flags = nla_get_u32(iter);
			break;
		case LOGGER_ATTRIBUTE_LOG_TIME_INTVAL:
			intval = nla_get_u32(iter);
			break;
		case LOGGER_ATTRIBUTE_LOG_MIN_DATA_SIZE:
			size = nla_get_u32(iter);
			break;
		case LOGGER_ATTRIBUTE_RING_NAME:
			strcpy(rb.name, nla_data(iter));
			break;
		default:
			pr_err("%s(%d), Unknown type: %d\n", __func__, __LINE__, type);
			return -EINVAL;
		}
	}

	ret = -EINVAL;
	for (i = 0; i < sizeof(ring_buffer) / sizeof(ring_buffer[0]); i++) {
		if (strcmp(rb.name, ring_buffer[i].name) == 0) {
			ret = 0;
			break;
		}
	}

	/* TODO
	 * Add handle in the future
	 */

	return ret;
}

static int aicwf_vendor_logger_get_ring_data(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret = 0, rem, type, i;
	const struct nlattr *iter;
	struct wifi_ring_buffer_status rb;

	nla_for_each_attr(iter, data, len, rem) {
		type = nla_type(iter);
		switch (type) {
		case LOGGER_ATTRIBUTE_RING_NAME:
			strcpy(rb.name, nla_data(iter));
			break;
		default:
			pr_err("%s(%d), Unknown type: %d\n", __func__, __LINE__, type);
			return -EINVAL;
		}
	}

	ret = -EINVAL;
	for (i = 0; i < sizeof(ring_buffer) / sizeof(ring_buffer[0]); i++) {
		if (strcmp(rb.name, ring_buffer[i].name) == 0) {
			ret = 0;
			break;
		}
	}

	/* TODO
	 * Add handle in the future
	 */

	return ret;
}

static int aicwf_vendor_logger_get_wake_reason_stats(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret;
	struct sk_buff *reply;
	uint32_t payload;

	payload = sizeof(wake_reason_cnt.total_cmd_event_wake);
	reply = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload);

	if (!reply)
		return -ENOMEM;

	/* TODO
	 * Add handle in the future
	 */
	if (nla_put_u32(reply, WAKE_STAT_ATTRIBUTE_TOTAL_CMD_EVENT, wake_reason_cnt.total_cmd_event_wake))
		goto out_put_fail;

	ret = cfg80211_vendor_cmd_reply(reply);
	if (ret)
		wiphy_err(wiphy, "reply cmd error\n");

	return ret;

out_put_fail:
	kfree_skb(reply);
	return -EMSGSIZE;
}

#ifdef CONFIG_APF
static int aicwf_vendor_apf_subcmd_get_capabilities(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret;
	struct sk_buff *reply;
	uint32_t payload;
	printk("%s\n", __func__);

	/* APF_ATTRIBUTE_VERSION + APF_ATTRIBUTE_MAX_LEN */
	payload = sizeof(u32) * 2;
	reply = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload);

	if (!reply)
		return -ENOMEM;

	if (nla_put_u32(reply, APF_ATTRIBUTE_VERSION, AIC_APF_VERSION))
		goto out_put_fail;

	if (nla_put_u32(reply, APF_ATTRIBUTE_MAX_LEN, AIC_APF_MEM_SIZE))
		goto out_put_fail;

	ret = cfg80211_vendor_cmd_reply(reply);
	if (ret)
		wiphy_err(wiphy, "reply cmd error\n");

	return ret;

out_put_fail:
	kfree_skb(reply);
	return -EMSGSIZE;
}

static int aicwf_vendor_apf_set_filter(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret = 0, rem, type;
	const struct nlattr *iter;
	unsigned int mProgramLen = 0;
	struct rwnx_hw *rwnx_hw = wiphy_priv(wiphy);

	printk("%s\n", __func__);

	nla_for_each_attr(iter, data, len, rem) {
		type = nla_type(iter);
		switch (type) {
		case APF_ATTRIBUTE_PROGRAM_LEN:
			memcpy(&mProgramLen, nla_data(iter), sizeof(unsigned int));
			ret = (mProgramLen > 0 && mProgramLen <= AIC_APF_MEM_SIZE) ? 0 : -EINVAL;
			break;
		case APF_ATTRIBUTE_PROGRAM:
			if (mProgramLen > AIC_APF_MEM_SIZE || mProgramLen == 0) {
				printk("%s: apf program size invalid: %d (should > 0 and <= %d)\n",
						__func__, mProgramLen, AIC_APF_MEM_SIZE);
				return -EINVAL;
			}

			memcpy(apfProgram, nla_data(iter), mProgramLen);
			ret = rwnx_send_set_apf_prog_req(rwnx_hw, apfProgram, mProgramLen,
					AIC_APF_VERSION > 4 ? 6 : 4);
			break;
		default:
			pr_err("%s(%d), Unknown type: %d\n", __func__, __LINE__, type);
			return -EINVAL;
		}
	}

	return ret;
}

static int aicwf_vendor_apf_read_filter_data(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void *data, int len)
{
	struct sk_buff *reply;
	int ret, payload, apf_mem_size;
	struct rwnx_hw *rwnx_hw = wiphy_priv(wiphy);

	printk("%s\n", __func__);

	apf_mem_size = AIC_APF_MEM_SIZE;
	payload = sizeof(u32) + apf_mem_size;

	ret = rwnx_send_get_apf_prog_req(rwnx_hw, apfProgram, apf_mem_size);
	if (ret != 0) {
		pr_err("%s, Failed to read apf mem from firmware, ret = %d\n", __func__, ret);
		return ret;
	}

	reply = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload);
	if (!reply)
		return -ENOMEM;

	ret = nla_put_u32(reply, APF_ATTRIBUTE_PROGRAM_LEN, apf_mem_size);
	if (ret < 0) {
		pr_err("%s, Failed to put APF_ATTRIBUTE_MAX_LEN, ret = %d\n", __func__, ret);
		goto out_put_fail;
	}

	/* copy the full apf mem */
	ret = nla_put(reply, APF_ATTRIBUTE_PROGRAM, apf_mem_size, &apfProgram[0]);
	if (ret < 0) {
		pr_err("%s, Failed to put APF_ATTRIBUTE_PROGRAM, ret = %d\n", __func__, ret);
		goto out_put_fail;
	}

	ret = cfg80211_vendor_cmd_reply(reply);
	if (ret)
		wiphy_err(wiphy, "reply cmd error\n");

	return ret;

out_put_fail:
	kfree_skb(reply);
	return -EMSGSIZE;
}
#endif

static int aicwf_vendor_sub_cmd_set_mac(struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int len)
{
	int ret = 0, rem, type;
	const struct nlattr *iter;
	u8 mac[ETH_ALEN];

	nla_for_each_attr(iter, data, len, rem) {
		type = nla_type(iter);
		switch (type) {
		case WIFI_VENDOR_ATTR_DRIVER_MAC_ADDR:
			memcpy(mac, nla_data(iter), ETH_ALEN);
			printk("%s, %02X:%02X:%02X:%02X:%02X:%02X\n", __func__,
					mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			break;
		default:
			pr_err("%s(%d), Unknown type: %d\n", __func__, __LINE__, type);
			return -EINVAL;
		}
	}

	/* TODO
	 * Add handle in the future
	 */

	return ret;
}

static int aicwf_vendor_subcmd_set_latency_mode(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void *data, int len)
{
	int err = 0, rem, type;
	u32 latency_mode;
	const struct nlattr *iter;
#ifdef AICWF_LATENCY_MODE
	struct rwnx_hw *rwnx_hw = wiphy_priv(wiphy);
#endif

	nla_for_each_attr(iter, data, len, rem) {
		type = nla_type(iter);
		switch (type) {
		case ANDR_WIFI_ATTRIBUTE_LATENCY_MODE:
			latency_mode = nla_get_u32(iter);
			printk("%s,Setting latency mode %d\n", __func__, latency_mode);
#ifdef AICWF_LATENCY_MODE
			#ifdef AICWF_SDIO_SUPPORT
			if (latency_mode) {
				rwnx_send_me_set_lp_level(rwnx_hw, 0, 1);
			} else {
				rwnx_send_me_set_lp_level(rwnx_hw, 1, 0);
			}
			#endif
#endif
			break;
		default:
			pr_err("%s(%d), Unknown type: %d\n", __func__, __LINE__, type);
			return err;
		}
	}

	return err;
}

static int aicwf_vendor_logger_start_pkt_fate_monitoring(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void *data, int len)
{
	return -3;  // WIFI_ERROR_NOT_SUPPORTED
}

#ifdef CONFIG_AIC_NAN
static int aicwf_vendor_nan_wifi_subcmd_enable(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void  *data, int len)
{
	int ret = -ENOTSUPP;
	printk("%s, enter\n", __func__);

	return ret;
}

static int aicwf_vendor_nan_wifi_subcmd_disable(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void  *data, int len)
{
	int ret = -ENOTSUPP;
	printk("%s, enter\n", __func__);

	return ret;
}

static int aicwf_vendor_nan_wifi_subcmd_data_path_iface_delete(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void  *data, int len)
{
	int ret = -ENOTSUPP;
	printk("%s, enter\n", __func__);

	return ret;
}

static int aicwf_vendor_nan_wifi_subcmd_get_capabilities(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void  *data, int len)
{
	int ret = 0;
	int payload = 0;
	nan_hal_resp_t nan_req_resp;
	struct sk_buff *reply;
	nan_hal_capabilities_t *capabilities = &nan_req_resp.capabilities;

	printk("%s, enter\n", __func__);

	payload = sizeof(nan_hal_resp_t);
	reply = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload);

	if (!reply)
		return -ENOMEM;

	nan_req_resp.subcmd = NAN_WIFI_SUBCMD_GET_CAPABILITIES;
	capabilities->max_concurrent_nan_clusters = MAX_CONCURRENT_NAN_CLUSTERS;
	capabilities->max_publishes = MAX_PUBLISHES;
	capabilities->max_subscribes = MAX_SUBSCRIBES;
	capabilities->max_service_name_len = MAX_SVC_NAME_LEN;
	capabilities->max_match_filter_len = MAX_MATCH_FILTER_LEN;
	capabilities->max_total_match_filter_len = MAX_TOTAL_MATCH_FILTER_LEN;
	capabilities->max_service_specific_info_len = NAN_MAX_SERVICE_SPECIFIC_INFO_LEN;
	capabilities->max_ndi_interfaces = NAN_MAX_NDI;
	capabilities->max_ndp_sessions = MAX_NDP_SESSIONS;
	capabilities->max_app_info_len = MAX_APP_INFO_LEN;
	capabilities->max_queued_transmit_followup_msgs = MAX_QUEUED_TX_FOLLOUP_MSGS;
	capabilities->max_sdea_service_specific_info_len = MAX_SDEA_SVC_INFO_LEN;
	capabilities->max_subscribe_address = MAX_SUBSCRIBE_ADDRESS;
	capabilities->max_scid_len = NAN_MAX_SCID_BUF_LEN;
	capabilities->is_ndp_security_supported = true;
	capabilities->ndp_supported_bands = NDP_SUPPORTED_BANDS;
	capabilities->ndpe_attr_supported = false;
	capabilities->cipher_suites_supported = -1;
	nan_req_resp.status = 0;

	if (nla_put_nohdr(reply, payload, &nan_req_resp)) {
		wiphy_err(wiphy, "%s put version error\n", __func__);
		return -1;
	}

	ret = cfg80211_vendor_cmd_reply(reply);
	if (ret)
		wiphy_err(wiphy, "%s reply cmd error\n", __func__);

	printk("%s, exit\n", __func__);

	return ret;
}

#define NAN_VERSION_INFO 0x2
static int aicwf_vendor_nan_wifi_subcmd_version_info(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void  *data, int len)
{
	int ret = 0;
	int payload = 0;
	u32 version = NAN_VERSION_INFO;
	struct sk_buff *reply;

	printk("%s, enter\n", __func__);

	payload = sizeof(u32);
	reply = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, payload);

	if (!reply)
		return -ENOMEM;

	if (nla_put_nohdr(reply, payload, &version)) {
		wiphy_err(wiphy, "%s put version error\n", __func__);
		return -1;
	}

	ret = cfg80211_vendor_cmd_reply(reply);
	if (ret)
		wiphy_err(wiphy, "%s reply cmd error\n", __func__);

	printk("%s, exit\n", __func__);

	return ret;
}

static int aicwf_vendor_nan_wifi_subcmd_config(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void  *data, int len)
{
	int ret = -ENOTSUPP;
	printk("%s, enter\n", __func__);

	return ret;
}

static int aicwf_vendor_nan_wifi_subcmd_req_publish(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void  *data, int len)
{
	int ret = -ENOTSUPP;
	printk("%s, enter\n", __func__);

	return ret;
}

static int aicwf_vendor_nan_wifi_subcmd_req_subscribe(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void  *data, int len)
{
	int ret = -ENOTSUPP;
	printk("%s, enter\n", __func__);

	return ret;
}

static int aicwf_vendor_nan_wifi_subcmd_cancel_publish(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void  *data, int len)
{
	int ret = -ENOTSUPP;
	printk("%s, enter\n", __func__);

	return ret;
}

static int aicwf_vendor_nan_wifi_subcmd_cancel_subscribe(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void  *data, int len)
{
	int ret = -ENOTSUPP;
	printk("%s, enter\n", __func__);

	return ret;
}
#endif /* CONFIG_AIC_NAN */

static const struct nla_policy
aicwf_cfg80211_mkeep_alive_policy[MKEEP_ALIVE_ATTRIBUTE_MAX+1] = {
	[0] = {.type = NLA_UNSPEC },
	[MKEEP_ALIVE_ATTRIBUTE_ID]		= { .type = NLA_U8 },
	[MKEEP_ALIVE_ATTRIBUTE_IP_PKT]		= { .type = NLA_MSECS },
	[MKEEP_ALIVE_ATTRIBUTE_IP_PKT_LEN]	= { .type = NLA_U16 },
	[MKEEP_ALIVE_ATTRIBUTE_SRC_MAC_ADDR]	= { .type = NLA_MSECS,
							.len  = ETH_ALEN },
	[MKEEP_ALIVE_ATTRIBUTE_DST_MAC_ADDR]	= { .type = NLA_MSECS,
							.len  = ETH_ALEN },
	[MKEEP_ALIVE_ATTRIBUTE_PERIOD_MSEC]	= { .type = NLA_U32 },
};

static const struct nla_policy
aicwf_cfg80211_logger_policy[LOGGER_ATTRIBUTE_MAX + 1] = {
	[0] = {.type = NLA_UNSPEC },
	[LOGGER_ATTRIBUTE_DRIVER_VER] = { .type = NLA_BINARY },
	[LOGGER_ATTRIBUTE_FW_VER] = { .type = NLA_BINARY },
	[LOGGER_ATTRIBUTE_LOG_LEVEL] = { .type = NLA_U32 },
	[LOGGER_ATTRIBUTE_RING_FLAGS] = { .type = NLA_U32 },
	[LOGGER_ATTRIBUTE_LOG_TIME_INTVAL] = { .type = NLA_U32 },
	[LOGGER_ATTRIBUTE_LOG_MIN_DATA_SIZE] = { .type = NLA_U32 },
	[LOGGER_ATTRIBUTE_RING_NAME] = { .type = NLA_STRING },
};

#ifdef CONFIG_APF
static const struct nla_policy
aicwf_cfg80211_apf_policy[APF_ATTRIBUTE_MAX + 1] = {
	[0] = {.type = NLA_UNSPEC },
	[APF_ATTRIBUTE_VERSION] = {.type = NLA_U32 },
	[APF_ATTRIBUTE_MAX_LEN] = {.type = NLA_U32 },
	[APF_ATTRIBUTE_PROGRAM] = { .type = NLA_BINARY },
	[APF_ATTRIBUTE_PROGRAM_LEN] = { .type = NLA_U32 },
};
#endif

static const struct nla_policy
aicwf_cfg80211_subcmd_policy[GSCAN_ATTRIBUTE_MAX + 1] = {
	[0] = {.type = NLA_UNSPEC },
	[GSCAN_ATTRIBUTE_BAND] = { .type = NLA_U32 },
};

static const struct nla_policy
aicwf_cfg80211_andr_wifi_policy[ANDR_WIFI_ATTRIBUTE_MAX + 1] = {
	[0] = {.type = NLA_UNSPEC },
	[ANDR_WIFI_ATTRIBUTE_COUNTRY] = { .type = NLA_STRING },
	[ANDR_WIFI_ATTRIBUTE_LATENCY_MODE] = { .type = NLA_U32, .len = sizeof(uint32_t) },
};

static const struct nla_policy
aicwf_cfg80211_subcmd_set_mac_policy[WIFI_VENDOR_ATTR_DRIVER_MAX + 1] = {
	[0] = {.type = NLA_UNSPEC },
	[WIFI_VENDOR_ATTR_DRIVER_MAC_ADDR] = { .type = NLA_MSECS, .len  = ETH_ALEN },
};

#ifdef CONFIG_AIC_NAN
static const struct nla_policy
aicwf_cfg80211_nan_attr_policy[NAN_ATTRIBUTE_MAX] = {
	[NAN_ATTRIBUTE_2G_SUPPORT] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_5G_SUPPORT] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_CLUSTER_LOW] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_CLUSTER_HIGH] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_SID_BEACON] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SUB_SID_BEACON] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SYNC_DISC_2G_BEACON] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SYNC_DISC_5G_BEACON] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SDF_2G_SUPPORT] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SDF_5G_SUPPORT] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_HOP_COUNT_LIMIT] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RANDOM_TIME] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_MASTER_PREF] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_OUI] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_WARMUP_TIME] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_CHANNEL] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_24G_CHANNEL] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_5G_CHANNEL] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_CONF_CLUSTER_VAL] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_DWELL_TIME] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SCAN_PERIOD] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_DWELL_TIME_5G] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SCAN_PERIOD_5G] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_AVAIL_BIT_MAP] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_ENTRY_CONTROL] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RSSI_CLOSE] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RSSI_MIDDLE] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RSSI_PROXIMITY] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RSSI_CLOSE_5G] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RSSI_MIDDLE_5G] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RSSI_PROXIMITY_5G] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RSSI_WINDOW_SIZE] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_CIPHER_SUITE_TYPE] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SCID_LEN] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_SCID] = { .type = NLA_BINARY, .len = NAN_MAX_SCID_BUF_LEN },
	[NAN_ATTRIBUTE_2G_AWAKE_DW] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_5G_AWAKE_DW] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_DISC_IND_CFG] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_MAC_ADDR] = { .type = NLA_BINARY, .len = ETHER_ADDR_LEN },
	[NAN_ATTRIBUTE_RANDOMIZATION_INTERVAL] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_CMD_USE_NDPE] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_ENABLE_MERGE] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_DISCOVERY_BEACON_INTERVAL] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_NSS] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_ENABLE_RANGING] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_DW_EARLY_TERM] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_TRANSAC_ID] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_PUBLISH_ID] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_SERVICE_SPECIFIC_INFO_LEN] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_SERVICE_SPECIFIC_INFO] = { .type = NLA_BINARY, .len =
	NAN_MAX_SERVICE_SPECIFIC_INFO_LEN },
	[NAN_ATTRIBUTE_SUBSCRIBE_ID] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_SUBSCRIBE_TYPE] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_PUBLISH_COUNT] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_PUBLISH_TYPE] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_PERIOD] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_TTL] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_SERVICE_NAME_LEN] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_SERVICE_NAME] = { .type = NLA_BINARY, .len = WL_NAN_SVC_HASH_LEN },
	[NAN_ATTRIBUTE_PEER_ID] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_INST_ID] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_SUBSCRIBE_COUNT] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SSIREQUIREDFORMATCHINDICATION] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SUBSCRIBE_MATCH] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_PUBLISH_MATCH] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SERVICERESPONSEFILTER] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SERVICERESPONSEINCLUDE] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_USESERVICERESPONSEFILTER] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RX_MATCH_FILTER_LEN] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_RX_MATCH_FILTER] = { .type = NLA_BINARY, .len = MAX_MATCH_FILTER_LEN },
	[NAN_ATTRIBUTE_TX_MATCH_FILTER_LEN] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_TX_MATCH_FILTER] = { .type = NLA_BINARY, .len = MAX_MATCH_FILTER_LEN },
	[NAN_ATTRIBUTE_MAC_ADDR_LIST_NUM_ENTRIES] = { .type = NLA_U16, .len = sizeof(uint16_t) },
	[NAN_ATTRIBUTE_MAC_ADDR_LIST] = { .type = NLA_BINARY, .len =
	(NAN_SRF_MAX_MAC*ETHER_ADDR_LEN) },
	[NAN_ATTRIBUTE_TX_TYPE] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SDE_CONTROL_CONFIG_DP] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SDE_CONTROL_RANGE_SUPPORT] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SDE_CONTROL_DP_TYPE] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SDE_CONTROL_SECURITY] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RECV_IND_CFG] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_KEY_TYPE] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_KEY_LEN] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_KEY_DATA] = { .type = NLA_BINARY, .len = NAN_MAX_PMK_LEN },
	[NAN_ATTRIBUTE_RSSI_THRESHOLD_FLAG] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_SDEA_SERVICE_SPECIFIC_INFO_LEN] = { .type = NLA_U16, .len =
	sizeof(uint16_t) },
	[NAN_ATTRIBUTE_SDEA_SERVICE_SPECIFIC_INFO] = { .type = NLA_BINARY, .len =
	MAX_SDEA_SVC_INFO_LEN },
	[NAN_ATTRIBUTE_SECURITY] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RANGING_INTERVAL] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_RANGING_NUM_FTM] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RANGING_INGRESS_LIMIT] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_RANGING_EGRESS_LIMIT] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_RANGING_INDICATION] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_SVC_RESPONDER_POLICY] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_NDP_ID] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_IFACE] = { .type = NLA_BINARY, .len = IFNAMSIZ+1 },
	[NAN_ATTRIBUTE_QOS] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_RSP_CODE] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_INST_COUNT] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_PEER_DISC_MAC_ADDR] = { .type = NLA_BINARY, .len = ETHER_ADDR_LEN },
	[NAN_ATTRIBUTE_PEER_NDI_MAC_ADDR] = { .type = NLA_BINARY, .len = ETHER_ADDR_LEN },
	[NAN_ATTRIBUTE_IF_ADDR] = { .type = NLA_BINARY, .len = ETHER_ADDR_LEN },
	[NAN_ATTRIBUTE_NO_CONFIG_AVAIL] = { .type = NLA_U8, .len = sizeof(uint8_t) },
	[NAN_ATTRIBUTE_CHANNEL_INFO] = { .type = NLA_BINARY, .len =
	sizeof(nan_channel_info_t) * NAN_MAX_CHANNEL_INFO_SUPPORTED },
	[NAN_ATTRIBUTE_NUM_CHANNELS] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_INSTANT_MODE_ENABLE] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_INSTANT_COMM_CHAN] = { .type = NLA_U32, .len = sizeof(uint32_t) },
	[NAN_ATTRIBUTE_CHRE_REQUEST] = { .type = NLA_U8, .len = sizeof(uint8_t) },
};
#endif /* CONFIG_AIC_NAN */

static int aicwf_dump_interface(struct wiphy *wiphy,
				struct wireless_dev *wdev, struct sk_buff *skb,
				const void *data, int data_len,
				unsigned long *storage)
{
	return 0;
}

const struct wiphy_vendor_command aicwf_vendor_cmd[] = {
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_OFFLOAD_SUBCMD_START_MKEEP_ALIVE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_start_mkeep_alive,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = aicwf_cfg80211_mkeep_alive_policy,
		.maxattr = MKEEP_ALIVE_ATTRIBUTE_MAX
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_OFFLOAD_SUBCMD_STOP_MKEEP_ALIVE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_stop_mkeep_alive,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = aicwf_cfg80211_mkeep_alive_policy,
		.maxattr = MKEEP_ALIVE_ATTRIBUTE_MAX
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = LOGGER_GET_VER
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_get_ver,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = aicwf_cfg80211_logger_policy,
		.maxattr = LOGGER_ATTRIBUTE_MAX
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = GSCAN_SUBCMD_GET_CHANNEL_LIST
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_subcmd_get_channel_list,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = aicwf_cfg80211_subcmd_policy,
		.maxattr = GSCAN_ATTRIBUTE_MAX
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_SUBCMD_SET_COUNTRY_CODE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_subcmd_set_country_code,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = aicwf_cfg80211_andr_wifi_policy,
		.maxattr = ANDR_WIFI_ATTRIBUTE_MAX
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = LOGGER_TRIGGER_MEM_DUMP
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |  WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_logger_trigger_memory_dump,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = VENDOR_CMD_RAW_DATA,
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_SUBCMD_GET_FEATURE_SET
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |  WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_subcmd_get_feature_set,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = VENDOR_CMD_RAW_DATA,
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = LOGGER_GET_FEATURE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |  WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_logger_get_feature,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = VENDOR_CMD_RAW_DATA,
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = LOGGER_GET_RING_STATUS
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |  WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_logger_get_ring_status,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = VENDOR_CMD_RAW_DATA,
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = LOGGER_START_LOGGING
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |  WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_logger_start_logging,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = aicwf_cfg80211_logger_policy,
		.maxattr = LOGGER_ATTRIBUTE_MAX
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = LOGGER_GET_RING_DATA
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |  WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_logger_get_ring_data,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = aicwf_cfg80211_logger_policy,
		.maxattr = LOGGER_ATTRIBUTE_MAX
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = LOGGER_GET_WAKE_REASON_STATS
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |  WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_logger_get_wake_reason_stats,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = VENDOR_CMD_RAW_DATA,
#endif
	},
#ifdef CONFIG_APF
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = APF_SUBCMD_GET_CAPABILITIES
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |  WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_apf_subcmd_get_capabilities,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = aicwf_cfg80211_apf_policy,
		.maxattr = APF_ATTRIBUTE_MAX,
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = APF_SUBCMD_SET_FILTER
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_apf_set_filter,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = aicwf_cfg80211_apf_policy,
		.maxattr = APF_ATTRIBUTE_MAX,
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = APF_SUBCMD_READ_FILTER_DATA
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_apf_read_filter_data,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_apf_policy,
		.maxattr = APF_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
#endif
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = VENDOR_NL80211_SUBCMD_SET_MAC
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = aicwf_vendor_sub_cmd_set_mac,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = aicwf_cfg80211_subcmd_set_mac_policy,
		.maxattr = WIFI_VENDOR_ATTR_DRIVER_MAX,
#endif
	},
	{
		{
			.vendor_id = BRCM_OUI,
			.subcmd = VENDOR_NL80211_SUBCMD_SET_MAC
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = aicwf_vendor_sub_cmd_set_mac,
		.dumpit = aicwf_dump_interface,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
		.policy = aicwf_cfg80211_subcmd_set_mac_policy,
		.maxattr = WIFI_VENDOR_ATTR_DRIVER_MAX,
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_SUBCMD_SET_LATENCY_MODE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_subcmd_set_latency_mode,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_andr_wifi_policy,
		.maxattr = ANDR_WIFI_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = LOGGER_START_PKT_FATE_MONITORING
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_logger_start_pkt_fate_monitoring,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = VENDOR_CMD_RAW_DATA,
#endif
	},
#ifdef CONFIG_AIC_NAN
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_ENABLE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_enable,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_DISABLE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_disable,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_CONFIG
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_config,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_REQUEST_PUBLISH
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_req_publish,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_REQUEST_SUBSCRIBE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_req_subscribe,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_CANCEL_PUBLISH
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_cancel_publish,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_CANCEL_SUBSCRIBE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_cancel_subscribe,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
#if 0
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_TRANSMIT
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_transmit,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
#endif
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_GET_CAPABILITIES
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_get_capabilities,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = VENDOR_CMD_RAW_DATA,
#endif
	},
#if 0
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_DATA_PATH_IFACE_CREATE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_data_path_iface_create,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
#endif
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_DATA_PATH_IFACE_DELETE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_data_path_iface_delete,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
#if 0
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_DATA_PATH_REQUEST
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_data_path_request,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_DATA_PATH_RESPONSE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_data_path_response,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_DATA_PATH_END
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_data_path_end,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
#ifdef WL_NAN_DISC_CACHE
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_DATA_PATH_SEC_INFO
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_data_path_sec_info,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
#endif /* WL_NAN_DISC_CACHE */
#endif
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_VERSION_INFO
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_version_info,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = VENDOR_CMD_RAW_DATA,
#endif
	},
#if 0
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = NAN_WIFI_SUBCMD_ENABLE_MERGE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV | WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = aicwf_vendor_nan_wifi_subcmd_enable_merge,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
		.policy = aicwf_cfg80211_nan_attr_policy,
		.maxattr = NAN_ATTRIBUTE_MAX
#endif /* LINUX_VERSION >= 5.3 */
	},
#endif
#endif /* CONFIG_AIC_NAN */
};

static const struct nl80211_vendor_cmd_info aicwf_vendor_events[] = {
};
#endif

int aicwf_vendor_init(struct wiphy *wiphy)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
	wiphy->vendor_commands = aicwf_vendor_cmd;
	wiphy->n_vendor_commands = ARRAY_SIZE(aicwf_vendor_cmd);
	wiphy->vendor_events = aicwf_vendor_events;
	wiphy->n_vendor_events = ARRAY_SIZE(aicwf_vendor_events);
#endif
	return 0;
}
