# SPDX-License-Identifier: GPL-2.0-or-later
#
# Allwinner audio codecs driver Makefile
#
# Copyright(c) 2022-2027 Allwinnertech Co., Ltd.

ccflags-y += -I $(srctree)/bsp/drivers/sound/platform/
ccflags-y += -I $(srctree)/bsp/drivers/sound/adapter/
ccflags-y += -I $(srctree)/bsp/drivers/sound/common/

# codec -> hdmi codec
snd_soc_sunxi_codec_hdmi-objs		+= snd_sunxi_codec_hdmi.o
obj-$(CONFIG_SND_SOC_SUNXI_CODEC_HDMI)	+= snd_soc_sunxi_codec_hdmi.o

snd_soc_sunxi_codec_hdmi_2-objs			+= snd_sunxi_codec_hdmi_2.o
obj-$(CONFIG_SND_SOC_SUNXI_CODEC_HDMI_2)	+= snd_soc_sunxi_codec_hdmi_2.o

# codec -> edp codec
snd_soc_sunxi_codec_edp-objs		+= snd_sunxi_codec_edp.o
obj-$(CONFIG_SND_SOC_SUNXI_CODEC_EDP)	+= snd_soc_sunxi_codec_edp.o

# codec -> av codec
snd_soc_sunxi_codec_av-objs		+= snd_sunxi_codec_av.o
obj-$(CONFIG_SND_SOC_SUNXI_CODEC_AV)	+= snd_soc_sunxi_codec_av.o

# codec -> ac101
snd_soc_ac101-objs			+= ac101.o
obj-$(CONFIG_SND_SOC_AC101)		+= snd_soc_ac101.o

# codec -> ac101b
snd_soc_ac101b-objs			+= ac101b.o
obj-$(CONFIG_SND_SOC_AC101B)		+= snd_soc_ac101b.o

# codec -> ac107
snd-soc-ac107-objs			+=ac107.o
obj-$(CONFIG_SND_SOC_AC107)		+= snd-soc-ac107.o

# codec -> ac108
snd-soc-ac108-objs			+= ac108.o
obj-$(CONFIG_SND_SOC_AC108)		+= snd-soc-ac108.o

# codec -> Fsm
snd-soc-fsm-objs			+= fsm/fsm_core.o fsm/fsm_i2c.o fsm/fs16xx/fs1603.o
obj-$(CONFIG_SND_SOC_FS16XX)		+= snd-soc-fsm.o

# codec -> es8323
snd_soc_es8323-objs := es8323.o
obj-$(CONFIG_SND_SOC_ES8323)	+= snd_soc_es8323.o