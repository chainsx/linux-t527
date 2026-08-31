/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/* sunxi_simple_drm_bridge.c
 *
 * Copyright (C) 2023 Allwinnertech Co.Ltd
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <drm/drm_atomic_helper.h>
#include <drm/drm_bridge.h>
#include <drm/drm_crtc.h>
#include <drm/drm_property.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_probe_helper.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_graph.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <video/display_timing.h>
#include <video/of_display_timing.h>
#include <video/videomode.h>
#include <linux/component.h>

#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/machine.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/phy/phy.h>
#include <linux/phy/phy-mipi-dphy.h>
#include <linux/version.h>
#include <drm/drm_panel.h>
#include <drm/drm_of.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_print.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/drm_property.h>
#include "../sunxi_drm_drv.h"
#include "../sunxi_drm_intf.h"

#define POWER_MAX 3
#define GPIO_MAX  3

struct simple_bridge_info {
	const struct drm_bridge_timings *timings;
	unsigned int connector_type;
};

struct sunxi_simple_drm_bridge {
	unsigned int priority;
	struct drm_property *prop_priority;
	unsigned int node_path_blob_id;
	struct drm_property *prop_node_path;
	unsigned char node_path[256];
	struct device *dev;
	struct drm_device *drm;
	struct drm_bridge bridge;
	struct videomode video_mode;
	struct drm_connector connector;
	const struct simple_bridge_info *info;
	struct regulator *supply[POWER_MAX];
	struct gpio_desc *enable_gpio[GPIO_MAX];
	struct gpio_desc *reset_gpio;
	struct {
		unsigned int power;
		unsigned int enable;
		unsigned int reset;
	} delay;
};

static inline struct sunxi_simple_drm_bridge *drm_bridge_to_sunxi_simple_drm_bridge(struct drm_bridge *bridge)
{
	return container_of(bridge, struct sunxi_simple_drm_bridge, bridge);
}

static inline struct sunxi_simple_drm_bridge *drm_connector_to_sunxi_simple_drm_bridge(struct drm_connector *connector)
{
	return container_of(connector, struct sunxi_simple_drm_bridge, connector);
}

static void sunxi_simple_bridge_sleep(unsigned int msec)
{
	if (msec > 20)
		msleep(msec);
	else
		usleep_range(msec * 1000, (msec + 1) * 1000);
}

static int sunxi_simple_bridge_connector_get_modes(struct drm_connector *connector)
{
	int ret;
/*
	ret = drm_add_modes_noedid(connector, 1280, 720);
	drm_set_preferred_mode(connector, 1280, 720);
	return ret;
*/
	struct sunxi_simple_drm_bridge *bridge = drm_connector_to_sunxi_simple_drm_bridge(connector);
	ret =  drm_bridge_get_modes(&(bridge->bridge), connector);
	return ret;
}

static int drm_bridge_connector_get_property(struct drm_connector *connector,
		const struct drm_connector_state *state,
		struct drm_property *property,
		uint64_t *val)
{
	struct sunxi_simple_drm_bridge *bridge = drm_connector_to_sunxi_simple_drm_bridge(connector);

	if (bridge->prop_priority == property)
		*val = bridge->priority;
	if (bridge->prop_node_path == property)
		*val = bridge->node_path_blob_id;

	return 0;
}

static const struct drm_connector_helper_funcs sunxi_simple_drm_bridge_con_helper_funcs = {
	.get_modes = sunxi_simple_bridge_connector_get_modes,
};

static const struct drm_connector_funcs sunxi_simple_drm_bridge_con_funcs = {
	.destroy = drm_connector_cleanup,
	.reset = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
	.fill_modes = drm_helper_probe_single_connector_modes,
	.atomic_get_property = drm_bridge_connector_get_property,
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 16, 0)
static int sunxi_simple_drm_bridge_attach(struct drm_bridge *bridge,
					  enum drm_bridge_attach_flags flags)
#else
static int sunxi_simple_drm_bridge_attach(struct drm_bridge *bridge, struct drm_encoder *encoder,
					  enum drm_bridge_attach_flags flags)
#endif
{
	struct sunxi_simple_drm_bridge *simple_bridge = drm_bridge_to_sunxi_simple_drm_bridge(bridge);
	int ret;
	struct drm_property_blob *blob = NULL;

	if (!bridge->encoder) {
		DRM_ERROR("Missing encoder\n");
		return -ENODEV;
	}

	ret = drm_connector_init(bridge->dev, &simple_bridge->connector, &sunxi_simple_drm_bridge_con_funcs, simple_bridge->info->connector_type);
	if (ret) {
		DRM_ERROR("Failed to initialize connector\n");
		return ret;
	}

	drm_connector_helper_add(&simple_bridge->connector, &sunxi_simple_drm_bridge_con_helper_funcs);
	drm_connector_register(&simple_bridge->connector);
	ret = drm_connector_attach_encoder(&simple_bridge->connector, bridge->encoder);
	if (ret) {
		DRM_ERROR("Failed to attach encoder\n");
		return ret;
	}

	simple_bridge->prop_priority = sunxi_drm_create_attach_property_range(simple_bridge->drm, &simple_bridge->connector.base, "priority", 0, 0xFF, simple_bridge->priority);
	simple_bridge->prop_node_path = drm_property_create(simple_bridge->drm, DRM_MODE_PROP_BLOB, "node_path", 0);
	drm_object_attach_property(&simple_bridge->connector.base, simple_bridge->prop_node_path, 0);
	blob = drm_property_create_blob(simple_bridge->drm, strlen(simple_bridge->node_path) + 1, simple_bridge->node_path);
	if (IS_ERR(blob)) {
		DRM_ERROR("Failed to create blob\n");
		return PTR_ERR(blob);
	}
	simple_bridge->node_path_blob_id = blob->base.id;

	return 0;
}

static void sunxi_simple_drm_bridge_enable(struct drm_bridge *bridge)
{
	int i, err;
	struct sunxi_simple_drm_bridge *simple_bridge = drm_bridge_to_sunxi_simple_drm_bridge(bridge);

	for (i = 0; i < POWER_MAX; i++) {
		if (simple_bridge->supply[i]) {
			err = regulator_enable(simple_bridge->supply[i]);
			if (err < 0) {
				dev_err(simple_bridge->dev, "failed to enable supply%d: %d\n", i, err);
				return;
			}
			if (simple_bridge->delay.power)
				sunxi_simple_bridge_sleep(simple_bridge->delay.power);
		}
	}

	if (simple_bridge->reset_gpio)
		gpiod_set_value_cansleep(simple_bridge->reset_gpio, 1);
	if (simple_bridge->delay.reset)
		sunxi_simple_bridge_sleep(simple_bridge->delay.reset);

	for (i = 0; i < GPIO_MAX; i++) {
		if (simple_bridge->enable_gpio[i]) {
			gpiod_set_value_cansleep(simple_bridge->enable_gpio[i], 1);
			if (simple_bridge->delay.enable)
				sunxi_simple_bridge_sleep(simple_bridge->delay.enable);
		}
	}
}

static void sunxi_simple_drm_bridge_disable(struct drm_bridge *bridge)
{
	int i;
	struct sunxi_simple_drm_bridge *simple_bridge = drm_bridge_to_sunxi_simple_drm_bridge(bridge);

	for (i = GPIO_MAX; i > 0; i--) {
		if (simple_bridge->enable_gpio[i - 1]) {
			gpiod_set_value_cansleep(simple_bridge->enable_gpio[i - 1], 0);
			if (simple_bridge->delay.enable)
				sunxi_simple_bridge_sleep(simple_bridge->delay.enable);
		}
	}

	if (simple_bridge->reset_gpio)
		gpiod_set_value_cansleep(simple_bridge->reset_gpio, 0);
	if (simple_bridge->delay.reset)
		sunxi_simple_bridge_sleep(simple_bridge->delay.reset);

	for (i = POWER_MAX; i > 0; i--) {
		if (simple_bridge->supply[i - 1]) {
			regulator_disable(simple_bridge->supply[i - 1]);
			if (simple_bridge->delay.power)
				sunxi_simple_bridge_sleep(simple_bridge->delay.power);
		}
	}
}

static int sunxi_simple_drm_bridge_get_modes(struct drm_bridge *bridge, struct drm_connector *connector)
{
	struct sunxi_simple_drm_bridge *simple_bridge = drm_bridge_to_sunxi_simple_drm_bridge(bridge);
	struct drm_display_mode *mode;

	mode = drm_mode_create(connector->dev);
	if (!mode)
		return 0;

	drm_display_mode_from_videomode(&simple_bridge->video_mode, mode);
	mode->type |= DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_bridge_funcs sunxi_simple_drm_bridge_bridge_funcs = {
	.attach = sunxi_simple_drm_bridge_attach,
	.enable = sunxi_simple_drm_bridge_enable,
	.disable = sunxi_simple_drm_bridge_disable,
	.get_modes = sunxi_simple_drm_bridge_get_modes,
};

static int simple_drm_bridge_parse_dt(struct sunxi_simple_drm_bridge *bridge)
{
	int ret, i;
	char *power_name = NULL;
	char *gpio_name = NULL;
	struct device_node *np = bridge->bridge.of_node;
	struct display_timings *disp = NULL;
	struct display_timing *timing = NULL;

	for (i = 0; i < POWER_MAX; i++) {
		power_name = kasprintf(GFP_KERNEL, "power%d", i);
		bridge->supply[i] = devm_regulator_get_optional(bridge->dev, power_name);
		if (IS_ERR(bridge->supply[i])) {
			ret = PTR_ERR(bridge->supply[i]);
			if (ret != -ENODEV) {
				if (ret != -EPROBE_DEFER)
					dev_err(bridge->dev, "failed to request regulator(%s): %d\n", power_name, ret);
				return ret;
			}
			bridge->supply[i] = NULL;
		}
	}

	/* Get GPIOs and backlight controller. */
	for (i = 0; i < GPIO_MAX; i++) {
		gpio_name = kasprintf(GFP_KERNEL, "enable%d", i);
		bridge->enable_gpio[i] = devm_gpiod_get_optional(bridge->dev, gpio_name, GPIOD_OUT_HIGH);
		if (IS_ERR(bridge->enable_gpio[i])) {
			ret = PTR_ERR(bridge->enable_gpio[i]);
			dev_err(bridge->dev, "failed to request %s GPIO: %d\n", gpio_name, ret);
			return ret;
		}
	}

	disp = of_get_display_timings(np);
	if (!disp) {
		dev_err(bridge->dev, "%pOF: problems parsing panel-timin\n", np);
		return -ENODEV;
	}

	timing = display_timings_get(disp, disp->native_mode);
	if (!timing) {
		dev_err(bridge->dev, "%pOF: problems parsing panel-timin\n", np);
		return -ENODEV;
	}

	videomode_from_timing(timing, &bridge->video_mode);

	return 0;
}

static int sunxi_drm_bridge_bind(struct device *dev, struct device *master, void *data)
{
	int ret;
	u32 value;
	struct device_node *np;
	struct sunxi_simple_drm_bridge *bridge = dev_get_drvdata(dev);

	DRM_INFO("%s start\n", __FUNCTION__);
	bridge->drm = (struct drm_device *)data;
	bridge->dev = (struct device *)dev;
	np = bridge->dev->of_node;
	if (np) {
		memset(bridge->node_path, 0, sizeof(bridge->node_path));
		sprintf(bridge->node_path, "/%s", of_node_full_name(np));
	}

	bridge->priority = SUNXI_DEFAULT_PRIORITY_VGA;
	ret = of_property_read_u32(dev->of_node, "priority", &value);
	if (!ret) {
		DRM_INFO("read priority ok %d\n", value);
		bridge->priority = value;
	}

	return 0;
}

static void sunxi_drm_bridge_unbind(struct device *dev, struct device *master, void *data) {}

static const struct component_ops sunxi_drm_bridge_component_ops = {
	.bind = sunxi_drm_bridge_bind,
	.unbind = sunxi_drm_bridge_unbind,
};


static int sunxi_simple_drm_bridge_probe(struct platform_device *pdev)
{
	struct sunxi_simple_drm_bridge *simple_bridge;

	DRM_INFO("%s start\n", __FUNCTION__);
	simple_bridge = devm_kzalloc(&pdev->dev, sizeof(*simple_bridge), GFP_KERNEL);
	if (!simple_bridge)
		return -ENOMEM;

	platform_set_drvdata(pdev, simple_bridge);
	simple_bridge->info = of_device_get_match_data(&pdev->dev);
	simple_bridge->bridge.funcs = &sunxi_simple_drm_bridge_bridge_funcs;
	simple_bridge->bridge.of_node = pdev->dev.of_node;
	simple_bridge->bridge.ops = DRM_BRIDGE_OP_MODES;
	drm_bridge_add(&simple_bridge->bridge);
	simple_drm_bridge_parse_dt(simple_bridge);

	return component_add(&pdev->dev, &sunxi_drm_bridge_component_ops);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
static int sunxi_simple_drm_bridge_remove(struct platform_device *pdev)
#else
static void sunxi_simple_drm_bridge_remove(struct platform_device *pdev)
#endif
{
	struct sunxi_simple_drm_bridge *simple_bridge = platform_get_drvdata(pdev);
	drm_bridge_remove(&simple_bridge->bridge);

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
	return 0;
#endif
}

static const struct simple_bridge_info corpro_gm7123_data = {
	.connector_type = DRM_MODE_CONNECTOR_VGA,
};

static const struct of_device_id sunxi_simple_drm_bridge_match[] = {
	{ .compatible = "corpro,gm7123", .data = &corpro_gm7123_data },
	{},
};

MODULE_DEVICE_TABLE(of, sunxi_simple_drm_bridge_match);

struct platform_driver sunxi_simple_drm_bridge_driver = {
	.probe = sunxi_simple_drm_bridge_probe,
	.remove = sunxi_simple_drm_bridge_remove,
	.driver = {
		.name = "allwinner,sunxi_simple_drm_bridge",
		.of_match_table = sunxi_simple_drm_bridge_match,
	},
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 13, 0)
MODULE_IMPORT_NS(DRM_BRIDGE);
#else
MODULE_IMPORT_NS("DRM_BRIDGE");
#endif
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.1");
MODULE_DESCRIPTION("Simple drm bridge driver");
