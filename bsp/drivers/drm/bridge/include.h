/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved.
 * lt8711.c
 *
 * Copyright (C) 2023 Allwinnertech Co.Ltd
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#ifndef _BRIDGE_INCLUDE_
#define _BRIDGE_INCLUDE_

#include <drm/drm_print.h>
#include <drm/drm_bridge.h>
#include <video/videomode.h>

#ifdef BRIDGE_INFO
#undef BRIDGE_INFO
#endif
#define BRIDGE_INFO(fmt, args...)				\
	do {										\
		DRM_INFO("[bridge][info] "fmt, ##args); \
	} while (0)

#ifdef BRIDGE_ERR
#undef BRIDGE_ERR
#endif
#define BRIDGE_ERR(fmt, args...)				  \
	do {										  \
		DRM_ERROR("[bridge][error] "fmt, ##args); \
	} while (0)

struct sunxi_bridge_drv_funcs {
	int (*bind)(struct device *dev, struct device *master, void *data);
	int (*unbind)(struct device *dev, struct device *master, void *data);
};

struct sunxi_bridge_info {
	char name[32];
	unsigned int bridge_id;
	unsigned int connector_type;
	unsigned int max_rate;
	struct sunxi_bridge_drv_funcs drv_func;
};

/**
 * @desc: sunxi bridge private data
 * You can add other private data to adapt your bridge device
 */
struct sunxi_bridge_priv {
	struct device *dev;
	struct device *parent_dev;
	struct drm_device *drm;
	struct drm_bridge bridge;
	struct videomode video_mode;
	struct drm_connector connector;
	const struct sunxi_bridge_info *info;
	struct gpio_desc *enable_gpio[3];
	struct regulator *supply[3];

	/* property */
	unsigned int priority;
	struct drm_property *prop_priority;
	unsigned int node_path_blob_id;
	struct drm_property *prop_node_path;
	unsigned char node_path[256];

	bool (*get_hpd)(struct device *dev);
	struct edid* (*get_edid)(struct device *dev);
};

int sunxi_drm_bridge_get_property(struct drm_connector *connector,
				  const struct drm_connector_state *state,
				  struct drm_property *property,
				  uint64_t *val,
				  struct sunxi_bridge_priv *priv);
int sunxi_drm_bridge_create_attatch_prop(struct sunxi_bridge_priv *priv);
int sunxi_drm_bridge_get_prop_value_from_dts(struct device *dev);

#endif /* _BRIDGE_INCLUDE_ */
