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
#include <linux/of_device.h>
#include <linux/component.h>
#include <drm/drm_property.h>

#include <drm/drm_print.h>
#include <video/of_display_timing.h>

#include "include.h"
#include "sunxi_drm_intf.h"
#include "sunxi_drm_drv.h"

#if IS_ENABLED(CONFIG_AW_DRM_BRIDGE_LT8711)
extern const struct sunxi_bridge_info lt8711_data;
#endif

#if IS_ENABLED(CONFIG_AW_DRM_BRIDGE_GM7123)
extern const struct sunxi_bridge_info gm7123_data;
#endif

static const struct of_device_id sunxi_drm_bridge_match[] = {
#if IS_ENABLED(CONFIG_AW_DRM_BRIDGE_LT8711)
	{ .compatible = "lontium,lt8711", .data = &lt8711_data},
#endif

#if IS_ENABLED(CONFIG_AW_DRM_BRIDGE_GM7123)
	{ .compatible = "corpro,gm7123", .data = &gm7123_data},
#endif
	{},
};

int sunxi_drm_bridge_get_property(struct drm_connector *connector,
				  const struct drm_connector_state *state,
				  struct drm_property *property,
				  uint64_t *val,
				  struct sunxi_bridge_priv *priv)
{
	if (priv->prop_priority == property)
		*val = priv->priority;
	if (priv->prop_node_path == property)
		*val = priv->node_path_blob_id;

	return 0;
}

int sunxi_drm_bridge_create_attatch_prop(struct sunxi_bridge_priv *priv)
{
	struct drm_property_blob *blob = NULL;

	/* priority */
	priv->prop_priority = sunxi_drm_create_attach_property_range(priv->drm,
								     &priv->connector.base,
								     "priority", 0, 0xFF,
								     priv->priority);

	/* node path */
	priv->prop_node_path = drm_property_create(priv->drm, DRM_MODE_PROP_BLOB, "node_path", 0);
	drm_object_attach_property(&priv->connector.base, priv->prop_node_path, 0);
	blob = drm_property_create_blob(priv->drm, strlen(priv->node_path) + 1, priv->node_path);
	if (IS_ERR(blob)) {
		DRM_ERROR("Failed to create blob\n");
		return PTR_ERR(blob);
	}
	priv->node_path_blob_id = blob->base.id;

	return 0;
}

int sunxi_drm_bridge_get_prop_value_from_dts(struct device *dev)
{
	struct sunxi_bridge_priv *priv = dev_get_drvdata(dev);
	const struct sunxi_bridge_info *info = of_device_get_match_data(dev);
	int ret = 0;
	int value = 0;
	struct device_node *np;
	struct display_timings *disp = NULL;
	struct display_timing *timing = NULL;

	/* priority */
	ret = of_property_read_u32(dev->of_node, "priority", &value);
	if (ret) {
		switch (info->connector_type) {
		case DRM_MODE_CONNECTOR_HDMIA:
			priv->priority = SUNXI_DEFAULT_PRIORITY_HDMI;
			break;
		case DRM_MODE_CONNECTOR_VGA:
			priv->priority = SUNXI_DEFAULT_PRIORITY_VGA;
			break;
		/* TODO: add other type */
		default:
			priv->priority = 1;
		}
	} else {
		priv->priority = value;
	}

	/* node path */
	np = dev->of_node;
	if (np) {
		memset(priv->node_path, 0, sizeof(priv->node_path));
		sprintf(priv->node_path, "/%s", of_node_full_name(np));
	}

	disp = of_get_display_timings(np);
	if (!disp) {
		BRIDGE_ERR("%pOF: problems parsing panel-timin\n", np);
		return -ENODEV;
	}
	timing = display_timings_get(disp, disp->native_mode);
	if (!timing) {
		BRIDGE_ERR("%pOF: problems parsing panel-timin\n", np);
		return -ENODEV;
	}
	videomode_from_timing(timing, &priv->video_mode);
	return 0;
}

static int sunxi_drm_bridge_bind(struct device *dev, struct device *master, void *data)
{
	const struct sunxi_bridge_info *match = of_device_get_match_data(dev);

	if (match->drv_func.bind)
		match->drv_func.bind(dev, master, data);

	BRIDGE_INFO("bind %s done\n", match->name);
	return 0;
}

static void sunxi_drm_bridge_unbind(struct device *dev, struct device *master, void *data)
{
	const struct sunxi_bridge_info *match = of_device_get_match_data(dev);

	if (match->drv_func.unbind)
		match->drv_func.unbind(dev, master, data);

	BRIDGE_INFO("unbind %s done\n", match->name);
}

static const struct component_ops sunxi_drm_bridge_component_ops = {
	.bind   = sunxi_drm_bridge_bind,
	.unbind = sunxi_drm_bridge_unbind,
};

static int sunxi_drm_bridge_probe(struct platform_device *pdev)
{
	return component_add(&pdev->dev, &sunxi_drm_bridge_component_ops);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
static int sunxi_drm_bridge_remove(struct platform_device *pdev)
#else
static void sunxi_drm_bridge_remove(struct platform_device *pdev)
#endif
{
	component_del(&pdev->dev, &sunxi_drm_bridge_component_ops);

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
	return 0;
#endif
}

struct platform_driver sunxi_drm_bridge_driver = {
	.probe  = sunxi_drm_bridge_probe,
	.remove = sunxi_drm_bridge_remove,
	.driver = {
		.name = "allwinner,sunxi_drm_bridge",
		.of_match_table = sunxi_drm_bridge_match,
	},
};
