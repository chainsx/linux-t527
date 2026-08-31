/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved.
 * gm7123.c
 *
 * Copyright (C) 2023 Allwinnertech Co.Ltd
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/component.h>
#include <linux/gpio/consumer.h>
#include <drm/drm_edid.h>
#include <drm/drm_atomic.h>
#include <drm/drm_bridge.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_atomic_helper.h>
#include "../sunxi_drm_drv.h"
#include "include.h"

static inline struct sunxi_bridge_priv *drm_bridge_to_priv(struct drm_bridge *bridge)
{
	return container_of(bridge, struct sunxi_bridge_priv, bridge);
}

static inline struct sunxi_bridge_priv *drm_conn_to_priv(struct drm_connector *connector)
{
	return container_of(connector, struct sunxi_bridge_priv, connector);
}

static enum drm_connector_status sunxi_connector_gm7123_detect(struct drm_connector *c, bool force)
{
	return connector_status_connected;
}

static int sunxi_connector_gm7123_get_property(struct drm_connector *connector,
					       const struct drm_connector_state *state,
					       struct drm_property *property,
					       uint64_t *val)
{
	struct sunxi_bridge_priv *gm7123 = drm_conn_to_priv(connector);
	sunxi_drm_bridge_get_property(connector, state, property, val, gm7123);
	return 0;
}

static const struct drm_connector_funcs sunxi_connector_gm7123_funcs = {
	.destroy				= drm_connector_cleanup,
	.reset					= drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_connector_destroy_state,
	.fill_modes				= drm_helper_probe_single_connector_modes,
	.detect					= sunxi_connector_gm7123_detect,
	.atomic_get_property = sunxi_connector_gm7123_get_property,
};

static int sunxi_connector_gm7123_get_modes(struct drm_connector *c)
{
	struct sunxi_bridge_priv *gm7123 = drm_conn_to_priv(c);
	return drm_bridge_get_modes(&(gm7123->bridge), c);
}

static const struct drm_connector_helper_funcs sunxi_connector_gm7123_helper_funcs = {
	.get_modes = sunxi_connector_gm7123_get_modes,
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 16, 0)
static int sunxi_bridge_gm7123_attach(struct drm_bridge *bridge,
				      enum drm_bridge_attach_flags flags)
#else
static int sunxi_bridge_gm7123_attach(struct drm_bridge *bridge, struct drm_encoder *encoder,
				      enum drm_bridge_attach_flags flags)
#endif
{
	struct sunxi_bridge_priv *gm7123 = drm_bridge_to_priv(bridge);
	int ret = 0;
	if (IS_ERR_OR_NULL(gm7123) || IS_ERR_OR_NULL(bridge->encoder)) {
		BRIDGE_ERR("check invalid params\n");
		return -ENOMEM;
	}

	drm_connector_helper_add(&gm7123->connector, &sunxi_connector_gm7123_helper_funcs);
	ret = drm_connector_init(bridge->dev, &gm7123->connector,
		&sunxi_connector_gm7123_funcs, gm7123->bridge.type);
	if (ret) {
		BRIDGE_ERR("drm connector init HDMIA failed\n");
		return ret;
	}

	gm7123->connector.polled = DRM_CONNECTOR_POLL_HPD;
	ret = drm_connector_attach_encoder(&gm7123->connector, bridge->encoder);
	if (ret) {
		BRIDGE_ERR("drm connector attach encoder failed\n");
		return ret;
	}

	sunxi_drm_bridge_create_attatch_prop(gm7123);

	return 0;
}

static void sunxi_bridge_gm7123_detach(struct drm_bridge *bridge)
{
	struct sunxi_bridge_priv *gm7123 = drm_bridge_to_priv(bridge);
	drm_connector_cleanup(&gm7123->connector);
}

static void sunxi_bridge_gm7123_enable(struct drm_bridge *bridge) {}

static void sunxi_bridge_gm7123_disable(struct drm_bridge *bridge) {}

static int sunxi_bridge_gm7123_get_modes(struct drm_bridge *bridge, struct drm_connector *connector)
{

	struct sunxi_bridge_priv *gm7123_priv = drm_bridge_to_priv(bridge);
	struct drm_display_mode *mode;
	mode = drm_mode_create(connector->dev);
	if (!mode)
		return 0;
	drm_display_mode_from_videomode(&gm7123_priv->video_mode, mode);
	mode->type |= DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	drm_mode_probed_add(connector, mode);
	return 1;
}

static enum drm_mode_status sunxi_bridge_gm7123_mode_valid(struct drm_bridge *bridge,
							   const struct drm_display_info *info,
							   const struct drm_display_mode *mode)
{
	if (mode->clock > 297000)
		return MODE_BAD;
	return MODE_OK;
}

static const struct drm_bridge_funcs sunxi_bridge_funcs_gm7123 = {
	.attach = sunxi_bridge_gm7123_attach,
	.detach = sunxi_bridge_gm7123_detach,
	.enable = sunxi_bridge_gm7123_enable,
	.disable = sunxi_bridge_gm7123_disable,
	.get_modes = sunxi_bridge_gm7123_get_modes,
	.mode_valid = sunxi_bridge_gm7123_mode_valid,
};

static void gm7123_power_on(struct device *dev)
{
	struct sunxi_bridge_priv *gm7123 = dev_get_drvdata(dev);
	char *gpio_name = NULL;
	int i = 0, ret = 0;

	BRIDGE_INFO("gm7123 power on\n");

	for (i = 0; i < 1; i++) {
		gpio_name = kasprintf(GFP_KERNEL, "enable%d", i);
		gm7123->enable_gpio[i] = devm_gpiod_get_optional(dev, gpio_name, GPIOD_OUT_HIGH);
		if (IS_ERR(gm7123->enable_gpio[i])) {
			ret = PTR_ERR(gm7123->enable_gpio[i]);
			BRIDGE_ERR("failed to request %s GPIO: %d\n", gpio_name, ret);
			continue;
		}
		BRIDGE_INFO("get gpio%d %s done\n", ret, gpio_name);
		gpiod_set_value_cansleep(gm7123->enable_gpio[0], 1);
	}
}

static int _gm7123_bind(struct device *dev, struct device *master, void *data)
{
	struct sunxi_bridge_priv *gm7123 = NULL;
	const struct sunxi_bridge_info *info = of_device_get_match_data(dev);

	gm7123 = devm_kzalloc(dev, sizeof(*gm7123), GFP_KERNEL);
	if (IS_ERR_OR_NULL(gm7123))
		return -ENOMEM;

	dev_set_drvdata(dev, gm7123);

	gm7123_power_on(dev);

	gm7123->dev = dev;
	gm7123->info = info;
	gm7123->drm = (struct drm_device *)data;
	gm7123->bridge.ops     = DRM_BRIDGE_OP_MODES;
	gm7123->bridge.funcs   = &sunxi_bridge_funcs_gm7123;
	gm7123->bridge.of_node = dev->of_node;
	gm7123->bridge.type    = info->connector_type;
	gm7123->bridge.driver_private = gm7123;

	drm_bridge_add(&gm7123->bridge);

	sunxi_drm_bridge_get_prop_value_from_dts(dev);

	return 0;
}

static int _gm7123_unbind(struct device *dev, struct device *master, void *data)
{
	struct sunxi_bridge_priv *gm7123 = dev_get_drvdata(dev);

	drm_bridge_remove(&gm7123->bridge);
	devm_kfree(dev, gm7123);
	gm7123 = NULL;
	return 0;
}

const struct sunxi_bridge_info gm7123_data = {
	.name = "gm7123",
	.connector_type = DRM_MODE_CONNECTOR_VGA,
	.max_rate = 297000,
	.drv_func.bind   = _gm7123_bind,
	.drv_func.unbind = _gm7123_unbind,
};
