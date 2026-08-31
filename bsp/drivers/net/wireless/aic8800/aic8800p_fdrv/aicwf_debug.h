#ifndef AICWF_DEBUG_H
#define AICWF_DEBUG_H

#define RWNX_FN_ENTRY_STR ">>> %s()\n", __func__



/* message levels */
#define LOGERROR        0x0001
#define LOGINFO         0x0002
#define LOGTRACE        0x0004
#define LOGDEBUG        0x0008
#define LOGDATA         0x0010
#define LOGSTEER        0x0020
#define LOGVERBOS       0x0040

extern int aicwf_dbg_level;
void rwnx_data_dump(char *tag, void *data, unsigned long len);

#define AICWF_LOG		"AICWF"

static inline const char *level_str(uint32_t level)
{
	switch (level) {
	case LOGERROR:  return "E";
	case LOGINFO:   return "I";
	case LOGTRACE:  return "T";
	case LOGDEBUG:  return "D";
	case LOGDATA:   return "A";
	case LOGSTEER:  return "S";
	case LOGVERBOS: return "V";
	default: return "U";
	}
}

#define AICWFDBG(level, args, arg...)	\
do {	\
	if (aicwf_dbg_level & level) {	\
		printk(AICWF_LOG "-%s: " args, level_str(level), ##arg); \
	}	\
} while (0)

#define RWNX_DBG(fmt, ...)	\
do {	\
	if (aicwf_dbg_level & LOGTRACE) {	\
		printk(AICWF_LOG "-T: " fmt, ##__VA_ARGS__); \
	}	\
} while (0)

#if 0
#define RWNX_DBG(fmt, ...)	\
	do {	\
		if (aicwf_dbg_level & LOGTRACE) {	\
			printk(AICWF_LOG"LOGTRACE"")\t" fmt, ##__VA_ARGS__); \
		}	\
	} while (0)
#define AICWFDBG(args, level)	\
do {	\
	if (aicwf_dbg_level & level) {	\
		printk(AICWF_LOG"(%s)\t", #level);	\
		printf args;	\
	}	\
} while (0)
#endif

#endif/* AICWF_DEBUG_H */
