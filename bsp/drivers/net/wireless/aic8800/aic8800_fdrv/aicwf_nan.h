#ifndef _AICWF_NAN_H_
#define _AICWF_NAN_H_

#include <linux/types.h>

/* NAN structs versioning b/w DHD and HAL
* define new version if any change in any of the shared structs
*/
#define NAN_HAL_VERSION_1                   0x2

#define NAN_EVENT_BUFFER_SIZE_LARGE         1024u

#define NAN_RANGE_EXT_CANCEL_SUPPORT_VER    2
#define WL_NAN_IOV_BATCH_VERSION            0x8000
#define WL_NAN_AVAIL_REPEAT_INTVL           0x0200
#define WL_NAN_AVAIL_START_INTVL            160
#define WL_NAN_AVAIL_DURATION_INTVL         336
#define NAN_IOCTL_BUF_SIZE                  256u
#define NAN_IOCTL_BUF_SIZE_MED              512u
#define NAN_IOCTL_BUF_SIZE_LARGE            1024u
#define NAN_EVENT_NAME_MAX_LEN              40u
#define NAN_RTT_IOVAR_BUF_SIZE              1024u
#define WL_NAN_EVENT_CLEAR_BIT              32
#define NAN_EVENT_MASK_ALL                  0x7fffffff
#define NAN_MAX_AWAKE_DW_INTERVAL           5
#define NAN_MAXIMUM_ID_NUMBER               255
#define NAN_MAXIMUM_MASTER_PREFERENCE       254
#define NAN_ID_RESERVED                     0
#define NAN_ID_MIN                          1
#define NAN_ID_MAX                          255
#define NAN_DEF_SOCIAL_CHAN_2G              6
#define NAN_DEF_SOCIAL_CHAN_5G              149
#define NAN_DEF_SEC_SOCIAL_CHAN_5G          44
#define NAN_MAX_SOCIAL_CHANNELS             3
/* Keeping RSSI threshold value to be -70dBm */
#define NAN_DEF_RSSI_NOTIF_THRESH           -70
/* Keeping default RSSI mid value to be -70dBm */
#define NAN_DEF_RSSI_MID                    -75
/* Keeping default RSSI close value to be -60dBm */
#define NAN_DEF_RSSI_CLOSE                  -60
#define WL_AVAIL_BIT_MAP    "1111111111111111111111111111111100000000000000000000000000000000"
#define WL_5G_AVAIL_BIT_MAP    "0000000011111111111111111111111111111111000000000000000000000000"
#define WL_AVAIL_CHANNEL_2G                 6
#define WL_AVAIL_BANDWIDTH_2G               WL_CHANSPEC_BW_20
#define WL_AVAIL_CHANNEL_5G                 149
#define WL_AVAIL_BANDWIDTH_5G               WL_CHANSPEC_BW_80
#define NAN_RANGING_PERIOD                  WL_AVAIL_PERIOD_1024
#define NAN_SYNC_DEF_AWAKE_DW               1
#define NAN_RNG_TERM_FLAG_NONE              0

#define NAN_BLOOM_LENGTH_DEFAULT            240u
#define NAN_SRF_MAX_MAC                     (NAN_BLOOM_LENGTH_DEFAULT / ETHER_ADDR_LEN)
#define NAN_SRF_CTRL_FIELD_LEN              1u

#define MAX_IF_ADD_WAIT_TIME                1000
#define NAN_DP_ROLE_INITIATOR               0x0001
#define NAN_DP_ROLE_RESPONDER               0x0002

#define WL_NAN_OBUF_DATA_OFFSET    (OFFSETOF(bcm_iov_batch_buf_t, cmds[0]) + \
		OFFSETOF(bcm_iov_batch_subcmd_t, data[0]))
#define NAN_INVALID_ROLE(role)    (role > WL_NAN_ROLE_ANCHOR_MASTER)
#define NAN_INVALID_CHANSPEC(chanspec)    ((chanspec == INVCHANSPEC) || \
	(chanspec == 0))
#define NAN_INVALID_EVENT(num)    ((num < WL_NAN_EVENT_START) || \
	(num >= WL_NAN_EVENT_INVALID))
#define NAN_INVALID_PROXD_EVENT(num)    (num != WLC_E_PROXD_NAN_EVENT)
#define NAN_EVENT_BIT(event)    (1U << (event - WL_NAN_EVENT_START))
#define NAN_EVENT_MAP(event)    ((event) - WL_NAN_EVENT_START)
#define NAME_TO_STR(name)    #name
#define NAN_ID_CTRL_SIZE    ((NAN_MAXIMUM_ID_NUMBER/8) + 1)

#ifndef tolower
#define tolower(c)    bcm_tolower(c)
#endif /* tolower */

#define NMR2STR(a)    ((a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5], (a)[6], (a)[7])
#define NMRSTR    "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x"

#define NAN_DBG_ENTER()    {WL_DBG(("Enter\n")); }
#define NAN_DBG_EXIT()     {WL_DBG(("Exit\n")); }

/* Service Control Type length */
#define NAN_SVC_CONTROL_TYPE_MASK    ((1 << NAN_SVC_CONTROL_TYPE_LEN) - 1)

#ifndef strtoul
#define strtoul(nptr, endptr, base)    bcm_strtoul((nptr), (endptr), (base))
#endif

#define NAN_MAC_ADDR_LEN           6u
#define NAN_DP_MAX_APP_INFO_LEN    512u

#define NAN_SDE_CF_DP_REQUIRED          (1 << 2)
#define NAN_SDE_CF_DP_TYPE              (1 << 3)
#define NAN_SDE_CF_MULTICAST_TYPE       (1 << 4)
#define NAN_SDE_CF_SECURITY_REQUIRED    (1 << 6)
#define NAN_SDE_CF_RANGING_REQUIRED     (1 << 7)
#define NAN_SDE_CF_RANGE_PRESENT        (1 << 8)

#define CHECK_BIT(m, n)    (((m >> n) & 1) ? 1 : 0)

#define WL_NAN_EVENT_DIC_MAC_ADDR_BIT    0
#define WL_NAN_EVENT_START_EVENT         1
#define WL_NAN_EVENT_JOIN_EVENT          2

/* Disabling svc specific(as per part of sub & pub calls) events based on below bits */
#define WL_NAN_EVENT_SUPPRESS_TERMINATE_BIT    0
#define WL_NAN_EVENT_SUPPRESS_MATCH_EXP_BIT    1
#define WL_NAN_EVENT_SUPPRESS_RECEIVE_BIT      2
#define WL_NAN_EVENT_SUPPRESS_REPLIED_BIT      3

/* Disabling tranmsit followup events based on below bit */
#define WL_NAN_EVENT_SUPPRESS_FOLLOWUP_RECEIVE_BIT    0

#define C2S(x)    (case x: id2str = #x)
#define NAN_BLOOM_LENGTH_DEFAULT             240u
#define NAN_SRF_MAX_MAC                      (NAN_BLOOM_LENGTH_DEFAULT / ETHER_ADDR_LEN)
#define NAN_MAX_PMK_LEN                      32u
#define NAN_ERROR_STR_LEN                    255u
#define NAN_MAX_SCID_BUF_LEN                 1024u

/* NAN related Capabilities */
#define MAX_CONCURRENT_NAN_CLUSTERS          1u
#define MAX_PUBLISHES                        8u
#define MAX_SUBSCRIBES                       8u
#define MAX_SVC_NAME_LEN                     255u
#define MAX_MATCH_FILTER_LEN                 255u
#define MAX_TOTAL_MATCH_FILTER_LEN           510u
#define NAN_MAX_SERVICE_SPECIFIC_INFO_LEN    255u
#define NAN_MAX_NDI                          3u
#define MAX_NDP_SESSIONS                     5u
#define MAX_APP_INFO_LEN                     255u
#define MAX_QUEUED_TX_FOLLOUP_MSGS           10u
#define MAX_SDEA_SVC_INFO_LEN                255u
#define MAX_SUBSCRIBE_ADDRESS                10u
#define CIPHER_SUITE_SUPPORTED               1u
#define MAX_SCID_LEN                         0u
#define IS_NDP_SECURITY_SUPPORTED            true
#define NDP_SUPPORTED_BANDS                  2u
#define NAN_MAX_RANGING_INST                 8u
#define NAN_MAX_RANGING_SSN_ALLOWED          1u
#define NAN_MAX_SVC_INST                     (MAX_PUBLISHES + MAX_SUBSCRIBES)
#define NAN_SVC_INST_SIZE                    32u
#define NAN_START_STOP_TIMEOUT               5000u
#define NAN_MAX_NDP_PEER                     8u
#define NAN_DISABLE_CMD_DELAY                530u
#define NAN_WAKELOCK_TIMEOUT                 (NAN_DISABLE_CMD_DELAY + 100u)

#define NAN_NMI_RAND_PVT_CMD_VENDOR          (1 << 31)
#define NAN_NMI_RAND_CLUSTER_MERGE_ENAB      (1 << 30)
#define NAN_NMI_RAND_AUTODAM_LWT_MODE_ENAB   (1 << 29)
#define NAN_NMI_RAND_INTVL_MASK    ~(NAN_NMI_RAND_PVT_CMD_VENDOR | \
						NAN_NMI_RAND_CLUSTER_MERGE_ENAB | \
						NAN_NMI_RAND_AUTODAM_LWT_MODE_ENAB)

#ifdef WL_NAN_DEBUG
#define NAN_MUTEX_LOCK()    {WL_DBG(("Mutex Lock: Enter: %s\n", __FUNCTION__)); \
	mutex_lock(&cfg->nancfg->nan_sync); }
#define NAN_MUTEX_UNLOCK()    {mutex_unlock(&cfg->nancfg->nan_sync); \
	WL_DBG(("Mutex Unlock: Exit: %s\n", __FUNCTION__)); }
#else
#define NAN_MUTEX_LOCK()      {mutex_lock(&cfg->nancfg->nan_sync); }
#define NAN_MUTEX_UNLOCK()    {mutex_unlock(&cfg->nancfg->nan_sync); }
#endif /* WL_NAN_DEBUG */
#define NAN_ATTR_SUPPORT_2G_CONFIG             (1<<0)
#define NAN_ATTR_SYNC_DISC_2G_BEACON_CONFIG    (1<<1)
#define NAN_ATTR_SDF_2G_SUPPORT_CONFIG         (1<<2)
#define NAN_ATTR_SUPPORT_5G_CONFIG             (1<<3)
#define NAN_ATTR_SYNC_DISC_5G_BEACON_CONFIG    (1<<4)
#define NAN_ATTR_SDF_5G_SUPPORT_CONFIG         (1<<5)
#define NAN_ATTR_2G_DW_CONFIG                  (1<<6)
#define NAN_ATTR_5G_DW_CONFIG                  (1<<7)
#define NAN_ATTR_2G_CHAN_CONFIG                (1<<8)
#define NAN_ATTR_5G_CHAN_CONFIG                (1<<9)
#define NAN_ATTR_2G_DWELL_TIME_CONFIG          (1<<10)
#define NAN_ATTR_5G_DWELL_TIME_CONFIG          (1<<11)
#define NAN_ATTR_2G_SCAN_PERIOD_CONFIG         (1<<12)
#define NAN_ATTR_5G_SCAN_PERIOD_CONFIG         (1<<13)
#define NAN_ATTR_RSSI_CLOSE_CONFIG             (1<<14)
#define NAN_ATTR_RSSI_MIDDLE_2G_CONFIG         (1<<15)
#define NAN_ATTR_RSSI_PROXIMITY_2G_CONFIG      (1<<16)
#define NAN_ATTR_RSSI_CLOSE_5G_CONFIG          (1<<17)
#define NAN_ATTR_RSSI_MIDDLE_5G_CONFIG         (1<<18)
#define NAN_ATTR_RSSI_PROXIMITY_5G_CONFIG      (1<<19)
#define NAN_ATTR_RSSI_WINDOW_SIZE_CONFIG       (1<<20)
#define NAN_ATTR_HOP_COUNT_LIMIT_CONFIG        (1<<21)
#define NAN_ATTR_SID_BEACON_CONFIG             (1<<22)
#define NAN_ATTR_HOP_COUNT_FORCE_CONFIG        (1<<23)
#define NAN_ATTR_RAND_FACTOR_CONFIG            (1<<24)
#define NAN_ATTR_CLUSTER_VAL_CONFIG            (1<<25)
#define NAN_ATTR_IF_ADDR_CONFIG                (1<<26)
#define NAN_ATTR_OUI_CONFIG                    (1<<27)
#define NAN_ATTR_SUB_SID_BEACON_CONFIG         (1<<28)
#define NAN_ATTR_DISC_BEACON_INTERVAL          (1<<29)
#define NAN_ATTR_INSTANT_MODE_CONFIG           (1<<30)

#define NAN_IOVAR_NAME_SIZE                    4u
#define NAN_XTLV_ID_LEN_SIZE                   OFFSETOF(bcm_xtlv_t, data)
#define NAN_RANGING_INDICATE_CONTINUOUS_MASK   0x01
#define NAN_RANGE_REQ_CMD                      0
#define NAN_RNG_REQ_ACCEPTED_BY_HOST           1
#define NAN_RNG_REQ_REJECTED_BY_HOST           0

#define NAN_RNG_REQ_ACCEPTED_BY_PEER           0
#define NAN_RNG_REQ_REJECTED_BY_PEER           1

#define NAN_RNG_GEOFENCE_MAX_RETRY_CNT         3u

#define NAN_MAX_CHANNEL_INFO_SUPPORTED         4u


/* Capabilities info supported by FW */
typedef struct nan_hal_capabilities {
	u32 max_concurrent_nan_clusters;
	u32 max_publishes;
	u32 max_subscribes;
	u32 max_service_name_len;
	u32 max_match_filter_len;
	u32 max_total_match_filter_len;
	u32 max_service_specific_info_len;
	u32 max_vsa_data_len;
	u32 max_mesh_data_len;
	u32 max_ndi_interfaces;
	u32 max_ndp_sessions;
	u32 max_app_info_len;
	u32 max_queued_transmit_followup_msgs;
	u32 ndp_supported_bands;
	u32 cipher_suites_supported;
	u32 max_scid_len;
	bool is_ndp_security_supported;
	u32 max_sdea_service_specific_info_len;
	u32 max_subscribe_address;
	u32 ndpe_attr_supported;
	bool is_instant_mode_supported;
} nan_hal_capabilities_t;

#define WL_NAN_SVC_HASH_LEN 6
typedef struct _nan_hal_resp {
	u16 instance_id;
	u16 subcmd;
	s32 status;
	s32 value;
	/* Identifier for the instance of the NDP */
	u16 ndp_instance_id;
	/* Publisher NMI */
	u8 pub_nmi[NAN_MAC_ADDR_LEN];
	/* SVC_HASH */
	u8 svc_hash[WL_NAN_SVC_HASH_LEN];
	char nan_reason[NAN_ERROR_STR_LEN]; /* Describe the NAN reason type */
	char pad[3];
	nan_hal_capabilities_t capabilities;
} nan_hal_resp_t;

typedef struct nan_channel_info {
	u32 channel;
	u32 bandwidth;
	u32 nss;
} nan_channel_info_t;

typedef struct nan_ndl_sched_info {
	u32 num_channels;
	nan_channel_info_t channel_info[NAN_MAX_CHANNEL_INFO_SUPPORTED];
} nan_ndl_sched_info_t;

typedef enum {
	NAN_ATTRIBUTE_INVALID                           = 0,
	NAN_ATTRIBUTE_HEADER                            = 100,
	NAN_ATTRIBUTE_HANDLE                            = 101,
	NAN_ATTRIBUTE_TRANSAC_ID                        = 102,

	/* NAN Enable request attributes */
	NAN_ATTRIBUTE_2G_SUPPORT                        = 103,
	NAN_ATTRIBUTE_5G_SUPPORT                        = 104,
	NAN_ATTRIBUTE_CLUSTER_LOW                       = 105,
	NAN_ATTRIBUTE_CLUSTER_HIGH                      = 106,
	NAN_ATTRIBUTE_SID_BEACON                        = 107,
	NAN_ATTRIBUTE_SYNC_DISC_2G_BEACON               = 108,
	NAN_ATTRIBUTE_SYNC_DISC_5G_BEACON               = 109,
	NAN_ATTRIBUTE_SDF_2G_SUPPORT                    = 110,
	NAN_ATTRIBUTE_SDF_5G_SUPPORT                    = 111,
	NAN_ATTRIBUTE_RSSI_CLOSE                        = 112,
	NAN_ATTRIBUTE_RSSI_MIDDLE                       = 113,
	NAN_ATTRIBUTE_RSSI_PROXIMITY                    = 114,
	NAN_ATTRIBUTE_HOP_COUNT_LIMIT                   = 115,
	NAN_ATTRIBUTE_RANDOM_TIME                       = 116,
	NAN_ATTRIBUTE_MASTER_PREF                       = 117,
	NAN_ATTRIBUTE_PERIODIC_SCAN_INTERVAL            = 118,

	/* Nan Publish/Subscribe request attributes */
	NAN_ATTRIBUTE_PUBLISH_ID                        = 119,
	NAN_ATTRIBUTE_TTL                               = 120,
	NAN_ATTRIBUTE_PERIOD                            = 121,
	NAN_ATTRIBUTE_REPLIED_EVENT_FLAG                = 122,
	NAN_ATTRIBUTE_PUBLISH_TYPE                      = 123,
	NAN_ATTRIBUTE_TX_TYPE                           = 124,
	NAN_ATTRIBUTE_PUBLISH_COUNT                     = 125,
	NAN_ATTRIBUTE_SERVICE_NAME_LEN                  = 126,
	NAN_ATTRIBUTE_SERVICE_NAME                      = 127,
	NAN_ATTRIBUTE_SERVICE_SPECIFIC_INFO_LEN         = 128,
	NAN_ATTRIBUTE_SERVICE_SPECIFIC_INFO             = 129,
	NAN_ATTRIBUTE_RX_MATCH_FILTER_LEN               = 130,
	NAN_ATTRIBUTE_RX_MATCH_FILTER                   = 131,
	NAN_ATTRIBUTE_TX_MATCH_FILTER_LEN               = 132,
	NAN_ATTRIBUTE_TX_MATCH_FILTER                   = 133,
	NAN_ATTRIBUTE_SUBSCRIBE_ID                      = 134,
	NAN_ATTRIBUTE_SUBSCRIBE_TYPE                    = 135,
	NAN_ATTRIBUTE_SERVICERESPONSEFILTER             = 136,
	NAN_ATTRIBUTE_SERVICERESPONSEINCLUDE            = 137,
	NAN_ATTRIBUTE_USESERVICERESPONSEFILTER          = 138,
	NAN_ATTRIBUTE_SSIREQUIREDFORMATCHINDICATION     = 139,
	NAN_ATTRIBUTE_SUBSCRIBE_MATCH                   = 140,
	NAN_ATTRIBUTE_SUBSCRIBE_COUNT                   = 141,
	NAN_ATTRIBUTE_MAC_ADDR                          = 142,
	NAN_ATTRIBUTE_MAC_ADDR_LIST                     = 143,
	NAN_ATTRIBUTE_MAC_ADDR_LIST_NUM_ENTRIES         = 144,
	NAN_ATTRIBUTE_PUBLISH_MATCH                     = 145,

	/* Nan Event attributes */
	NAN_ATTRIBUTE_ENABLE_STATUS                     = 146,
	NAN_ATTRIBUTE_JOIN_STATUS                       = 147,
	NAN_ATTRIBUTE_ROLE                              = 148,
	NAN_ATTRIBUTE_MASTER_RANK                       = 149,
	NAN_ATTRIBUTE_ANCHOR_MASTER_RANK                = 150,
	NAN_ATTRIBUTE_CNT_PEND_TXFRM                    = 151,
	NAN_ATTRIBUTE_CNT_BCN_TX                        = 152,
	NAN_ATTRIBUTE_CNT_BCN_RX                        = 153,
	NAN_ATTRIBUTE_CNT_SVC_DISC_TX                   = 154,
	NAN_ATTRIBUTE_CNT_SVC_DISC_RX                   = 155,
	NAN_ATTRIBUTE_AMBTT                             = 156,
	NAN_ATTRIBUTE_CLUSTER_ID                        = 157,
	NAN_ATTRIBUTE_INST_ID                           = 158,
	NAN_ATTRIBUTE_OUI                               = 159,
	NAN_ATTRIBUTE_STATUS                            = 160,
	NAN_ATTRIBUTE_DE_EVENT_TYPE                     = 161,
	NAN_ATTRIBUTE_MERGE                             = 162,
	NAN_ATTRIBUTE_IFACE                             = 163,
	NAN_ATTRIBUTE_CHANNEL                           = 164,
	NAN_ATTRIBUTE_PEER_ID                           = 165,
	NAN_ATTRIBUTE_NDP_ID                            = 167,
	NAN_ATTRIBUTE_SECURITY                          = 168,
	NAN_ATTRIBUTE_QOS                               = 169,
	NAN_ATTRIBUTE_RSP_CODE                          = 170,
	NAN_ATTRIBUTE_INST_COUNT                        = 171,
	NAN_ATTRIBUTE_PEER_DISC_MAC_ADDR                = 172,
	NAN_ATTRIBUTE_PEER_NDI_MAC_ADDR                 = 173,
	NAN_ATTRIBUTE_IF_ADDR                           = 174,
	NAN_ATTRIBUTE_WARMUP_TIME                       = 175,
	NAN_ATTRIBUTE_RECV_IND_CFG                      = 176,
	NAN_ATTRIBUTE_RSSI_CLOSE_5G                     = 177,
	NAN_ATTRIBUTE_RSSI_MIDDLE_5G                    = 178,
	NAN_ATTRIBUTE_RSSI_PROXIMITY_5G                 = 179,
	NAN_ATTRIBUTE_CONNMAP                           = 180,
	NAN_ATTRIBUTE_24G_CHANNEL                       = 181,
	NAN_ATTRIBUTE_5G_CHANNEL                        = 182,
	NAN_ATTRIBUTE_DWELL_TIME                        = 183,
	NAN_ATTRIBUTE_SCAN_PERIOD                       = 184,
	NAN_ATTRIBUTE_RSSI_WINDOW_SIZE                  = 185,
	NAN_ATTRIBUTE_CONF_CLUSTER_VAL                  = 186,
	NAN_ATTRIBUTE_AVAIL_BIT_MAP                     = 187,
	NAN_ATTRIBUTE_ENTRY_CONTROL                     = 188,
	NAN_ATTRIBUTE_CIPHER_SUITE_TYPE                 = 189,
	NAN_ATTRIBUTE_KEY_TYPE                          = 190,
	NAN_ATTRIBUTE_KEY_LEN                           = 191,
	NAN_ATTRIBUTE_SCID                              = 192,
	NAN_ATTRIBUTE_SCID_LEN                          = 193,
	NAN_ATTRIBUTE_SDE_CONTROL_CONFIG_DP             = 194,
	NAN_ATTRIBUTE_SDE_CONTROL_SECURITY              = 195,
	NAN_ATTRIBUTE_SDE_CONTROL_DP_TYPE               = 196,
	NAN_ATTRIBUTE_SDE_CONTROL_RANGE_SUPPORT         = 197,
	NAN_ATTRIBUTE_NO_CONFIG_AVAIL                   = 198,
	NAN_ATTRIBUTE_2G_AWAKE_DW                       = 199,
	NAN_ATTRIBUTE_5G_AWAKE_DW                       = 200,
	NAN_ATTRIBUTE_RANGING_INTERVAL                  = 201,
	NAN_ATTRIBUTE_RANGING_INDICATION                = 202,
	NAN_ATTRIBUTE_RANGING_INGRESS_LIMIT             = 203,
	NAN_ATTRIBUTE_RANGING_EGRESS_LIMIT              = 204,
	NAN_ATTRIBUTE_RANGING_AUTO_ACCEPT               = 205,
	NAN_ATTRIBUTE_RANGING_RESULT                    = 206,
	NAN_ATTRIBUTE_DISC_IND_CFG                      = 207,
	NAN_ATTRIBUTE_RSSI_THRESHOLD_FLAG               = 208,
	NAN_ATTRIBUTE_KEY_DATA                          = 209,
	NAN_ATTRIBUTE_SDEA_SERVICE_SPECIFIC_INFO_LEN    = 210,
	NAN_ATTRIBUTE_SDEA_SERVICE_SPECIFIC_INFO        = 211,
	NAN_ATTRIBUTE_REASON                            = 212,
	NAN_ATTRIBUTE_DWELL_TIME_5G                     = 215,
	NAN_ATTRIBUTE_SCAN_PERIOD_5G                    = 216,
	NAN_ATTRIBUTE_SVC_RESPONDER_POLICY              = 217,
	NAN_ATTRIBUTE_EVENT_MASK                        = 218,
	NAN_ATTRIBUTE_SUB_SID_BEACON                    = 219,
	NAN_ATTRIBUTE_RANDOMIZATION_INTERVAL            = 220,
	NAN_ATTRIBUTE_CMD_RESP_DATA                     = 221,
	NAN_ATTRIBUTE_CMD_USE_NDPE                      = 222,
	NAN_ATTRIBUTE_ENABLE_MERGE                      = 223,
	NAN_ATTRIBUTE_DISCOVERY_BEACON_INTERVAL         = 224,
	NAN_ATTRIBUTE_NSS                               = 225,
	NAN_ATTRIBUTE_ENABLE_RANGING                    = 226,
	NAN_ATTRIBUTE_DW_EARLY_TERM                     = 227,
	NAN_ATTRIBUTE_CHANNEL_INFO                      = 228,
	NAN_ATTRIBUTE_NUM_CHANNELS                      = 229,
	NAN_ATTRIBUTE_INSTANT_MODE_ENABLE               = 230,
	NAN_ATTRIBUTE_INSTANT_COMM_CHAN                 = 231,
	NAN_ATTRIBUTE_CHRE_REQUEST                      = 232,
	NAN_ATTRIBUTE_SVC_CFG_SUSPENDABLE               = 233,
	NAN_ATTRIBUTE_RANGING_NUM_FTM                   = 234,
	NAN_ATTRIBUTE_MAX                               = 235
} NAN_ATTRIBUTE;

#endif
