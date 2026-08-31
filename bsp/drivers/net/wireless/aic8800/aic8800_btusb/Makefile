obj-$(CONFIG_AIC8800_BTUSB_SUPPORT) := aic8800_btusb.o

SRCPATH := $(shell dirname $(lastword $(MAKEFILE_LIST)))
ccflags-y += -I$(SRCPATH)/../aic8800_bsp
ccflags-y += -Wno-implicit-fallthrough

aic8800_btusb-y := aic_btusb.o
