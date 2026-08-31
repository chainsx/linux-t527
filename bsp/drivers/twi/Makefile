# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the twi drivers.
#

obj-$(CONFIG_AW_TWI) += twi-sunxi.o
twi-sunxi-objs		+= twi-sunxi-core.o
twi-sunxi-$(CONFIG_AW_TWI_DVFS)		+= dvfs/twi-sunxi-dvfs.o
ccflags-$(CONFIG_AW_TWI_DYNAMIC_DEBUG) := -DDYNAMIC_DEBUG_MODULE
