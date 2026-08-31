# SPDX-License-Identifier: GPL-2.0-or-later
#
# Allwinner audio driver Makefile
#
# Copyright(c) 2022-2027 Allwinnertech Co., Ltd.

ifneq (, $(filter y m, $(CONFIG_ARCH_SUN60IW3)))
obj-y += ahub/
else
obj-y += platform/
endif
obj-y += codecs/
obj-y += misc/
obj-y += pa/
