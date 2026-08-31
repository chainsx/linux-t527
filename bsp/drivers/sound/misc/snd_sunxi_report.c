// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright(c) 2025 - 2028 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's ALSA SoC Audio driver
 *
 * Copyright (c) 2023, huhaoxin <huhaoxin@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/extcon.h>
#include <linux/cdev.h>

#include "snd_sunxi_log.h"
#include "snd_sunxi_adapter.h"

/* User-level code */
/** #include <poll.h>
  *
  * struct pollfd pfd;
  * bool is_insert;
  * int ret;
  * int fd;
  * fd = open("/dev/report_event", O_RDONLY);
  * if (fd < 0) {
  *   AudioLogE("open report_event failed");
  *   goto exit;
  * }
  *
  * pfd.fd = fd;
  * pfd.events = POLLIN;
  *
  * while (1) {
  *   ret = poll(&pfd, 1, -1);
  *   AudioLogI("poll return,ret =%d", ret);
  *   if (ret > 0 && (pfd.revents & POLLIN)) {
  *     if (read(fd, &is_insert, sizeof(bool)) > 0)
  *       AudioLogI("linein status:%d", is_insert);
  *   }
  */

/* define in drivers/gpadc/sunxi_gpadc.c */
extern const char *sunxi_gpadc_get_extcon_name(void);

int g_dev_num;

static DECLARE_WAIT_QUEUE_HEAD(report_wq);
struct sunxi_report {
	dev_t report_dev;
	struct cdev report_cdev;
	struct class *report_class;
	struct device *report_device;

	struct extcon_dev *extdev;
	struct notifier_block nb;
	bool status;
	bool is_readable;
} g_report;

static unsigned int report_poll(struct file *file, poll_table *wait)
{
	SND_LOG_INFO("report wait...\n");

	poll_wait(file, &report_wq, wait);

	if (g_report.is_readable) {
		g_report.is_readable = false;
		return POLLIN | POLLRDNORM;
	}

	return 0;
}

static ssize_t report_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	(void)file;
	(void)count;
	(void)ppos;

	SND_LOG_INFO("report status -> %d\n", g_report.status);

	if (copy_to_user(buf, &g_report.status, sizeof(bool)))
	    return -EFAULT;

	return sizeof(bool);
}

static const struct file_operations report_fops = {
    .owner = THIS_MODULE,
    .poll  = report_poll,
    .read  = report_read,
};

/* if startup detection is required, then extcon_get_state() needs to be called */
static int sunxi_linein_plugin_notifier(struct notifier_block *nb, unsigned long event, void *ptr)
{
	SND_LOG_INFO("report event -> %lu\n", event);

	if (event)
		g_report.status = 1;
	else
		g_report.status = 0;

	g_report.is_readable = true;

	wake_up_interruptible(&report_wq);

	return NOTIFY_DONE;
}

static int __init sunxi_report_init(void)
{
	const char *extcon_name = NULL;
	int ret;

	SND_LOG_DEBUG("\n");

	ret = alloc_chrdev_region(&g_report.report_dev, 0, 1, "report_dev");
	if (ret) {
		SND_LOG_ERR("alloc_chrdev_region failed\n");
		goto err_alloc_chrdev;
	}

	cdev_init(&g_report.report_cdev, &report_fops);
	ret = cdev_add(&g_report.report_cdev, g_report.report_dev, 1);
	if (ret) {
		SND_LOG_ERR("cdev_add failed\n");
		goto err_cdev_add;
	}

	g_report.report_class = sunxi_adpt_class_create(THIS_MODULE, "report_class");
	if (IS_ERR_OR_NULL(g_report.report_class)) {
		SND_LOG_ERR("class_create failed\n");
		goto err_class_create;
	}
	g_report.report_device = device_create(g_report.report_class,
					       NULL,
					       g_report.report_dev,
					       NULL,
					       "report_event");
	if (IS_ERR_OR_NULL(g_report.report_device)) {
		SND_LOG_ERR("device_create failed\n");
		goto err_device_create;
	}

	extcon_name = sunxi_gpadc_get_extcon_name();
	if (!extcon_name) {
		SND_LOG_ERR("report extcon name null");
		goto err_sunxi_gpadc_get_extcon_name;
	}
	SND_LOG_INFO("report extcon name:%s", extcon_name);

	g_report.extdev = extcon_get_extcon_dev(extcon_name);
	if (IS_ERR(g_report.extdev)) {
		SND_LOG_ERR("get extcon dev failed\n");
		goto err_extcon_get_extcon_dev;
	}

	g_report.nb.notifier_call = sunxi_linein_plugin_notifier;
	ret = extcon_register_notifier(g_report.extdev, EXTCON_JACK_HEADPHONE, &g_report.nb);
	if (ret < 0) {
		SND_LOG_ERR("register notifier failed\n");
		goto err_extcon_register_notifier;
	}

	SND_LOG_INFO("report driver loaded\n");

	return 0;

err_extcon_register_notifier:
err_extcon_get_extcon_dev:
err_sunxi_gpadc_get_extcon_name:
err_device_create:
	class_destroy(g_report.report_class);
err_class_create:
	cdev_del(&g_report.report_cdev);
err_cdev_add:
	unregister_chrdev_region(g_report.report_dev, 1);
err_alloc_chrdev:
	return -1;
}

static void __exit sunxi_report_exit(void)
{
	SND_LOG_DEBUG("\n");

	unregister_chrdev(g_dev_num, "linein_insert_event");
}

module_init(sunxi_report_init);
module_exit(sunxi_report_exit);

MODULE_AUTHOR("huhaoxin@allwinnertech.com");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
MODULE_DESCRIPTION("sunxi report driver");