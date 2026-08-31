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
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/component.h>
#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_edid.h>
#include <drm/drm_atomic.h>
#include <drm/drm_bridge.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_atomic_helper.h>

#include "../sunxi_drm_drv.h"
#include "include.h"

static inline
struct sunxi_bridge_priv *drm_bridge_to_priv(struct drm_bridge *bridge)
{
	return container_of(bridge, struct sunxi_bridge_priv, bridge);
}

static inline
struct sunxi_bridge_priv *drm_conn_to_priv(struct drm_connector *connector)
{
	return container_of(connector, struct sunxi_bridge_priv, connector);
}

static int sunxi_connector_lt8711_get_modes(struct drm_connector *c)
{
	struct sunxi_bridge_priv *lt8711 = drm_conn_to_priv(c);
	struct edid *edid = NULL;
	int mode_cnt = 0;

	if (lt8711->get_edid) {
		edid = lt8711->get_edid(lt8711->parent_dev);
		mode_cnt = drm_add_edid_modes(c, edid);
	}

	if (mode_cnt == 0) {
		/* TODO, when can't get mode from edid. we will use default mode */
	}

	BRIDGE_INFO("lt8711 get mode count %d\n", mode_cnt);
	return mode_cnt;
}

static enum drm_connector_status
sunxi_connector_lt8711_detect(struct drm_connector *c, bool force)
{
	struct sunxi_bridge_priv *lt8711 = drm_conn_to_priv(c);
	bool hpd = false;

	if (lt8711->get_hpd)
		hpd = lt8711->get_hpd(lt8711->parent_dev);

	BRIDGE_INFO("lt8711 get hpd %d\n", hpd);
	return hpd ? connector_status_connected : connector_status_disconnected;
}

static int sunxi_connector_lt8711_get_property(struct drm_connector *connector,
					       const struct drm_connector_state *state,
					       struct drm_property *property,
					       uint64_t *val)
{
	struct sunxi_bridge_priv *lt8711 = drm_conn_to_priv(connector);

	sunxi_drm_bridge_get_property(connector, state, property, val, lt8711);

	return 0;
}

static const struct drm_connector_funcs sunxi_connector_lt8711_funcs = {
	.destroy				= drm_connector_cleanup,
	.reset					= drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_connector_destroy_state,
	.fill_modes				= drm_helper_probe_single_connector_modes,
	.detect					= sunxi_connector_lt8711_detect,
	.atomic_get_property = sunxi_connector_lt8711_get_property,
};

static const struct drm_connector_helper_funcs sunxi_connector_lt8711_helper_funcs = {
	.get_modes = sunxi_connector_lt8711_get_modes,
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 16, 0)
static int sunxi_bridge_lt8711_attach(struct drm_bridge *bridge,
				      enum drm_bridge_attach_flags flags)
#else
static int sunxi_bridge_lt8711_attach(struct drm_bridge *bridge, struct drm_encoder *encoder,
				      enum drm_bridge_attach_flags flags)
#endif
{
	struct sunxi_bridge_priv *lt8711 = drm_bridge_to_priv(bridge);
	int ret = 0;

	if (IS_ERR_OR_NULL(lt8711) || IS_ERR_OR_NULL(bridge->encoder)) {
		BRIDGE_ERR("check invalid params\n");
		return -ENOMEM;
	}

	drm_connector_helper_add(&lt8711->connector, &sunxi_connector_lt8711_helper_funcs);
	ret = drm_connector_init(bridge->dev, &lt8711->connector,
		&sunxi_connector_lt8711_funcs, lt8711->bridge.type);
	if (ret) {
		BRIDGE_ERR("drm connector init HDMIA failed\n");
		return ret;
	}

	lt8711->connector.polled = DRM_CONNECTOR_POLL_HPD;
	ret = drm_connector_attach_encoder(&lt8711->connector, bridge->encoder);
	if (ret) {
		BRIDGE_ERR("drm connector attach encoder failed\n");
		return ret;
	}

	sunxi_drm_bridge_create_attatch_prop(lt8711);

	return 0;
}

static void sunxi_bridge_lt8711_detach(struct drm_bridge *bridge)
{
	struct sunxi_bridge_priv *lt8711 = drm_bridge_to_priv(bridge);

	drm_connector_cleanup(&lt8711->connector);
}

static enum drm_mode_status sunxi_bridge_lt8711_mode_valid(struct drm_bridge *bridge,
			const struct drm_display_info *info,
			const struct drm_display_mode *mode)
{
	if (mode->clock > 297000)
		return MODE_BAD;
	return MODE_OK;
}

static const struct drm_bridge_funcs sunxi_bridge_funcs_lt8711 = {
	.attach = sunxi_bridge_lt8711_attach,
	.detach = sunxi_bridge_lt8711_detach,
	.mode_valid = sunxi_bridge_lt8711_mode_valid,
};

static void lt8711_power_on(struct device *dev)
{
	struct sunxi_bridge_priv *lt8711 = dev_get_drvdata(dev);
	char *gpio_name = NULL;
	char *power_name = NULL;
	int i = 0, ret = 0;

	BRIDGE_INFO("lt8711 power on\n");

	for (i = 0; i < 1; i++) {
		gpio_name = kasprintf(GFP_KERNEL, "enable%d", i);
		lt8711->enable_gpio[i] =
			devm_gpiod_get_optional(dev, gpio_name, GPIOD_OUT_HIGH);
		if (IS_ERR(lt8711->enable_gpio[i])) {
			ret = PTR_ERR(lt8711->enable_gpio[i]);
			BRIDGE_ERR("failed to request %s GPIO: %d\n", gpio_name, ret);
			continue;
		}
		BRIDGE_INFO("get gpio%d %s done\n", ret, gpio_name);
		gpiod_set_value_cansleep(lt8711->enable_gpio[0], 1);
	}

	for (i = 0; i < 1; i++) {
		power_name = kasprintf(GFP_KERNEL, "power%d", i);
		lt8711->supply[i] = devm_regulator_get_optional(dev, power_name);
		if (IS_ERR(lt8711->supply[i])) {
			ret = PTR_ERR(lt8711->supply[i]);

			if (ret != -ENODEV) {
				if (ret != -EPROBE_DEFER)
					dev_err(lt8711->dev,
						"failed to request regulator(%s): %d\n",
						power_name, ret);
			}

			lt8711->supply[i] = NULL;
		}
		if (lt8711->supply[i]) {
			ret = regulator_enable(lt8711->supply[i]);
			BRIDGE_INFO("get power%d %s done\n", ret, power_name);
		}
	}

}

static int _lt8711_bind(struct device *dev, struct device *master, void *data)
{
	struct sunxi_bridge_priv *lt8711 = NULL;
	const struct sunxi_bridge_info *info = of_device_get_match_data(dev);

	lt8711 = devm_kzalloc(dev, sizeof(*lt8711), GFP_KERNEL);
	if (IS_ERR_OR_NULL(lt8711))
		return -ENOMEM;

	dev_set_drvdata(dev, lt8711);

	lt8711_power_on(dev);

	lt8711->dev = dev;
	lt8711->info = info;
	lt8711->drm = (struct drm_device *)data;
	lt8711->bridge.ops     = DRM_BRIDGE_OP_MODES;
	lt8711->bridge.funcs   = &sunxi_bridge_funcs_lt8711;
	lt8711->bridge.of_node = dev->of_node;
	lt8711->bridge.type    = info->connector_type;
	lt8711->bridge.driver_private = lt8711;

	drm_bridge_add(&lt8711->bridge);

	sunxi_drm_bridge_get_prop_value_from_dts(dev);

	return 0;
}

static int _lt8711_unbind(struct device *dev, struct device *master, void *data)
{
	struct sunxi_bridge_priv *lt8711 = dev_get_drvdata(dev);

	drm_bridge_remove(&lt8711->bridge);
	devm_kfree(dev, lt8711);
	lt8711 = NULL;
	return 0;
}

const struct sunxi_bridge_info lt8711_data = {
	.name = "LT8711",
	.connector_type = DRM_MODE_CONNECTOR_HDMIA,
	.max_rate = 297000,

	.drv_func.bind   = _lt8711_bind,
	.drv_func.unbind = _lt8711_unbind,
};
