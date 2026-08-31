# SPDX-License-Identifier: GPL-2.0-or-later
#
# Allwinner audio ahub driver Makefile
#
# Copyright(c) 2022-2027 Allwinnertech Co., Ltd.

ccflags-y	+= -I $(srctree)/bsp/drivers/sound/pa/
ccflags-y	+= -I $(srctree)/bsp/drivers/sound/adapter/
ccflags-y	+= -I $(srctree)/bsp/drivers/sound/common/
ADPT_DIR	:= ../adapter
COMM_DIR	:= ../common
PLATS_DIR	:= ./platforms

# common
snd_soc_sunxi_ahub_common-objs		+= $(ADPT_DIR)/snd_sunxi_adapter.o
snd_soc_sunxi_ahub_common-objs		+= $(ADPT_DIR)/snd_sunxi_pcm_adapter.o
snd_soc_sunxi_ahub_common-objs		+= $(COMM_DIR)/snd_sunxi_rxsync.o
snd_soc_sunxi_ahub_common-objs		+= $(COMM_DIR)/snd_sunxi_jack_codec.o
snd_soc_sunxi_ahub_common-objs		+= $(COMM_DIR)/snd_sunxi_jack_extcon.o
snd_soc_sunxi_ahub_common-objs		+= $(COMM_DIR)/snd_sunxi_jack_gpio.o
snd_soc_sunxi_ahub_common-objs		+= $(COMM_DIR)/snd_sunxi_jack_advance.o
snd_soc_sunxi_ahub_common-objs		+= $(COMM_DIR)/snd_sunxi_jack.o
snd_soc_sunxi_ahub_common-objs		+= $(COMM_DIR)/snd_sunxi_common_utils.o
snd_soc_sunxi_ahub_common-objs		+= $(COMM_DIR)/snd_sunxi_common.o
snd_soc_sunxi_ahub_common-objs		+= $(COMM_DIR)/snd_sunxi_pcm.o
obj-$(CONFIG_SND_SOC_SUNXI_AHUB_COMMON)	+= snd_soc_sunxi_ahub_common.o

# mgmt
ifneq (, $(filter y m, $(CONFIG_ARCH_SUN60IW3)))
snd_soc_sunxi_ahub_mgmt-objs		+= $(PLATS_DIR)/snd_sun60iw3_ahub.o
endif
snd_soc_sunxi_ahub_mgmt-objs		+= snd_sunxi_ahub_mgmt.o
obj-$(CONFIG_SND_SOC_SUNXI_AHUB_MGMT)	+= snd_soc_sunxi_ahub_mgmt.o

# module
# AAUDIO
snd_soc_sunxi_ahub_aaudio-objs		+= snd_sunxi_ahub_aaudio.o
obj-$(CONFIG_SND_SOC_SUNXI_AHUB_AAUDIO)	+= snd_soc_sunxi_ahub_aaudio.o

# AC203C
ifneq (, $(filter y m, $(CONFIG_ARCH_SUN60IW3)))
snd_soc_sunxi_ahub_ac203c-objs		+= $(PLATS_DIR)/snd_sun60iw3_ac203c.o
endif
snd_soc_sunxi_ahub_ac203c-objs		+= snd_sunxi_ahub_ac203c.o
obj-$(CONFIG_SND_SOC_SUNXI_AHUB_AC203C)	+= snd_soc_sunxi_ahub_ac203c.o

# I2S
ifneq (, $(filter y m, $(CONFIG_ARCH_SUN60IW3)))
snd_soc_sunxi_ahub_i2s-objs		+= $(PLATS_DIR)/snd_sun60iw3_i2s.o
endif
snd_soc_sunxi_ahub_i2s-objs		+= $(PLATS_DIR)/snd_sun60iw3_i2s.o
snd_soc_sunxi_ahub_i2s-objs		+= snd_sunxi_ahub_i2s.o
obj-$(CONFIG_SND_SOC_SUNXI_AHUB_I2S)	+= snd_soc_sunxi_ahub_i2s.o

# OWA
ifneq (, $(filter y m, $(CONFIG_ARCH_SUN60IW3)))
snd_soc_sunxi_ahub_owa-objs		+= $(PLATS_DIR)/snd_sun60iw3_owa.o
endif
snd_soc_sunxi_ahub_owa-objs		+= $(PLATS_DIR)/snd_sun60iw3_owa.o
snd_soc_sunxi_ahub_owa-objs		+= snd_sunxi_ahub_owa.o
obj-$(CONFIG_SND_SOC_SUNXI_AHUB_OWA)	+= snd_soc_sunxi_ahub_owa.o

# DMIC
ifneq (, $(filter y m, $(CONFIG_ARCH_SUN60IW3)))
snd_soc_sunxi_ahub_dmic-objs		+= $(PLATS_DIR)/snd_sun60iw3_dmic.o
endif
snd_soc_sunxi_ahub_dmic-objs		+= $(PLATS_DIR)/snd_sun60iw3_dmic.o
snd_soc_sunxi_ahub_dmic-objs		+= snd_sunxi_ahub_dmic.o
obj-$(CONFIG_SND_SOC_SUNXI_AHUB_DMIC)	+= snd_soc_sunxi_ahub_dmic.o

# machine
snd_soc_sunxi_ahub_mach-objs		+= snd_sunxi_ahub_mach_utils.o
snd_soc_sunxi_ahub_mach-objs		+= snd_sunxi_ahub_mach.o
obj-$(CONFIG_SND_SOC_SUNXI_AHUB_MACH)	+= snd_soc_sunxi_ahub_mach.o

ccflags-$(CONFIG_SND_SOC_SUNXI_DYNAMIC_DEBUG)	+= -DDYNAMIC_DEBUG_MODULE
