// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * cpu vf test module
 * Copyright (C) 2019 frank@allwinnertech.com
 */

#include <sunxi-log.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/arch_topology.h>
#include <linux/platform_device.h>

#define MAX_NAME_LEN		(20)
struct vf_device {
	struct regulator *cpu_reg;
	struct clk *cpu_clk;
	struct class vf_class;
	char vf_name[MAX_NAME_LEN];
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
static ssize_t volt_show(struct class *class, struct class_attribute *attr,
			 char *buf)
#else
static ssize_t volt_show(const struct class *class, const struct class_attribute *attr,
			 char *buf)
#endif
{
	struct vf_device *vf_dev = container_of(class, struct vf_device, vf_class);

	return sprintf(buf, "%d\n", regulator_get_voltage(vf_dev->cpu_reg));
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
static ssize_t volt_store(struct class *class, struct class_attribute *attr,
			  const char *buf, size_t count)
#else
static ssize_t volt_store(const struct class *class, const struct class_attribute *attr,
			  const char *buf, size_t count)
#endif
{
	struct vf_device *vf_dev = container_of(class, struct vf_device, vf_class);
	int volt;
	int ret;

	ret = kstrtoint(buf, 10, &volt);
	if (ret)
		return -EINVAL;

	regulator_set_voltage(vf_dev->cpu_reg, volt, INT_MAX);

	return count;
}
static CLASS_ATTR_RW(volt);

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
static ssize_t freq_show(struct class *class, struct class_attribute *attr,
			 char *buf)
#else
static ssize_t freq_show(const struct class *class, const struct class_attribute *attr,
			 char *buf)
#endif
{
	struct vf_device *vf_dev = container_of(class, struct vf_device, vf_class);

	return sprintf(buf, "%lu\n", clk_get_rate(vf_dev->cpu_clk));
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
static ssize_t freq_store(struct class *class, struct class_attribute *attr,
			  const char *buf, size_t count)
#else
static ssize_t freq_store(const struct class *class, const struct class_attribute *attr,
			  const char *buf, size_t count)
#endif
{
	struct vf_device *vf_dev = container_of(class, struct vf_device, vf_class);
	unsigned long freq;
	int ret;

	ret = kstrtoul(buf, 10, &freq);
	if (ret)
		return -EINVAL;

	clk_set_rate(vf_dev->cpu_clk, freq);

	return count;
}
static CLASS_ATTR_RW(freq);

static struct attribute *vf_class_attrs[] = {
	&class_attr_volt.attr,
	&class_attr_freq.attr,
	NULL,
};
ATTRIBUTE_GROUPS(vf_class);

/*
 * An earlier version of opp-v1 bindings used to name the regulator
 * "cpu0-supply", we still need to handle that for backwards compatibility.
 */
static const char *find_supply_name(struct device *dev)
{
	struct device_node *np;
	struct property *pp;
	int cpu = dev->id;
	const char *name = NULL;

	np = of_node_get(dev->of_node);

	/* This must be valid for sure */
	if (WARN_ON(!np))
		return NULL;

	/* Try "cpu0" for older DTs */
	if (!cpu) {
		pp = of_find_property(np, "cpu0-supply", NULL);
		if (pp) {
			name = "cpu0";
			goto node_put;
		}
	}

	pp = of_find_property(np, "cpu-supply", NULL);
	if (pp) {
		name = "cpu";
		goto node_put;
	}

	dev_dbg(dev, "no regulator for cpu%d\n", cpu);
node_put:
	of_node_put(np);
	return name;
}

static unsigned int vf_test_is_enable(struct device *dev)
{
	struct device_node *np;
	unsigned int vf_test_enable = 0;

	np = of_node_get(dev->of_node);

	/* This must be valid for sure */
	if (WARN_ON(!np))
		return 0;

	of_property_read_u32(np, "vf-test-enable", &vf_test_enable);

	of_node_put(np);
	return vf_test_enable;
}

static int vf_test_probe(struct platform_device *pdev)
{
	struct device *cpu_dev;
	const char *name;
	struct vf_device *vf_dev;
	int ret = 0;
	int cpu = 0;
	int cluster_id = -1;

	for_each_possible_cpu(cpu) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
		if (cluster_id == topology_physical_package_id(cpu))
			continue;

		cluster_id = topology_physical_package_id(cpu);
#else
		if (cluster_id == topology_cluster_id(cpu))
			continue;

		cluster_id = topology_cluster_id(cpu);
#endif
		cpu_dev = get_cpu_device(cpu);
		if (!cpu_dev) {
			sunxi_err(NULL, "failed to get cpu%d device\n", cpu);
			return -ENODEV;
		}

		if (!vf_test_is_enable(cpu_dev) && cpu == 0) {
			return 0;
		}

		vf_dev = kzalloc(sizeof(*vf_dev), GFP_KERNEL);
		if (!vf_dev) {
			return -ENOMEM;
		}

		vf_dev->cpu_clk = clk_get(cpu_dev, NULL);
		ret = PTR_ERR_OR_ZERO(vf_dev->cpu_clk);
		if (ret) {
			/*
			 * If cpu's clk node is present, but clock is not yet
			 * registered, we should try defering probe.
			 */
			if (ret == -EPROBE_DEFER)
				sunxi_warn(cpu_dev, "cpu%d clock not ready, retry\n", cpu);
			else
				sunxi_err(cpu_dev, "cpu%d failed to get clock: %d\n", cpu, ret);

			return ret;
		}

		name = find_supply_name(cpu_dev);
		if (name) {
			vf_dev->cpu_reg = regulator_get_optional(cpu_dev, name);
			ret = PTR_ERR_OR_ZERO(vf_dev->cpu_reg);
			if (ret) {
				/*
				 * If cpu's regulator supply node is present, but regulator is
				 * not yet registered, we should try defering probe.
				 */
				if (ret == -EPROBE_DEFER)
					sunxi_warn(cpu_dev, "cpu%d regulator not ready, retry\n", cpu);
				else
					sunxi_err(cpu_dev, "cpu%d no regulator for cpu0: %d\n", cpu, ret);

				return ret;
			}
		}

		snprintf(vf_dev->vf_name, MAX_NAME_LEN, "sunxi_vf_cpu%d", cpu);
		vf_dev->vf_class.name = vf_dev->vf_name;
		vf_dev->vf_class.class_groups = vf_class_groups;

		ret = class_register(&vf_dev->vf_class);
		if (ret) {
			sunxi_err(NULL, "failed to vf class register for cpu%d\n", cpu);
			return ret;
		}
	}

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0))
static int vf_test_remove(struct platform_device *pdev)
{
	/* add cleanup if it need */
	return 0;
}
#else
static void vf_test_remove(struct platform_device *pdev)
{
}
#endif

static struct platform_driver vf_test_driver = {
	.probe = vf_test_probe,
	.remove = vf_test_remove,
	.driver = {
		.name = "sunxi-cpu-vf-test",
	},
};

static int __init sunxi_vf_init(void)
{
	struct platform_device *pdev;
	int ret;

	ret = platform_driver_register(&vf_test_driver);
	if (ret)
		return ret;

	pdev = platform_device_register_simple("sunxi-cpu-vf-test", -1, NULL, 0);
	if (IS_ERR(pdev)) {
		platform_driver_unregister(&vf_test_driver);
		return PTR_ERR(pdev);
	}

	return 0;
}

static void __exit sunxi_vf_exit(void)
{
	/* add cleanup if it need */
}

module_init(sunxi_vf_init);
module_exit(sunxi_vf_exit);
MODULE_DESCRIPTION("SUNXI CPU VF TEST");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("panzhijian <panzhijian@allwinnertech.com>");
MODULE_VERSION("1.0.4");
